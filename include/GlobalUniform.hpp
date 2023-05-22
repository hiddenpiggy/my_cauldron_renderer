#ifndef GLOBAL_UNIFORM_HPP
#define GLOBAL_UNIFORM_HPP
#include <vulkan/vulkan.hpp>
#include <VkBufferPool.hpp>
#include <glm/glm.hpp>

namespace hiddenpiggy {
class GlobalUniformBuffer {

    BufferWrapper buffer;

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    } ubo;

    void allocateBuffer(VkContext *context, BufferPool *bufferPool) {
        vk::BufferCreateInfo bufferCreateInfo {
            {}, sizeof(ubo), vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive,  
            context->getQueueFamilyIndices().graphicsFamilyIndex.value(), nullptr
        };

        VmaAllocationCreateInfo vmaAllocCreateInfo{};
        vmaAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        bufferPool->allocateMemory(bufferCreateInfo, vmaAllocCreateInfo);
    }

    BufferWrapper getBufferWrapper() const {
        return this->buffer;
    }

    void updateMatrices(const glm::mat4 &model, const glm::mat4& view, const glm::mat4& proj) {
        ubo.model = model;
        ubo.view = view;
        ubo.proj = proj;
    }

    void uploadData()  {
        memcpy(buffer.allocationInfo.pMappedData, &this->ubo, sizeof(UniformBufferObject));
    }
};
}
#endif