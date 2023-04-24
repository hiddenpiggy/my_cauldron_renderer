#ifndef UNIFORM_BUFFERS_HPP
#define UNIFORM_BUFFERS_HPP

#include "VkBufferPool.hpp"
#include "glm/glm.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace hiddenpiggy {
struct UniformBufferObject {
  // mvp matrix
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;

  inline void UpdateModelMatrix(const glm::mat4& model) {
    this->model = model;
  }

  inline void UpdateViewMatrix(const glm::mat4& view) {
    this->model = view;
  }

  inline void UpdateProjMatrix(const glm::mat4& proj) {
    this->proj = proj;
  }
};

class UniformBuffers {
public:
  UniformBuffers(VkContext *pContext, BufferPool *pBufferPool)
      : m_Context(pContext), m_BufferPool(pBufferPool) {}
  void OnCreate(uint32_t descriptorCount) {
    assert(m_BufferPool != nullptr);
    m_descriptorCount = descriptorCount;
    m_layoutBinding =
        vk::DescriptorSetLayoutBinding{{},
                                       vk::DescriptorType::eUniformBuffer,
                                       1,
                                       vk::ShaderStageFlagBits::eVertex,
                                       nullptr};
    
    auto queueFamilyIndex = m_Context->getQueueFamilyIndices().graphicsFamilyIndex.value();

    vk::BufferCreateInfo bufferCreateInfo{
        {},  //flags
        vk::DeviceSize{sizeof(UniformBufferObject)},  // size
        vk::BufferUsageFlagBits::eUniformBuffer, // usage flags
        vk::SharingMode::eExclusive,   //sharing mode
        1, //queueFamilyIndex count
        &queueFamilyIndex,   //pQueueFamiyIndices
        nullptr  //pNext
    };

    VmaAllocationCreateInfo allocCreateInfo {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; //set this so that I can get buffer bits on Updating buffer

    //allocate uniform buffers
    m_uniformBuffers.resize(descriptorCount);
    for(size_t i = 0; i < descriptorCount; ++i) {
        BufferWrapper buffer = m_BufferPool->allocateMemory(bufferCreateInfo, allocCreateInfo);
        m_uniformBuffers[i] = buffer;
    }

    //set uniform buffer object
    m_uniformBufferObjects.resize(descriptorCount);

  }

  BufferWrapper getUniformBuffer(uint32_t index) const {
    return m_uniformBuffers[index];
  }

  void OnUpdate(uint32_t index) {
    memcpy(m_uniformBuffers[index].allocationInfo.pMappedData, &(m_uniformBufferObjects[index]), sizeof(UniformBufferObject));
  }

  void OnDestroy() {
    for(const auto &uniformBuffer : m_uniformBuffers) {
        m_BufferPool->freeBuffer(uniformBuffer);
    }
    m_uniformBufferObjects.clear();
  }

  std::vector<UniformBufferObject>& getUniformBufferObjects() {
    return m_uniformBufferObjects;
  }

  void SetUniformBuffer(const UniformBufferObject& object, uint32_t index) {
    assert(index >= 0 && index < m_descriptorCount);
    m_uniformBufferObjects[index] = object;
  }


  vk::DescriptorSetLayoutBinding getDescriptorLayoutBinding() const {
    return m_layoutBinding;
  }

private:
  vk::DescriptorSetLayoutBinding m_layoutBinding;
  uint32_t m_descriptorCount = 0;

  std::vector<UniformBufferObject> m_uniformBufferObjects;
  std::vector<BufferWrapper> m_uniformBuffers;

  VkContext *m_Context;
  BufferPool *m_BufferPool;
};
}; // namespace hiddenpiggy
#endif