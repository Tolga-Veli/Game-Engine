#pragma once

#include "AssetManager.hpp"
#include "Core/Assert.hpp"
#include "Rendering/RendererAPI.hpp"

namespace Jupiter::Assets {

template <Asset T>
AssetHandle<T> AssetManager::LoadAsset(const fs::path &relativePath, const typename AssetTraits<T>::LoadOptions &opts) {
  std::unique_lock lock(m_RegistryMutex);
  if (!m_Active)
    return {};

  const fs::path path = (m_AssetsDir / relativePath).lexically_normal();
  const AssetID id = AssetTraits<T>::MakeID(path, opts);
  if (auto it = m_Registry.find(id); it != m_Registry.end())
    return {it->second.poolIndex, it->second.poolGeneration};

  auto &pool = m_AssetPools.GetPool<T>();
  const auto handle = pool.Reserve(id);
  try {
    m_Registry.emplace(id, AssetRecord{.type = AssetTraits<T>::type,
                                       .name = relativePath.generic_string(),
                                       .path = path,
                                       .lastWriteTime = {},
                                       .loadOptions = opts,
                                       .poolIndex = handle.GetID(),
                                       .poolGeneration = handle.GetGen(),
                                       .runtimeCreated = false});
    m_FileAssets.insert(id);
    if (QueueAssetLoad<T>(id, handle, path, opts))
      return handle;

  } catch (...) {
    pool.Release(handle);
    m_Registry.erase(id);
    m_FileAssets.erase(id);
    throw;
  }

  pool.Release(handle);
  m_Registry.erase(id);
  m_FileAssets.erase(id);
  return {};
}

template <Asset T> Ref<T> AssetManager::ResolveAsset(AssetHandle<T> handle) const {
  std::shared_lock lock(m_RegistryMutex);
  CORE_VERIFY(IsRenderThread(), "GPU-backed assets may only be resolved on the render thread");
  return m_AssetPools.GetPool<T>().Resolve(handle);
}

template <Asset T>
AssetHandle<T> AssetManager::CreateAsset(std::string_view name, typename AssetTraits<T>::AssetData data) {
  const AssetID id = HashRuntimeAssetName(name, AssetTraits<T>::type);
  std::unique_lock lock(m_RegistryMutex);
  if (!m_Active)
    return {};

  if (auto it = m_Registry.find(id); it != m_Registry.end())
    return {it->second.poolIndex, it->second.poolGeneration};

  auto &pool = m_AssetPools.GetPool<T>();
  const auto handle = pool.Reserve(id);
  try {
    m_Registry.emplace(id, AssetRecord{.type = AssetTraits<T>::type,
                                       .name = std::string{name},
                                       .path = {},
                                       .lastWriteTime = {},
                                       .loadOptions = {},
                                       .poolIndex = handle.GetID(),
                                       .poolGeneration = handle.GetGen(),
                                       .runtimeCreated = true});
    // Enqueue before publishing the handle to another producer. The render
    // task never waits for producer work while it owns the registry lock.
    if (QueueRenderTask([this, id, handle, data = std::move(data)](Rendering::RenderExecutionContext &context) {
          PublishAsset<T>(id, handle, data, context.resources);
        }))
      return handle;

  } catch (...) {
    pool.Release(handle);
    m_Registry.erase(id);
    throw;
  }

  pool.Release(handle);
  m_Registry.erase(id);
  return {};
}

template <Asset T> bool AssetManager::QueueAssetLoad(AssetID id, AssetHandle<T> handle, fs::path path,
                                                     typename AssetTraits<T>::LoadOptions opts) {
  return QueueLoad([this, id, handle, path = std::move(path), opts = std::move(opts)] {
    try {
      std::error_code ec;
      const auto writeTime = fs::last_write_time(path, ec);
      {
        std::unique_lock lock(m_RegistryMutex);
        auto it = m_Registry.find(id);
        if (!m_Active || it == m_Registry.end() || !m_AssetPools.GetPool<T>().IsValid(handle))
          return;

        if (!ec)
          it->second.lastWriteTime = writeTime;
      }

      auto data = AssetTraits<T>::LoadDataFromFile(path, opts);
      if (!QueueRenderTask([this, id, handle, data = std::move(data)](Rendering::RenderExecutionContext &context) {
            PublishAsset<T>(id, handle, data, context.resources);
          }))
        FailAsset<T>(id, handle);

    } catch (const std::exception &error) {
      LOG_ERROR("AssetManager: failed to read '{}': {}", path.string(), error.what());
      FailAsset<T>(id, handle);
    } catch (...) { FailAsset<T>(id, handle); }
  });
}

template <Asset T> void AssetManager::PublishAsset(AssetID id, AssetHandle<T> handle,
                                                   const typename AssetTraits<T>::AssetData &data,
                                                   Rendering::RHI::IResourceFactory &factory) {
  {
    std::shared_lock lock(m_RegistryMutex);
    CORE_VERIFY(IsRenderThread(), "Asset publication must run on the render thread");
    if (!m_Active || !m_AssetPools.GetPool<T>().IsValid(handle))
      return;
  }

  Ref<T> asset;
  try {
    auto impl = CreateAssetImpl<T>(factory, data);
    if (impl)
      asset = CreateAssetFromImpl<T>(std::move(impl));

  } catch (const std::exception &error) {
    LOG_ERROR("AssetManager: failed to create {}: {}", to_string(AssetTraits<T>::type), error.what());
  } catch (...) {}

  if (!asset) {
    FailAsset<T>(id, handle);
    LOG_ERROR("AssetManager: failed to create {} (asset {})", to_string(AssetTraits<T>::type), id);
    return;
  }

  {
    std::unique_lock lock(m_RegistryMutex);
    auto it = m_Registry.find(id);
    if (it == m_Registry.end() || !m_AssetPools.GetPool<T>().Replace(handle, std::move(asset)))
      return;

    it->second.loading = false;
  }
  LOG_INFO("AssetManager: published {} (asset {})", to_string(AssetTraits<T>::type), id);
}

template <Asset T> void AssetManager::FailAsset(AssetID id, AssetHandle<T> handle) {
  std::unique_lock lock(m_RegistryMutex);
  auto it = m_Registry.find(id);
  if (it == m_Registry.end() || !m_AssetPools.GetPool<T>().IsValid(handle))
    return;

  // A failed reload preserves the old payload and its Ready state.
  m_AssetPools.GetPool<T>().MarkFailed(handle);
  it->second.loading = false;
}

template <Asset T> void AssetManager::ReloadAsset(const PendingReload &job) {
  std::unique_lock lock(m_RegistryMutex);
  auto it = m_Registry.find(job.id);
  if (!m_Active || it == m_Registry.end() || it->second.runtimeCreated || it->second.loading)
    return;

  auto &record = it->second;
  AssetHandle<T> handle{record.poolIndex, record.poolGeneration};
  record.loading = true;
  try {
    if (QueueAssetLoad<T>(job.id, handle, record.path,
                          std::get<typename AssetTraits<T>::LoadOptions>(record.loadOptions)))
      return;
  } catch (...) {
    record.loading = false;
    throw;
  }
  record.loading = false;
  // Retry a saturated load queue on the next watcher pass.
  record.lastWriteTime = {};
}

template <Asset T> bool AssetManager::Unload(AssetHandle<T> handle) {
  auto &manager = GetInstance();
  std::shared_lock lock(manager.m_RegistryMutex);
  if (!manager.m_Active || !manager.m_AssetPools.GetPool<T>().IsValid(handle))
    return false;

  return manager.QueueRenderTask([&manager, handle](Rendering::RenderExecutionContext &) {
    std::unique_lock registryLock(manager.m_RegistryMutex);
    auto &pool = manager.m_AssetPools.GetPool<T>();
    if (!pool.IsValid(handle))
      return;

    const auto it = manager.m_Registry.find(pool.GetKey(handle));
    if (it != manager.m_Registry.end() && it->second.type == AssetTraits<T>::type &&
        it->second.poolIndex == handle.GetID() && it->second.poolGeneration == handle.GetGen()) {
      manager.m_FileAssets.erase(it->first);
      manager.m_Registry.erase(it);
    }
    pool.Release(handle);
  });
}

template <Asset T>
Scope<typename AssetTraits<T>::Impl> AssetManager::CreateAssetImpl(Rendering::RHI::IResourceFactory &factory,
                                                                   const typename AssetTraits<T>::AssetData &data) {
  constexpr auto type = AssetTraits<T>::type;
  if constexpr (type == AssetType::Texture)
    return factory.CreateTexture(data);
  else if constexpr (type == AssetType::Shader)
    return factory.CreateShader(data);
  else if constexpr (type == AssetType::Mesh)
    return factory.CreateMesh(data);
  else if constexpr (type == AssetType::Font)
    return factory.CreateFont(data);
  else if constexpr (type == AssetType::Material)
    return factory.CreateMaterial();
  else if constexpr (type == AssetType::Sampler)
    return factory.CreateSampler(data);
}

} // namespace Jupiter::Assets
