#include "VkSwapchainGraphicsPipeline.hpp"
#include "Model.hpp"
#include "VkShaderModuleFactory.hpp"
#include "vulkan/vulkan_enums.hpp"

namespace hiddenpiggy {
void VkSwapchainGraphicsPipeline::createShaderStages() {
  m_vertexModule = VkShaderModuleFactory::CreateShaderModule(
      m_device,
      (std::string{SHADERS_PATH} + std::string{"swapchain_vert.spv"}).c_str());
  m_fragmentModule = VkShaderModuleFactory::CreateShaderModule(
      m_device,
      (std::string{SHADERS_PATH} + std::string{"swapchain_frag.spv"}).c_str());

  vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
  vertShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageCreateInfo.module = m_vertexModule;
  vertShaderStageCreateInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
  fragShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageCreateInfo.module = m_fragmentModule;
  fragShaderStageCreateInfo.pName = "main";

  m_shaderStages.push_back(vertShaderStageCreateInfo);
  m_shaderStages.push_back(fragShaderStageCreateInfo);
}

void VkSwapchainGraphicsPipeline::OnCreate() {
  // create shader stages
  createShaderStages();

  // Vertex input state
  vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
  vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
  //vertexInputStateCreateInfo.setVertexBindingDescriptionCount(0);
  //vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(0);
  auto bindingDescription = Vertex::getBindingDescription();
  vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputStateCreateInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(Vertex::getAttributeDescriptions().size());
  vertexInputStateCreateInfo.pVertexAttributeDescriptions =
         Vertex::getAttributeDescriptions().data();

  // Input assembly state
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo;
  inputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
  inputAssemblyStateCreateInfo.primitiveRestartEnable = false;

  // Viewport and scissor state
  vk::Viewport viewport;
  // Configure viewport based on your framebuffer size
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  auto extent = m_pSwapchain->getExtent();
  viewport.width = extent.width;
  viewport.height = extent.height;

  vk::Rect2D scissor;
  // Configure scissor based on your framebuffer size
  scissor.setExtent(extent);
  scissor.setOffset({0, 0});

  vk::PipelineViewportStateCreateInfo viewportStateCreateInfo;
  viewportStateCreateInfo.viewportCount = 1;
  viewportStateCreateInfo.pViewports = &viewport;
  viewportStateCreateInfo.scissorCount = 1;
  viewportStateCreateInfo.pScissors = &scissor;

  // Rasterization state
  vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
  rasterizationStateCreateInfo.depthClampEnable = false;
  rasterizationStateCreateInfo.rasterizerDiscardEnable = false;
  rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
  rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
  rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
  rasterizationStateCreateInfo.depthBiasEnable = false;

  // Multisampling state
  vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
  multisampleStateCreateInfo.sampleShadingEnable = false;
  multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

  // Depth and stencil state
  vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
  // Configure depth and stencil state based on your requirements
  depthStencilStateCreateInfo.setDepthBoundsTestEnable(false);

  // Color blend state
  vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
  // Configure color blend attachment state based on your requirements
  colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
  colorBlendAttachmentState.blendEnable = VK_FALSE;



  vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
  colorBlendStateCreateInfo.logicOpEnable = false;
  colorBlendStateCreateInfo.attachmentCount = 1;
  colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;

  // Dynamic states (if needed)
  // ...
  const std::vector<vk::DynamicState> dynamicStates{
      vk::DynamicState::eViewport, vk::DynamicState::eScissor,
      vk::DynamicState::eLineWidth};

  vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo {
    {}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data(),
        nullptr
  };

// Create graphics pipeline
vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
pipelineCreateInfo.pStages = m_shaderStages.data();
pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
pipelineCreateInfo.layout = m_pipelineLayout;
pipelineCreateInfo.renderPass = m_RenderPass;
pipelineCreateInfo.subpass = 0;

auto res = m_device.createGraphicsPipeline({}, pipelineCreateInfo);
assert(res.result == vk::Result::eSuccess);
m_pipeline = res.value;

m_device.destroyShaderModule(m_vertexModule, nullptr);
m_device.destroyShaderModule(m_fragmentModule, nullptr);
}

void VkSwapchainGraphicsPipeline::OnDestroy() {
  m_device.destroyPipeline(m_pipeline);
}

vk::Pipeline VkSwapchainGraphicsPipeline::getPipeline() { return m_pipeline; }

} // namespace hiddenpiggy