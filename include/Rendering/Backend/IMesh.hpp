#pragma once

#include "Core/Utils.hpp"
#include "Rendering/Backend/BufferLayout.hpp"
#include "Rendering/Backend/IBuffer.hpp"
#include "Core/Storage.hpp"

namespace Jupiter::Rendering::RHI {

struct MeshDesc {
  BufferLayout layout;
  RHI::BufferDesc vertices{}, indices{BufferType::Index, BufferUsage::Static, DataType::UInt, 0};
  u32 vertexCap{}, indexCap{};
  Storage vertexData, indexData;
  BufferLayout instanceLayout;
  u32 instanceCap{};
};

class IMesh {
public:
  virtual ~IMesh() = default;

  IMesh(const IMesh &) = delete;
  IMesh &operator=(const IMesh &) = delete;
  IMesh(IMesh &&) = delete;
  IMesh &operator=(IMesh &&) = delete;

  virtual bool SetGeometry(const BufferData &vertices, const BufferData &indices) = 0;
  virtual bool UpdateVertices(const BufferData &data) = 0;
  virtual bool UpdateIndices(const BufferData &data) = 0;

  virtual const BufferLayout &GetLayout() const noexcept = 0;

  virtual u32 GetVertexCount() const noexcept = 0;
  virtual u32 GetVertexCapacity() const noexcept = 0;

  virtual u32 GetIndexCount() const noexcept = 0;
  virtual u32 GetIndexCapacity() const noexcept = 0;

protected:
  IMesh() = default;
};
} // namespace Jupiter::Rendering::RHI
