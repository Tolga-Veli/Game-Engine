#pragma once
#include "Core/Utils.hpp"

namespace Jupiter::Rendering::RHI {

enum class FilterMode : u8 { Nearest, Linear };
enum class WrapMode : u8 { Repeat, ClampToEdge, MirroredRepeat, ClampToBorder };

struct SamplerSpec {
  FilterMode minFilter{FilterMode::Linear}, magFilter{FilterMode::Linear};
  WrapMode wrapS{WrapMode::Repeat}, wrapT{WrapMode::Repeat};
};

class ISampler {
public:
  virtual ~ISampler() = default;

  static Scope<ISampler> Create(const SamplerSpec &nfo);
};

} // namespace Jupiter::Rendering::RHI
