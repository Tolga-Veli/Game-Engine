#pragma once

#include "Renderer/Backend/ITexture.hpp"

#include <glad/gl.h>

namespace Jupiter::Renderer {
class GLTexture : public ITexture {
public:
  GLTexture() = default;
  GLTexture(const std::string &path);

  void Bind(u32 slot = 0) const override {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
  }

  void Unbind() const override { glBindTexture(GL_TEXTURE_2D, 0); }

private:
  u32 m_RendererID{}, m_Width{}, m_Height{};
  std::filesystem::path m_Path;
};
} // namespace Jupiter::Renderer
