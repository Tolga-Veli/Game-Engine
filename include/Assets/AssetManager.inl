#pragma once

#include "AssetManager.hpp"
#include "Core/Assert.hpp"
#include "Rendering/RendererAPI.hpp"

namespace Jupiter::Assets {

template <Asset T>
AssetHandle<T> AssetManager::LoadAsset(const fs::path &relativePath, const typename AssetTraits<T>::LoadOptions &opts) {
  fs::path path = (m_AssetsDir / relativePath).lexically_normal();
  std::string name = relativePath.generic_string();
  AssetID id = AssetTraits<T>::MakeID(path, opts);

  {
    std::shared_lock lock(m_RegistryMutex);
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};
  }

  std::error_code ec;
  const auto writeTime = fs::last_write_time(path, ec);
  if (ec)
    LOG_WARN("AssetManager: couldn't determine modification time for '{}'", path.string());

  AssetHandle<T> handle;
  {
    std::unique_lock lock(m_RegistryMutex);

    // might have already been created
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};

    handle = m_AssetPools.GetPool<T>().Reserve();
    m_Registry.emplace(id, AssetRecord{.type = AssetTraits<T>::type,
                                       .name = name,
                                       .path = path,
                                       .lastWriteTime = writeTime,
                                       .loadOptions = opts,
                                       .poolIndex = handle.GetID(),
                                       .poolGeneration = handle.GetGen(),
                                       .runtimeCreated = false});
  }

  QueueRenderTask([this, id, handle, path, name, opts]() {
    auto data = AssetTraits<T>::LoadDataFromFile(path, opts);
    auto impl = CreateAssetImpl<T>(data);

    std::unique_lock lock(m_RegistryMutex);
    auto &pool = m_AssetPools.GetPool<T>();

    if (!pool.IsValid(handle))
      return;

    if (!impl) {
      LOG_ERROR("AssetManager: failed to load {} '{}'", to_string(AssetTraits<T>::type), path.string());

      pool.Release(handle);
      m_Registry.erase(id);
      return;
    }

    pool.Replace(handle, CreateAssetFromImpl<T>(std::move(impl)));
    LOG_INFO("AssetManager: loaded {} '{}'", to_string(AssetTraits<T>::type), name);
  });

  return handle;
}

template <Asset T> Ref<T> AssetManager::ResolveAsset(AssetHandle<T> handle) const {
  std::shared_lock lock(m_RegistryMutex);
  return m_AssetPools.GetPool<T>().Resolve(handle);
}

// Reserve+register happen synchronously so concurrent Create() calls for
// the same name still dedupe correctly regardless of which thread they
// come from. factory() itself is queued and only ever runs later, on the
// render thread, out of ProcessRenderTasks().
template <Asset T>
AssetHandle<T> AssetManager::CreateAsset(std::string_view name, const typename AssetTraits<T>::AssetData &data) {
  AssetID id = HashRuntimeAssetName(name, AssetTraits<T>::type);
  {
    std::shared_lock lock(m_RegistryMutex);
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};
  }

  std::string nameStr{name};
  AssetHandle<T> handle;
  {
    std::unique_lock lock(m_RegistryMutex);

    // someone may have beaten us to it between the two locks above
    if (auto it = m_Registry.find(id); it != m_Registry.end())
      return AssetHandle<T>{it->second.poolIndex, it->second.poolGeneration};

    handle = m_AssetPools.GetPool<T>().Reserve();
    m_Registry.emplace(id, AssetRecord{.type = AssetTraits<T>::type,
                                       .name = nameStr,
                                       .path = {},
                                       .lastWriteTime = {},
                                       .loadOptions = {},
                                       .poolIndex = handle.GetID(),
                                       .poolGeneration = handle.GetGen(),
                                       .runtimeCreated = true});
  }

  QueueRenderTask([this, id, handle, nameStr, data]() mutable {
    CORE_ASSERT(IsRenderThread(), "Runtime asset factory must run on render thread");

    // NOTE:
    // GPU creation happens here, on the render thread, whenever
    // ProcessRenderTasks() next picks this task up.
    auto impl = CreateAssetImpl<T>(data);

    std::unique_lock lock(m_RegistryMutex);
    auto &pool = m_AssetPools.GetPool<T>();

    if (!pool.IsValid(handle))
      return;

    if (!impl) {
      LOG_ERROR("AssetManager: failed to create runtime {} '{}'", to_string(AssetTraits<T>::type), nameStr);
      pool.Release(handle);
      m_Registry.erase(id);
      return;
    }

    pool.Replace(handle, CreateAssetFromImpl<T>(std::move(impl)));
    LOG_INFO("AssetManager: created runtime {} '{}'", to_string(AssetTraits<T>::type), nameStr);
  });

  return handle;
}

template <Asset T> void AssetManager::ReloadAsset(const PendingReload &job) {
  CORE_ASSERT(IsRenderThread(), "Asset reload must run on render thread");

  AssetRecord record;
  {
    std::shared_lock lock(m_RegistryMutex);
    auto it = m_Registry.find(job.id);
    if (it == m_Registry.end())
      return;

    record = it->second;
  }

  if (record.runtimeCreated || record.path.empty())
    return;

  const auto &opts = std::get<typename AssetTraits<T>::LoadOptions>(record.loadOptions);

  auto data = AssetTraits<T>::LoadDataFromFile(job.path, opts);
  auto impl = CreateAssetImpl<T>(data);

  if (!impl) {
    LOG_ERROR("AssetManager: reload FAILED for {} '{}'", to_string(AssetTraits<T>::type), job.path.string());
    return;
  }

  auto reloaded = CreateAssetFromImpl<T>(std::move(impl));
  {
    std::unique_lock lock(m_RegistryMutex);
    auto it = m_Registry.find(job.id);
    if (it == m_Registry.end())
      return;

    AssetHandle<T> handle{it->second.poolIndex, it->second.poolGeneration};
    if (!m_AssetPools.GetPool<T>().Replace(handle, std::move(reloaded)))
      return;
  }

  LOG_INFO("AssetManager: reloaded {} '{}'", to_string(AssetTraits<T>::type), record.name);
}

template <Asset T>
Scope<typename AssetTraits<T>::Impl> AssetManager::CreateAssetImpl(typename AssetTraits<T>::AssetData data) {
  constexpr auto type = AssetTraits<T>::type;

  if constexpr (type == AssetType::Texture)
    return Rendering::RendererAPI::GetResourceFactory()->CreateTexture(data);
  else if constexpr (type == AssetType::Shader)
    return Rendering::RendererAPI::GetResourceFactory()->CreateShader(data);
  else if constexpr (type == AssetType::Mesh)
    return Rendering::RendererAPI::GetResourceFactory()->CreateMesh(data);
  else if constexpr (type == AssetType::Font)
    return Rendering::RendererAPI::GetResourceFactory()->CreateFont(data);
  else if constexpr (type == AssetType::Material)
    return Rendering::RendererAPI::GetResourceFactory()->CreateMaterial();
}

} // namespace Jupiter::Assets
