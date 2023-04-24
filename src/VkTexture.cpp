#include "VkTexture.hpp"
#include "ResourceUploadHeap.hpp"
#include "VkBufferPool.hpp"
#include "stb_image.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"

namespace hiddenpiggy {
    void VulkanTexture::OnCreate(const std::string filename) {
        assert(m_pContext!= nullptr && m_pBufferPool != nullptr);

        //get device handle
        vk::Device device = m_pContext->getDevice();

        // Load texture image data from file
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        // Create Image Object
        vk::ImageCreateInfo imageinfo{
            {},  //flags
            vk::ImageType::e2D,  //ImageType
            vk::Format::eR8G8B8A8Srgb,  //Format
            {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}, //Extent 
            1,                   //miplevels
            1,                //arrayLayers
            vk::SampleCountFlagBits::e1,  //Samples
            vk::ImageTiling::eOptimal,  //imagetiling
            vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,  //usage
            vk::SharingMode::eExclusive
        };

        //set initial layout to undefined
        imageinfo.initialLayout = vk::ImageLayout::eUndefined;

        // Create image Memory
        VmaAllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        m_image = m_pBufferPool->allocateMeomryForImage(imageinfo, allocCreateInfo);


        // transition image layout
        m_pResourceUploadHeap->transitionImageLayout(m_image.image, imageinfo.format, vk::ImageLayout::eUndefined,  vk::ImageLayout::eTransferDstOptimal);

        // Upload data to the GPU memory
        m_pResourceUploadHeap->uploadImageData(pixels, imageSize, texWidth, texHeight, m_image.image, m_image.allocation);

        // transfer data to shader read
        m_pResourceUploadHeap->transitionImageLayout(m_image.image, imageinfo.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        // create Image view
        vk::ImageViewCreateInfo viewinfo{
            {},  //flags
            m_image.image, //image
            vk::ImageViewType::e2D, //image view type
            vk::Format::eR8G8B8A8Srgb,  //format
            {},                          //components
            {
                vk::ImageAspectFlagBits::eColor,
                0,  //base mip level
                1, //level count
                0,  //base array layer
                1 //layer count
            } //subresourceRange
        };

        m_imageView = device.createImageView(viewinfo);

        // create sampler
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = false;
        samplerInfo.maxAnisotropy = 1.0f;


        m_sampler = device.createSampler(samplerInfo);
    }

    void VulkanTexture::OnDestroy() {
        vk::Device device = m_pContext->getDevice();
        device.destroyImageView(m_imageView);
        device.destroySampler(m_sampler);
        m_pBufferPool->freeImage(m_image);
    }

    vk::ImageView VulkanTexture::getImageView() const {
        return m_imageView;
    }

    vk::Sampler VulkanTexture::getSampler() const {
        return m_sampler;
    }
}