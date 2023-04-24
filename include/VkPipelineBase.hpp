#ifndef VK_PIPELINE_HPP
#define VK_PIPELINE_HPP
#include "vulkan/vulkan.hpp"
namespace hiddenpiggy {
class VkPipelineBase {
public:
    VkPipelineBase(vk::Device device, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass)
                : m_device(device), m_pipelineLayout(pipelineLayout), m_RenderPass(renderPass) {}
    
    virtual ~VkPipelineBase() {}
    virtual void OnCreate() {}
    virtual void OnDestroy() {}
    virtual vk::Pipeline getPipeline() { return m_pipeline; }
protected:
    vk::Device m_device;
    vk::Pipeline m_pipeline;
    vk::PipelineLayout m_pipelineLayout;
    vk::RenderPass  m_RenderPass;
    std::vector<vk::PipelineShaderStageCreateInfo> m_shaderStages;
};
}
#endif