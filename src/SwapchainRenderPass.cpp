#include "VkSwapchainRenderPass.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"
namespace hiddenpiggy {
void SwapchainRenderPass::OnCreate() {
  assert(m_pSwapchain != nullptr);
  // setup color attachment
  vk::AttachmentDescription colorAttachment{
      {},
      m_pSwapchain->getFormat(),
      VULKAN_HPP_NAMESPACE::SampleCountFlagBits::e1,
      VULKAN_HPP_NAMESPACE::AttachmentLoadOp::eClear,
      VULKAN_HPP_NAMESPACE::AttachmentStoreOp::eStore,
      VULKAN_HPP_NAMESPACE::AttachmentLoadOp::eDontCare,
      VULKAN_HPP_NAMESPACE::AttachmentStoreOp::eDontCare,
      VULKAN_HPP_NAMESPACE::ImageLayout::eUndefined,
      VULKAN_HPP_NAMESPACE::ImageLayout::ePresentSrcKHR};
  m_attachments.push_back(colorAttachment);

  vk::AttachmentReference colorAttachmentRef{
      0, vk::ImageLayout::eColorAttachmentOptimal};

  // setup subpass description
  vk::SubpassDescription subpass{
      {},      VULKAN_HPP_NAMESPACE::PipelineBindPoint::eGraphics,
      0,       nullptr,
      1,       &colorAttachmentRef,
      nullptr, nullptr,
      0,       nullptr};

  m_subpasses.push_back(subpass);

  // setup subpass dependency
  vk::SubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask =
      VULKAN_HPP_NAMESPACE::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.srcAccessMask = {};
  dependency.dstStageMask =
      VULKAN_HPP_NAMESPACE::PipelineStageFlagBits::eColorAttachmentOutput;
  dependency.dstAccessMask =
      VULKAN_HPP_NAMESPACE::AccessFlagBits::eColorAttachmentWrite;
  m_dependencies.push_back(dependency);

  // Create the Vulkan render pass
  vk::RenderPassCreateInfo renderPassInfo;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(m_attachments.size());
  renderPassInfo.pAttachments = m_attachments.data();
  renderPassInfo.subpassCount = static_cast<uint32_t>(m_subpasses.size());
  renderPassInfo.pSubpasses = m_subpasses.data();
  renderPassInfo.dependencyCount = static_cast<uint32_t>(m_dependencies.size());
  renderPassInfo.pDependencies = m_dependencies.data();

  assert(m_context != nullptr && m_pSwapchain != nullptr);
  m_RenderPass = m_context->getDevice().createRenderPass(renderPassInfo);
};

void SwapchainRenderPass::OnExecuteSubpass(uint32_t subpassIndex) {}
void SwapchainRenderPass::OnDestroy() {
  vk::Device device = m_context->getDevice();
  if (m_RenderPass) {
    device.destroyRenderPass(m_RenderPass);
    m_RenderPass = nullptr;
  }
  m_attachments.clear();
  m_subpasses.clear();
  m_dependencies.clear();
}

vk::RenderPass SwapchainRenderPass::getRenderPass() { return m_RenderPass; }

} // namespace hiddenpiggy
