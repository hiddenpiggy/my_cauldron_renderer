#include "VkSwapchainFramebuffers.hpp"
#include "vulkan/vulkan_enums.hpp"

namespace hiddenpiggy {
void VkSwapchainFramebuffers::OnCreate() {
  assert(m_swapchain != nullptr &&
         m_swapchainRenderPass != nullptr);
  uint32_t imageCount = m_swapchain->getImageCount();
  vk::Extent2D extent = m_swapchain->getExtent();
  vk::RenderPass renderPass = m_swapchainRenderPass->getRenderPass();
  m_framebuffers.resize(imageCount);

  for (size_t i = 0; i < imageCount; ++i) {
    std::vector<vk::ImageView> attachments = {m_swapchain->getImageView(i)};
    vk::FramebufferCreateInfo createInfo{};
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    createInfo.pAttachments = attachments.data();
    createInfo.width = extent.width;
    createInfo.height = extent.height;
    createInfo.layers = 1;
    createInfo.pNext = nullptr;
    m_framebuffers[i] = m_device.createFramebuffer(createInfo);
  }
}

void VkSwapchainFramebuffers::OnDestroy() {
    for(int i = 0; i < m_framebuffers.size(); ++i) {
        m_device.destroyFramebuffer(m_framebuffers[i]);
    }
}
} // namespace hiddenpiggy