#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "UniformBuffers.hpp"
#include "VkContext.hpp"
#include "VkSwapchain.hpp"
#include "VkSwapchainFramebuffers.hpp"
#include "VkSwapchainGraphicsPipeline.hpp"
#include "VkSwapchainRenderPass.hpp"
#include "VkCommandBuffers.hpp"
#include "VkBufferPool.hpp"
#include "ResourceUploadHeap.hpp"
#include "Model.hpp"
#include "VkTexture.hpp"
#include "Scene.hpp"

namespace hiddenpiggy {
class Renderer {
public:
  Renderer() {}
  void OnCreate(std::string AppName, uint32_t width, uint32_t height,
                GLFWwindow *pWindow);
  void OnUpdate();
  void OnDraw();
  void OnDestroy();
  void OnResize();

  void loadScene(const std::string filename);


private:
  std::string m_AppName;
  GLFWwindow *m_pWindow;
  int m_width, m_height;

  //scene
  Scene scene;

  // Memory Management
  BufferPool *m_pBufferPool;
  ResourceUploadHeap *m_pResourceUploadHeap;

  // swapchain related handles
  VkSwapchain m_swapchain;
  SwapchainRenderPass *m_pSwapchainRenderPass = nullptr;
  VkSwapchainFramebuffers *m_pFramebuffers = nullptr;
  VkSwapchainGraphicsPipeline *m_swapchainPipeline = nullptr;

  // swapchain resource binding
  struct ResourceBinding {
    vk::DescriptorPool m_descriptorPool;
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
    vk::PipelineLayout m_pipelineLayout;
    std::vector<vk::DescriptorSet> m_descriptorSets;
  };

  ResourceBinding m_swapchainResourceBinding;

  //command buffers
  VkCommandBuffers *m_pCommandBuffers;
  VkContext *m_Context;

  //Uniform Buffers
  UniformBuffers *m_pUniformBuffers;

  //Model
  std::vector<Model> m_models;

  //Texture
  std::vector<VulkanTexture *> m_textures;
};
} // namespace hiddenpiggy
#endif
