#pragma once

#include "Core/Utils.hpp"

#include "Rendering/Backend/RenderUtils.hpp"
#include "IShader.hpp"
#include "BufferLayout.hpp"

namespace Jupiter::Rendering {

enum class CullMode : u8 { None, Front, Back };

enum class CompareOp : u8 {
  Never,
  Less,
  LEqual,
  Greater,
  GEqual,
  Equal,
  NotEqual,
  Always
};

struct PipelineSpec {
  Ref<IShader> shader;
  BufferLayout vertexLayout; // reuses whatever you already pass to
                             // IVertexBuffer::SetLayout

  BlendMode blend = BlendMode::None;
  bool depthTest = true;
  bool depthWrite = true;
  CompareOp depthCompare = CompareOp::Less;
  CullMode cull = CullMode::Back;
};

class IPipelineState {
public:
  virtual ~IPipelineState() = default;
  virtual void Bind() = 0;

  static Ref<IPipelineState> Create(const PipelineSpec &spec);
};

} // namespace Jupiter::Rendering
