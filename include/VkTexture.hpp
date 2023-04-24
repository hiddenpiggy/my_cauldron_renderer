#ifndef VULKAN_TEXTURE_HPP
#define VULKAN_TEXTURE_HPP

#include "ResourceUploadHeap.hpp"
#include "VkContext.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"
namespace hiddenpiggy {
class VulkanTexture {
public:
    VulkanTexture(VkContext *pContext, BufferPool *pBufferPool, ResourceUploadHeap *pResourceUploadHeap) : m_pContext(pContext), m_pBufferPool(pBufferPool), m_pResourceUploadHeap(pResourceUploadHeap) {}

    void OnCreate(const std::string filename);
    void OnDestroy();

    vk::ImageView getImageView() const;
    vk::Sampler getSampler() const;
private:
    VkContext *m_pContext;
    BufferPool *m_pBufferPool;
    ResourceUploadHeap *m_pResourceUploadHeap;
    ImageWrapper m_image;
    vk::ImageView m_imageView;
    vk::Sampler m_sampler;
};
}
#endif