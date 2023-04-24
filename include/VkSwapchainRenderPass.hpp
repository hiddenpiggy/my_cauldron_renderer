#ifndef VK_SWAPCHAIN_RENDER_PASS
#define VK_SWAPCHAIN_RENDER_PASS

#include "VkContext.hpp"
#include "VkRenderPass.hpp"
#include "VkSwapchain.hpp"
#include "Vulkan-Headers/include/vulkan/vulkan_core.h"
#include "Vulkan-Headers/include/vulkan/vulkan_handles.hpp"
namespace hiddenpiggy {
class SwapchainRenderPass : public VKRenderPass {
public:
  SwapchainRenderPass(VkContext *context, VkSwapchain *pSwapchain)
      : m_context(context), m_pSwapchain(pSwapchain) {}
  void OnCreate() override;
  void OnDestroy() override;
  void OnExecuteSubpass(uint32_t subpassIndex) override;

  VkContext *m_context = nullptr;
  VkSwapchain *m_pSwapchain = nullptr;

  vk::RenderPass getRenderPass();

private:
  vk::RenderPass m_RenderPass;
};
} // namespace hiddenpiggy

#endif
