#ifndef RESOURCE_UPLOAD_HEAP_HPP
#define RESOURCE_UPLOAD_HEAP_HPP
#include "VkBufferPool.hpp"
#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace hiddenpiggy {
class ResourceUploadHeap {
public:
  ResourceUploadHeap(VkContext *context, BufferPool *bufferPool)
      : m_context(context), m_bufferPool(bufferPool) {}

  void OnCreate() {
    // create single shot command pool
    auto queueFamilyIndex =
        m_context->getQueueFamilyIndices().graphicsFamilyIndex;
    vk::CommandPoolCreateInfo commandPoolCreateInfo(
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex.value(), nullptr);
    
    m_commandPool = m_context->getDevice().createCommandPool(commandPoolCreateInfo);

    vk::CommandBufferAllocateInfo cmdBufAllocInfo(
        m_commandPool, vk::CommandBufferLevel::ePrimary, 1);
    auto commandBuffers =
        m_context->getDevice().allocateCommandBuffers(cmdBufAllocInfo);
    m_commandBuffer = std::move(commandBuffers[0]);

    vk::FenceCreateInfo fenceCreateInfo({}, nullptr);

    m_fence = m_context->getDevice().createFence(fenceCreateInfo);
  }

  void uploadBufferData(const void *data, uint32_t size,
                        vk::Buffer &destinationBuffer,
                        VmaAllocation destinationAllocation, uint32_t offset) {
    // Create a staging buffer and allocate memory for it
    vk::BufferCreateInfo stagingBufferCreateInfo(
        {}, size, vk::BufferUsageFlagBits::eTransferSrc);
    VmaAllocationCreateInfo stagingBufferAllocCreateInfo = {};
    stagingBufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBufferAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    BufferWrapper stagingBuffer = m_bufferPool->allocateMemory(stagingBufferCreateInfo,
                                 stagingBufferAllocCreateInfo);

    VmaAllocator allocator = m_bufferPool->getAllocator();

    // get new allocation info
    VmaAllocationInfo stagingBufferAllocInfo{};
    vmaGetAllocationInfo(allocator, stagingBuffer.allocation, &stagingBufferAllocInfo);
    memcpy(stagingBufferAllocInfo.pMappedData, data, size);

    // Create a command buffer and begin recording
    vk::CommandBufferBeginInfo beginInfo(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    m_commandBuffer.begin(beginInfo);

    // Record the copy command
    vk::BufferCopy bufferCopy(0, offset, size);
    m_commandBuffer.copyBuffer(stagingBuffer.buffer, destinationBuffer, bufferCopy);

    // End recording and submit the command buffer
    m_commandBuffer.end();
    vk::SubmitInfo submitInfo({}, {}, m_commandBuffer);
    m_context->getGraphicsQueue().submit(submitInfo, m_fence);

   
    vk::ArrayProxy<vk::Fence> fences{
      m_fence
    };

    // Wait for the command buffer to finish executing
    m_context->getDevice().waitForFences(fences, VK_TRUE, UINT64_MAX);

    // fence must be unsignaled after execution
    m_context->getDevice().resetFences(fences);

    // reset command buffer
    m_commandBuffer.reset();

    // Free the staging buffer
    m_bufferPool->freeBuffer(stagingBuffer);
  }

  void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
    m_commandBuffer.reset();
    vk::CommandBufferBeginInfo beginInfo(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    m_commandBuffer.begin(beginInfo);

    vk::ImageMemoryBarrier barrier{
      {}, //src access mask
      {}, //dst access mask
      oldLayout,
      newLayout,
      VK_QUEUE_FAMILY_IGNORED, //srcQueueFamilyIndex
      VK_QUEUE_FAMILY_IGNORED, //dstQueueFamilyIndex
      image,     //image
      {
        vk::ImageAspectFlagBits::eColor,
        0,   //base miplevel
        1,   //level count
        0,     //base array layer
        1 //layer count
      }, //subresourceRange
      nullptr  //pNext
    };


    vk::PipelineStageFlags sourceStage{};
    vk::PipelineStageFlags destinationStage{};

    if(oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
      barrier.srcAccessMask = {};
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

      sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
      destinationStage = vk::PipelineStageFlagBits::eTransfer;
    } else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

      sourceStage = vk::PipelineStageFlagBits::eTransfer;
      destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    } else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eTransferSrcOptimal) {
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
      sourceStage = vk::PipelineStageFlagBits::eTransfer;
      destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else {
      throw std::invalid_argument("unsupported layout transition!");
    }

    m_commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);
    m_commandBuffer.end();
    
    vk::SubmitInfo submitInfo{
      {}, {}, m_commandBuffer
    };

    m_context->getGraphicsQueue().submit(submitInfo, m_fence);

        vk::ArrayProxy<vk::Fence> fences{
      m_fence
    };

    // Wait for the command buffer to finish executing
    m_context->getDevice().waitForFences(fences, VK_TRUE, UINT64_MAX);

    // fence must be unsignaled after execution
    m_context->getDevice().resetFences(fences);

    // reset command buffer
    m_commandBuffer.reset();
  }

  void uploadImageData(const void *data, uint32_t size, uint32_t width, uint32_t height,
                        vk::Image &destinationImage,
                        VmaAllocation destinationAllocation) {
    // Create a staging buffer and allocate memory for it
    vk::BufferCreateInfo stagingBufferCreateInfo(
        {}, size, vk::BufferUsageFlagBits::eTransferSrc);
    VmaAllocationCreateInfo stagingBufferAllocCreateInfo = {};
    stagingBufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingBufferAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    BufferWrapper stagingBuffer = m_bufferPool->allocateMemory(stagingBufferCreateInfo,
                                 stagingBufferAllocCreateInfo);

    VmaAllocator allocator = m_bufferPool->getAllocator();

    // get new allocation info
    VmaAllocationInfo stagingBufferAllocInfo{};
    vmaGetAllocationInfo(allocator, stagingBuffer.allocation, &stagingBufferAllocInfo);
    memcpy(stagingBufferAllocInfo.pMappedData, data, size);

    // Create a command buffer and begin recording
    vk::CommandBufferBeginInfo beginInfo(
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    m_commandBuffer.begin(beginInfo);

    // Record the copy command
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D(0, 0, 0);
    region.imageExtent = vk::Extent3D(width, height, 1);

    m_commandBuffer.copyBufferToImage(stagingBuffer.buffer, destinationImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);

    // End recording and submit the command buffer
    m_commandBuffer.end();
    vk::SubmitInfo submitInfo({}, {}, m_commandBuffer);
    m_context->getGraphicsQueue().submit(submitInfo, m_fence);

   
    vk::ArrayProxy<vk::Fence> fences{
      m_fence
    };

    // Wait for the command buffer to finish executing
    m_context->getDevice().waitForFences(fences, VK_TRUE, UINT64_MAX);

    // fence must be unsignaled after execution
    m_context->getDevice().resetFences(fences);

    // reset command buffer
    m_commandBuffer.reset();

    // Free the staging buffer
    m_bufferPool->freeBuffer(stagingBuffer);
  }

  

  void OnDestroy() {
    vk::Device device = m_context->getDevice();
    device.destroyFence(m_fence);
    device.freeCommandBuffers(m_commandPool, m_commandBuffer);
    device.destroyCommandPool(m_commandPool);
  }

private:
  VkContext *m_context;
  BufferPool *m_bufferPool;
  vk::CommandPool m_commandPool;
  vk::CommandBuffer m_commandBuffer;
  vk::Fence m_fence;
};
} // namespace hiddenpiggy
#endif