#include "AssetManager.hpp"
#include "Renderer/Backend/IShader.hpp"

namespace Jupiter::Assets {

AssetManager *AssetManager::s_Instance = nullptr;

AssetManager::AssetManager() { s_Instance = this; }
AssetManager::~AssetManager() { s_Instance = nullptr; }

void AssetManager::Init() {
  fs::path base = fs::current_path();
  if (fs::exists(base / "assets"))
    m_AssetsDir = base / "assets";
  else {
    J_ERROR("AssetManager: FAILED to find assets folder!");
    return;
  }

  J_INFO("AssetManager: Root established at {}", m_AssetsDir.string());
}

void AssetManager::Shutdown() {
  m_Textures.clear();
  m_Shaders.clear();
  m_Audios.clear();
  J_INFO("AssetManager: Resources cleared.");
}

Ref<Renderer::ITexture> AssetManager::GetTexture(const std::string name) {
  if (!s_Instance) {
    J_ERROR("AssetManager isn't initialized");
    return nullptr;
  }

  auto &textures = s_Instance->m_Textures;
  if (textures.find(name) == textures.end()) {
    fs::path path = s_Instance->m_AssetsDir / "Textures" / name;
    // TODO: textures[name] = m_RenderDevice->CreateTexture({});
  }
  return textures[name];
}

Ref<Renderer::IShader> AssetManager::GetShader(const std::string name) {
  if (!s_Instance) {
    J_ERROR("AssetManager isn't initialized");
    return nullptr;
  }

  auto &shaders = s_Instance->m_Shaders;
  if (shaders.find(name) == shaders.end()) {
    fs::path path = s_Instance->m_AssetsDir / "Shaders" / (name + ".glsl");
    shaders[name] = LoadShaderFromFile(path);
  }
  return shaders[name];
}

Ref<Core::Audio> AssetManager::GetAudio(const std::string name) {
  if (!s_Instance) {
    J_ERROR("AssetManager isn't initialized");
    return nullptr;
  }

  auto &audios = s_Instance->m_Audios;
  if (audios.find(name) == audios.end()) {
    fs::path path = s_Instance->m_AssetsDir / "Audio" / name;
    audios[name] = LoadAudioFromFile(path);
  }
  return audios[name];
}

Ref<Renderer::IShader> AssetManager::LoadShaderFromFile(const fs::path &path) {
  std::array<std::string, EnumToSize(Renderer::ShaderType::Count)> shaderSrcs;

  static auto StringToType =
      [](const std::string &type) -> Renderer::ShaderType {
    if (type == "vertex")
      return Renderer::ShaderType::Vertex;
    else if (type == "fragment")
      return Renderer::ShaderType::Fragment;
    else if (type == "geometry")
      return Renderer::ShaderType::Geometry;
    else if (type == "compute")
      return Renderer::ShaderType::Compute;
    return Renderer::ShaderType::None;
  };

  std::string source = Utils::ReadFile(path);

  std::string_view token = "#type";
  size_t pos = source.find(token, 0);
  while (pos != std::string::npos) {
    size_t eol = source.find_first_of("\r\n", pos);
    size_t beg = pos + token.length() + 1;
    std::string type = source.substr(beg, eol - beg);

    size_t nextLinePos = source.find_first_not_of("\r\n", eol);
    pos = source.find(token, nextLinePos);

    shaderSrcs[static_cast<u32>(StringToType(type))] =
        (pos == std::string::npos)
            ? source.substr(nextLinePos)
            : source.substr(nextLinePos, pos - nextLinePos);
  }
  return Renderer::IShader::Create(shaderSrcs);
}
} // namespace Jupiter::Assets
