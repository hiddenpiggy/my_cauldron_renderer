#include "Renderer.hpp"
#include "GLFW/glfw3.h"
#include "ResourceUploadHeap.hpp"
#include "UniformBuffers.hpp"
#include "VkBufferPool.hpp"
#include "VkCommandBuffers.hpp"
#include "VkShaderModuleFactory.hpp"
#include "VkSwapchain.hpp"
#include "VkSwapchainFramebuffers.hpp"
#include "VkSwapchainGraphicsPipeline.hpp"
#include "VkSwapchainRenderPass.hpp"
#include "VkTexture.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glTFScene.hpp"
#include "App.hpp"

namespace hiddenpiggy {
void Renderer::OnCreate(const std::string AppName, uint32_t width,
                        uint32_t height, GLFWwindow *pWindow) {
  m_AppName = AppName;
  m_width = width;
  m_height = height;
  m_pWindow = pWindow;

  // create vulkan context and swapchain
  m_Context = new VkContext();
  m_Context->OnCreate(m_AppName);
  m_swapchain.OnCreate(m_Context, pWindow, width, height);

  // setup buffer utils
  m_pBufferPool = new BufferPool(m_Context);
  m_pBufferPool->OnCreate();

  // setup resource uploadheaps
  m_pResourceUploadHeap = new ResourceUploadHeap(m_Context, m_pBufferPool);
  m_pResourceUploadHeap->OnCreate();

  // loading textures
  m_textures.resize(1);
  m_textures[0] =
      new VulkanTexture(m_Context, m_pBufferPool, m_pResourceUploadHeap);
  std::string texturePath{TEXTURES_PATH};
  texturePath += "texture.jpg";
  m_textures[0]->OnCreate(texturePath);

  // create swapchain renderpass
  assert(m_pSwapchainRenderPass == nullptr);
  m_pSwapchainRenderPass = new SwapchainRenderPass(m_Context, &m_swapchain);
  m_pSwapchainRenderPass->OnCreate();

  // create swapchain framebuffers
  vk::Device device = m_Context->getDevice();
  m_pFramebuffers =
      new VkSwapchainFramebuffers(device, &m_swapchain, m_pSwapchainRenderPass);
  m_pFramebuffers->OnCreate();

  // setup uniform buffers
  m_pUniformBuffers = new UniformBuffers(m_Context, m_pBufferPool);
  m_pUniformBuffers->OnCreate(m_swapchain.getImageCount());

  //
  // setup swapchain resource binding
  // create descriptor pool
  vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
  std::array<vk::DescriptorPoolSize, 2> poolSizes = {
      vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,
                             m_swapchain.getImageCount()),

      vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,
                             m_swapchain.getImageCount())};

  descriptorPoolCreateInfo.setPoolSizeCount(
      static_cast<uint32_t>(poolSizes.size())); // Set pool size count
  descriptorPoolCreateInfo.setPPoolSizes(poolSizes.data()); // Set pool sizes
  descriptorPoolCreateInfo.setMaxSets(
      static_cast<uint32_t>(m_swapchain.getImageCount()));
  m_swapchainResourceBinding.m_descriptorPool =
      device.createDescriptorPool(descriptorPoolCreateInfo);

  // create descriptorSetLayout
  std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(poolSizes.size());

  layoutBindings[0] = m_pUniformBuffers->getDescriptorLayoutBinding();
  layoutBindings[1] = vk::DescriptorSetLayoutBinding{
      1, vk::DescriptorType::eCombinedImageSampler, 1,
      vk::ShaderStageFlagBits::eFragment};

  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(
      vk::DescriptorSetLayoutCreateFlags(),         // Flags
      static_cast<uint32_t>(layoutBindings.size()), // Binding count
      layoutBindings.data()                         // Pointer to bindings
  );

  m_swapchainResourceBinding.m_descriptorSetLayouts.resize(
      static_cast<uint32_t>(m_swapchain.getImageCount()));

  for (size_t i = 0;
       i < m_swapchainResourceBinding.m_descriptorSetLayouts.size(); ++i) {
    m_swapchainResourceBinding.m_descriptorSetLayouts[i] =
        device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
    ; // Create descriptor set layout
  }

  // create descriptorsets
  vk::DescriptorSetAllocateInfo descriptorAllocateInfo{
      m_swapchainResourceBinding.m_descriptorPool, // descriptorPool
      static_cast<uint32_t>(m_swapchainResourceBinding.m_descriptorSetLayouts
                                .size()), // descriptorSetCount
      m_swapchainResourceBinding.m_descriptorSetLayouts
          .data(), // pDescriptorSetLayouts
      nullptr      // pNext
  };

  m_swapchainResourceBinding.m_descriptorSets =
      device.allocateDescriptorSets(descriptorAllocateInfo);

  // setup binding to uniform buffers and image buffer
  for (size_t i = 0; i < m_swapchainResourceBinding.m_descriptorSets.size();
       ++i) {
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = (m_pUniformBuffers->getUniformBuffer(i)).buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    vk::DescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfo.imageView = m_textures[0]->getImageView();
    imageInfo.sampler = m_textures[0]->getSampler();

    std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].dstSet = m_swapchainResourceBinding.m_descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;

    descriptorWrites[1].dstSet = m_swapchainResourceBinding.m_descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType =
        vk::DescriptorType::eCombinedImageSampler;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()),
                                descriptorWrites.data(), 0, nullptr);
  }

  // setup pipelinelayout of swapchain
  vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      {}, // flags
      static_cast<uint32_t>(m_swapchainResourceBinding.m_descriptorSetLayouts
                                .size()), // setlayoutCount
      m_swapchainResourceBinding.m_descriptorSetLayouts.data(), // pSetLayouts
      0U,      // push constant range count
      nullptr, // pPushConstantRanges
      nullptr  // pNext
  };

  m_swapchainResourceBinding.m_pipelineLayout =
      device.createPipelineLayout(pipelineLayoutCreateInfo);

  // setup graphics pipeline
  m_swapchainPipeline = new VkSwapchainGraphicsPipeline(
      device, m_swapchainResourceBinding.m_pipelineLayout,
      m_pSwapchainRenderPass->getRenderPass(), &m_swapchain);

  m_swapchainPipeline->OnCreate();

  // setup command buffers
  m_pCommandBuffers = new VkCommandBuffers(
      device, m_Context->getGraphicsQueue(),
      m_Context->getQueueFamilyIndices().graphicsFamilyIndex.value());
  m_pCommandBuffers->OnCreate(1);

  //setup camera
  m_cameras.push_back(
    Camera(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f))
  );

  m_cameras[0].setPerspectiveParameters(45.0f, 0.1f, 10.0f, (float)m_swapchain.getExtent().width /
                                  (float)m_swapchain.getExtent().height);

  //glTF model
  glTFModel model{};
  std::string ScenePath {SCENES_PATH};
  ScenePath += "Box/Box.gltf";
  model.loadModel(ScenePath.c_str());
  model.AllocateBuffersAndUpload(m_pBufferPool, m_pResourceUploadHeap, m_Context->getQueueFamilyIndices().graphicsFamilyIndex.value());

  m_models.push_back(model);

  //setup UI
  m_ui = new UI();
  m_ui->OnCreate(m_Context,  m_pWindow, m_pSwapchainRenderPass->getRenderPass(), m_pCommandBuffers);

  //start time counting
  m_timer.start();
}

void Renderer::OnUpdate() {}

void Renderer::OnDraw() {
  auto frameStartTime = m_timer.getCurrentTime();
  m_ui->OnCommandRecord();

  // Acquire the next available image from the swapchain
  vk::Device device = m_Context->getDevice();
  vk::SwapchainKHR swapchain = m_swapchain.getSwapchain();

  // get semaphore data
  vk::Semaphore imageAvailableSemaphore =
      m_swapchain.getimageAvailableSemaphore();
  vk::Semaphore renderFinishedSemaphore =
      m_swapchain.getimageAvailableSemaphore();

  vk::ResultValue<uint32_t> result = device.acquireNextImageKHR(
      swapchain, UINT64_MAX, imageAvailableSemaphore);

  uint32_t imageIndex = result.value;

  auto commandBuffer = m_pCommandBuffers->getCommandBuffer(0);
  auto framebuffer = m_pFramebuffers->getFrameBuffer(imageIndex);
  auto pipeline = m_swapchainPipeline->getPipeline();

  // update uniform buffers
  UniformBufferObject obj{};

  //m_models[0].rotate(m_deltaTime * 20, glm::vec3(0.0f, 0.0f, 1.0f));
  obj.model = m_models[0].getModelMatrix();
  obj.view = m_cameras[0].getViewMatrix();
  m_cameras[0].setPerspectiveParameters(45.0f, 0.1f, 10.0f, (float)m_swapchain.getExtent().width /
                                  (float)m_swapchain.getExtent().height);

  //Get window params
  auto params = reinterpret_cast<App::WindowParams*>(glfwGetWindowUserPointer(m_pWindow));

  if(params->isMoving && params->leftButtonPressed) {
    m_cameras[0].RotationAroundUpdate(glm::vec3(params->delta, 0.0), 0.005f, glm::vec3(0.0f, 0.0f, 0.0f));
    params->isMoving = false;
    params->delta = glm::vec2(0.0f, 0.0f);
  }

  obj.proj = m_cameras[0].getProjectionMatrix();


  obj.proj[1][1] *= -1;
  m_pUniformBuffers->SetUniformBuffer(obj, imageIndex);
  m_pUniformBuffers->OnUpdate(imageIndex);


  // record command for swapchain
  {
    // begin recording of command buffer
    vk::CommandBufferBeginInfo beginInfo({}, nullptr);
    commandBuffer.begin(beginInfo);

    // begin render pass
    vk::RenderPass renderPass = m_pSwapchainRenderPass->getRenderPass();
    vk::Extent2D extent = m_swapchain.getExtent();

    vk::ClearValue clearValue;
    clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};
    vk::RenderPassBeginInfo renderpassBeginInfo{
        renderPass, framebuffer, vk::Rect2D({0, 0}, extent), 1, &clearValue};

    commandBuffer.beginRenderPass(renderpassBeginInfo,
                                  vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

    vk::Viewport viewport{0.0f,
                          0.0f,
                          static_cast<float>(extent.width),
                          static_cast<float>(extent.height),
                          0.0f,
                          1.0f};
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{{0, 0}, extent};

    commandBuffer.setScissor(0, 1, &scissor);
    vk::DescriptorSet descriptorSets[] = { m_swapchainResourceBinding.m_descriptorSets[imageIndex] };
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_swapchainResourceBinding.m_pipelineLayout, 0, descriptorSets, nullptr);


    for(auto &model : m_models) {
        model.draw(commandBuffer);
    }

    m_ui->OnDraw(commandBuffer);
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }
  // Submit commands to the graphics queue
  vk::SubmitInfo submitInfo;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
  auto pipelineStageFlags =
      vk::PipelineStageFlags{vk::PipelineStageFlagBits::eColorAttachmentOutput};
  submitInfo.pWaitDstStageMask = &pipelineStageFlags;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &imageAvailableSemaphore;

  // Get graphics queue
  auto graphicsQueue = m_Context->getGraphicsQueue();

  // wait for command buffer to complete execution
  auto fence = m_pCommandBuffers->getFence(0);
  graphicsQueue.submit(submitInfo, fence);

  assert(device.waitForFences(1, &fence, VK_TRUE, UINT64_MAX) ==
         vk::Result::eSuccess);
  assert(device.resetFences(1, &fence) == vk::Result::eSuccess);

  // Present the image to the swapchain
  vk::PresentInfoKHR presentInfo;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &imageIndex;

  auto presentQueue = m_Context->getPresentQueue();
  vk::Result presentResult = presentQueue.presentKHR(presentInfo);

  if (presentResult == vk::Result::eErrorOutOfDateKHR ||
      presentResult == vk::Result::eSuboptimalKHR) {

  } else if (presentResult != vk::Result::eSuccess) {
    // Handle other errors
    // ...
  }

  auto frameEndTime = m_timer.getCurrentTime();
  m_deltaTime = static_cast<float>((frameEndTime - frameStartTime).count() / 10e9);
  m_ui->setFPS(1.0f / m_deltaTime);
}

void Renderer::OnResize() {
  m_pFramebuffers->OnDestroy();
  int width = 0, height = 0;
  glfwGetFramebufferSize(m_pWindow, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(m_pWindow, &width, &height);
    glfwWaitEvents();
  }

  this->m_width = width;
  this->m_height = height;
  m_swapchain.OnRecreate(m_width, m_height);
  m_pFramebuffers->OnCreate();
}

void Renderer::OnDestroy() {
  // get device handle
  vk::Device device = m_Context->getDevice();

  m_ui->OnDestroy();
  delete m_ui;
  m_ui = nullptr;


  // destroy models
  for (auto &model : m_models) {
    model.destroy();
  }

  for (auto &texture : m_textures) {
    texture->OnDestroy();
  }

  // destroy command buffer
  m_pCommandBuffers->OnDestroy();
  delete m_pCommandBuffers;
  m_pCommandBuffers = nullptr;

  // destroy pipeline
  m_swapchainPipeline->OnDestroy();
  delete m_swapchainPipeline;
  m_swapchainPipeline = nullptr;

  // destroy resource binding data
  device.destroyPipelineLayout(m_swapchainResourceBinding.m_pipelineLayout);
  for (size_t i = 0;
       i < m_swapchainResourceBinding.m_descriptorSetLayouts.size(); ++i) {
    device.destroyDescriptorSetLayout(
        m_swapchainResourceBinding.m_descriptorSetLayouts[i]);
  }
  device.destroyDescriptorPool(m_swapchainResourceBinding.m_descriptorPool);

  // destroy uniform buffers
  m_pUniformBuffers->OnDestroy();
  delete m_pUniformBuffers;
  m_pUniformBuffers = nullptr;

  // destroy upload heaps
  m_pResourceUploadHeap->OnDestroy();
  delete m_pResourceUploadHeap;
  m_pResourceUploadHeap = nullptr;

  // destroy bufferPool
  m_pBufferPool->OnDestroy();
  delete m_pBufferPool;
  m_pBufferPool = nullptr;

  // destroy swapchain renderpass
  m_pFramebuffers->OnDestroy();
  delete m_pFramebuffers;
  m_pFramebuffers = nullptr;

  m_pSwapchainRenderPass->OnDestroy();
  delete m_pSwapchainRenderPass;
  m_pSwapchainRenderPass = nullptr;

  m_swapchain.OnDestroy();

  if (m_Context != nullptr) {
    m_Context->OnDestroy();
    delete m_Context;
    m_Context = nullptr;
  }
}


} // namespace hiddenpiggy
