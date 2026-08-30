#pragma once
#include "Core/Utils.hpp"

namespace Jupiter::Rendering::RHI {

enum class FilterMode : u8 { Nearest, Linear };
enum class WrapMode : u8 { Repeat, ClampToEdge, MirroredRepeat };

struct SamplerSpec {
  FilterMode minFilter = FilterMode::Linear;
  FilterMode magFilter = FilterMode::Linear;
  WrapMode wrapS = WrapMode::Repeat;
  WrapMode wrapT = WrapMode::Repeat;
};

class ISampler {
public:
  virtual ~ISampler() = default;
  virtual void Bind(u32 slot) const = 0;

  static Ref<ISampler> Create(const SamplerSpec &spec);
};

} // namespace Jupiter::Rendering::RHI
