#pragma once

#include <filesystem>
#include <unordered_map>
#include <string>

#include "Core/Utils.hpp"
#include "Renderer/Backend/ITexture.hpp"
#include "Renderer/Backend/IShader.hpp"
#include "Audio.hpp"

namespace Jupiter::Assets {
namespace fs = std::filesystem;

class AssetManager {
public:
  AssetManager();
  ~AssetManager();

  AssetManager(AssetManager &) = delete;
  AssetManager &operator=(AssetManager &) = delete;

  void Init();
  void Shutdown();

  AssetManager &GetInstance() { return *s_Instance; }

  static Ref<Renderer::ITexture> GetTexture(const std::string name);
  static Ref<Renderer::IShader> GetShader(const std::string name);
  static Ref<Core::Audio> GetAudio(const std::string name);

private:
  static AssetManager *s_Instance;

  fs::path m_AssetsDir;

  std::unordered_map<std::string, Ref<Renderer::ITexture>> m_Textures;
  std::unordered_map<std::string, Ref<Renderer::IShader>> m_Shaders;
  std::unordered_map<std::string, Ref<Core::Audio>> m_Audios;

  static Ref<Renderer::ITexture> LoadTextureFromFile(const fs::path &path) {
    return nullptr;
  }

  static Ref<Renderer::IShader> LoadShaderFromFile(const fs::path &path);

  // TODO:
  static Ref<Core::Audio> LoadAudioFromFile(const fs::path &) {
    return nullptr;
  }
};
}; // namespace Jupiter::Assets
//

/*
*namespace Assets {

*/
