#ifndef VULKAN_BUFFERPOOL_HPP
#define VULKAN_BUFFERPOOL_HPP

#include "VkContext.hpp"
#include "vk_mem_alloc.h"
#include <vector>
#include <functional>
#include <unordered_set>

namespace hiddenpiggy {

//struct that wraps buffer and its allocation data
struct BufferWrapper {
  vk::Buffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocationInfo;

  bool operator==(const BufferWrapper& other) const {
    return buffer==other.buffer && allocation == other.allocation;
  }
};

//hash function used for std::unordered_set use
struct BufferWrapperHash {
  std::size_t operator()(const BufferWrapper& obj) const {
    VkBuffer buffer = obj.buffer;
    uint64_t buffer_value =  reinterpret_cast<uint64_t>(buffer);
    uint64_t allocation =  reinterpret_cast<uint64_t>(obj.allocation);
    std::hash<uint64_t> intHash;
    return intHash(buffer_value) ^ intHash(allocation);
  }
};

// same as above
struct ImageWrapper {
  vk::Image image;
  VmaAllocation allocation;
  VmaAllocationInfo allocationInfo;

  bool operator==(const ImageWrapper& other) const {
    return image==other.image && allocation == other.allocation;
  }
};

struct ImageHash {
    std::size_t operator()(const ImageWrapper& obj) const {
    VkImage image = obj.image;
    uint64_t image_value =  reinterpret_cast<uint64_t>(image);
    uint64_t allocation =  reinterpret_cast<uint64_t>(obj.allocation);
    std::hash<uint64_t> intHash;
    return intHash(image_value) ^ intHash(allocation);
  }
};

class BufferPool {
public:
  BufferPool(VkContext *context) : m_pContext(context) {}

  VmaAllocator getAllocator() const {
    return m_allocator;
  }

  uint32_t getSize() const { return m_buffers.size(); }

  void OnCreate() {
    //prepare for vulkan functions
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo {};
    allocatorCreateInfo.physicalDevice = m_pContext->getPhysicalDevice();
    allocatorCreateInfo.device = m_pContext->getDevice();
    allocatorCreateInfo.instance = m_pContext->getInstance();
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    assert(vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) ==
           VK_SUCCESS);

  }

  BufferWrapper allocateMemory(vk::BufferCreateInfo& bufferCreateInfo,
                      VmaAllocationCreateInfo& allocCreateInfo) {
    VkBuffer buffer;

    //convert cpp createinfo to c version
    VkBufferCreateInfo vkBufferCreateInfo = {};
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = nullptr;
    vkBufferCreateInfo.flags =
        static_cast<VkBufferCreateFlags>(bufferCreateInfo.flags);
    vkBufferCreateInfo.size = bufferCreateInfo.size;
    vkBufferCreateInfo.usage =
        static_cast<VkBufferUsageFlags>(bufferCreateInfo.usage);
    vkBufferCreateInfo.sharingMode =
        static_cast<VkSharingMode>(bufferCreateInfo.sharingMode);
    vkBufferCreateInfo.queueFamilyIndexCount =
        bufferCreateInfo.queueFamilyIndexCount;
    vkBufferCreateInfo.pQueueFamilyIndices =
        bufferCreateInfo.pQueueFamilyIndices;

    VmaAllocation vmaAllocation;
    VmaAllocationInfo allocInfo;
    assert(vmaCreateBuffer(m_allocator, &vkBufferCreateInfo, &allocCreateInfo,
                           &buffer, &vmaAllocation, &allocInfo) == VK_SUCCESS);

    BufferWrapper wrapper{ buffer, vmaAllocation, allocInfo }; 
    m_buffers.insert(wrapper);

    return wrapper;
  }

  ImageWrapper allocateMeomryForImage(vk::ImageCreateInfo &imageInfo, VmaAllocationCreateInfo& allocInfo) {
    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    
    //covert cpp createinfo to c version
    VkImageCreateInfo vkImageInfo = {};
    vkImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageInfo.pNext = imageInfo.pNext;

    // Copy fields
    vkImageInfo.flags = static_cast<VkImageCreateFlags>(imageInfo.flags);
    vkImageInfo.imageType = static_cast<VkImageType>(imageInfo.imageType);
    vkImageInfo.format = static_cast<VkFormat>(imageInfo.format);
    vkImageInfo.extent.width = imageInfo.extent.width;
    vkImageInfo.extent.height = imageInfo.extent.height;
    vkImageInfo.extent.depth = imageInfo.extent.depth;
    vkImageInfo.mipLevels = imageInfo.mipLevels;
    vkImageInfo.arrayLayers = imageInfo.arrayLayers;
    vkImageInfo.samples = static_cast<VkSampleCountFlagBits>(imageInfo.samples);
    vkImageInfo.tiling = static_cast<VkImageTiling>(imageInfo.tiling);
    vkImageInfo.usage = static_cast<VkImageUsageFlags>(imageInfo.usage);
    vkImageInfo.sharingMode = static_cast<VkSharingMode>(imageInfo.sharingMode);
    vkImageInfo.queueFamilyIndexCount = static_cast<uint32_t>(imageInfo.queueFamilyIndexCount);
    vkImageInfo.pQueueFamilyIndices = imageInfo.pQueueFamilyIndices;
    vkImageInfo.initialLayout = static_cast<VkImageLayout>(imageInfo.initialLayout);
    assert(vmaCreateImage(m_allocator, &vkImageInfo, &allocInfo, &image, &allocation, &allocationInfo) == VK_SUCCESS);

    ImageWrapper imageWrapper = {image, allocation, allocationInfo};
    m_images.insert(imageWrapper);

    return imageWrapper;
  }

  void freeBuffer(const BufferWrapper& buffer) {
    vmaDestroyBuffer(m_allocator,  buffer.buffer, buffer.allocation);
    auto it = m_buffers.find(buffer);

    //if buffer found
    if(it != m_buffers.end()) {
      m_buffers.erase(it);
    }
    //if not found, do nothing
  }

  void freeImage(const ImageWrapper& image) {
    vmaDestroyImage(m_allocator, image.image, image.allocation);
    auto it = m_images.find(image);

    //if buffer found
    if(it != m_images.end()) {
      m_images.erase(it);
    }
  }

  void OnDestroy() {

    for(const auto &buffer: m_buffers) {
      vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
    }

    for(const auto &image : m_images) {
      vmaDestroyImage(m_allocator, image.image, image.allocation);
    }

    m_buffers.clear();
    m_images.clear();
    vmaDestroyAllocator(m_allocator);
  }

private:
  VkContext *m_pContext;
  VmaAllocator m_allocator;
  std::unordered_set<BufferWrapper, BufferWrapperHash> m_buffers;
  std::unordered_set<ImageWrapper, ImageHash> m_images;
};
} // namespace hiddenpiggy

#endif