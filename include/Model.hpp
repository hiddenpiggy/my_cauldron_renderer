#ifndef VERTEX_HPP
#define VERTEX_HPP
#include "ResourceUploadHeap.hpp"
#include "glm/glm.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "VkBufferPool.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include <vector>

namespace hiddenpiggy {
class Vertex {
public:
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 texCoord;

  static vk::VertexInputBindingDescription getBindingDescription() {
    vk::VertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = vk::VertexInputRate::eVertex;
    return bindingDescription;
  }

  static std::array<vk::VertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

    // Position attribute
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    // Normal attribute
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    // Texture coordinate attribute
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
    return attributeDescriptions;
  }
};

class Mesh {
public:
  Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
      : m_vertices(vertices), m_indices(indices) {}

  const std::vector<Vertex> &getVertices() const { return m_vertices; }
  const std::vector<uint32_t> &getIndices() const { return m_indices; }

  static vk::VertexInputBindingDescription getBindingDescription() {
    return Vertex::getBindingDescription();
  }

  static std::array<vk::VertexInputAttributeDescription, 3>
  getAttributeDescriptions() {
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    return attributeDescriptions;
  }

private:
  std::vector<Vertex> m_vertices;
  std::vector<uint32_t> m_indices;
};

class Model {
public:
    Model(std::vector<Mesh> meshes, glm::mat4 transform, BufferPool *bufferPool)
        : m_meshes(meshes), m_transform(transform), m_bufferPool(bufferPool) {}

    const std::vector<Mesh>& getMeshes() const { return m_meshes; }
    const glm::mat4& getTransform() const { return m_transform; }

    void setTransform(const glm::mat4& transform) { m_transform = transform; }

    //this code is used for testing only
    static Model generateDefaultModel(BufferPool *bufferPool) {
      std::vector<Vertex> vertices = {
         {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f},  {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
      };

      std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0
      };

      std::vector<Mesh> meshes = {
        Mesh(vertices, indices)
      };


      glm::mat4 transform(1.0f);
      return Model(meshes, transform, bufferPool);
    }

    //here is the code to allocate the buffer of a model, and then upload it to vertex buffer and index buffer
    void allocateMemoryAndUpload(VkContext *context, BufferPool *bufferPool, ResourceUploadHeap *resourceUploadHeap) {
      //calculate current total buffer size
      int totalVertexBufferSize = 0;
      int totalIndexBufferSize = 0;
      for(size_t i = 0; i < m_meshes.size(); ++i) {
        totalVertexBufferSize += sizeof(Vertex) * m_meshes[i].getVertices().size();
        totalIndexBufferSize += sizeof(uint32_t) * m_meshes[i].getIndices().size();
      }

      vk::DeviceSize vertexBufferSize = totalVertexBufferSize;
      vk::DeviceSize indexBufferSize = totalIndexBufferSize;

      //Get queue family indices
      uint32_t graphicsFamilyIndex = context->getQueueFamilyIndices().graphicsFamilyIndex.value();

      vk::BufferCreateInfo VertexbufferCreateInfo {
        {},  vertexBufferSize, vk::BufferUsageFlagBits::eVertexBuffer| vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive,1, &graphicsFamilyIndex, nullptr 
      };

      VmaAllocationCreateInfo VertexallocCreateInfo{};
      VertexallocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
      VertexallocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

      //get vertex buffer
      BufferWrapper vertexBuffer = bufferPool->allocateMemory(VertexbufferCreateInfo, VertexallocCreateInfo);
      m_vertexBuffer = vertexBuffer.buffer;
      m_vertexBufferAllocation = vertexBuffer.allocation;

      //here I want to upload all mesh data on the buffer
      uint32_t offset = 0;
      for(const auto &mesh : m_meshes) {
        auto vertices = mesh.getVertices();
        resourceUploadHeap->uploadBufferData(vertices.data(), vertices.size() * sizeof(Vertex), m_vertexBuffer, m_vertexBufferAllocation, offset);
        offset += vertices.size() * sizeof(Vertex);
      }

      //here is index buffer create info
      vk::BufferCreateInfo indexBufferCreateInfo {
        {}, indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, 1, &graphicsFamilyIndex, nullptr
      };

      VmaAllocationCreateInfo indexBufferAllocCreateInfo{};
      VertexallocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
      VertexallocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

      BufferWrapper indexBuffer = bufferPool->allocateMemory(indexBufferCreateInfo, indexBufferAllocCreateInfo);
      m_indexBuffer = indexBuffer.buffer;
      m_indexBufferAllocation = indexBuffer.allocation;

      //here I want to upload all mesh data on the buffer
      offset = 0;
      for(const auto &mesh : m_meshes) {
        auto indices = mesh.getIndices();
        resourceUploadHeap->uploadBufferData(indices.data(), indices.size() * sizeof(uint32_t), m_indexBuffer, m_indexBufferAllocation, offset);
        offset += indices.size() * sizeof(uint32_t);
      }
    }
    

    void draw(vk::CommandBuffer commandBuffer, vk::DescriptorSet& descriptorSet, vk::PipelineLayout& pipelineLayout) {
      vk::Buffer vertexBuffers[] = { m_vertexBuffer };
      vk::DeviceSize offsets[] = {0};
      vk::DescriptorSet descriptorSets[] = { descriptorSet };

      commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
      commandBuffer.bindIndexBuffer(m_indexBuffer, 0, vk::IndexType::eUint32);
      commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSets, nullptr);
      commandBuffer.drawIndexed(static_cast<uint32_t>(m_meshes[0].getIndices().size()), 1, 0, 0, 0);
    }

    void destroy() {
      BufferWrapper vertexBufferWrapper = { m_vertexBuffer, m_vertexBufferAllocation, m_vertexBufferAllocationInfo };
      BufferWrapper indexBufferWrapper = { m_indexBuffer, m_indexBufferAllocation, m_indexBufferAllocationInfo};
      m_bufferPool->freeBuffer(indexBufferWrapper);
      m_bufferPool->freeBuffer(vertexBufferWrapper);
    }


private:
    std::vector<Mesh> m_meshes;
    glm::mat4 m_transform;

    BufferPool *m_bufferPool;
    vk::Buffer m_vertexBuffer;
    VmaAllocation m_vertexBufferAllocation;
    VmaAllocationInfo m_vertexBufferAllocationInfo;
    vk::Buffer m_indexBuffer;
    VmaAllocation m_indexBufferAllocation;
    VmaAllocationInfo m_indexBufferAllocationInfo;
};


} // namespace hiddenpiggy

#endif