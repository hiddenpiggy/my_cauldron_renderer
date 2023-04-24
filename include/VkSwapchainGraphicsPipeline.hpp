#ifndef SWAPCHAIN_GRAPHICS_PIPELINE_HPP
#define SWAPCHAIN_GRAPHICS_PIPELINE_HPP
#include "VkPipelineBase.hpp"
#include "VkShaderModuleFactory.hpp"
#include "vulkan/vulkan.hpp"
#include "VkSwapchain.hpp"
namespace hiddenpiggy {
class VkSwapchainGraphicsPipeline : public VkPipelineBase {
public:
  VkSwapchainGraphicsPipeline(vk::Device device,
                              vk::PipelineLayout pipelineLayout,
                              vk::RenderPass renderPass,
                              VkSwapchain* pSwapchain)
      : VkPipelineBase(device, pipelineLayout, renderPass) {
        m_pSwapchain = pSwapchain;
      }
    void OnCreate() override;
    void OnDestroy() override;
    vk::Pipeline getPipeline() override;
private:
    void createShaderStages();

    vk::ShaderModule m_vertexModule;
    vk::ShaderModule m_fragmentModule;


    VkSwapchain *m_pSwapchain;
};
} // namespace hiddenpiggy

#endif