#ifndef UI_HPP
#define UI_HPP
#include <vulkan/vulkan.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include "VkContext.hpp"
#include "VkCommandBuffers.hpp"
#include "imgui.h"
#include "vulkan/vulkan_core.h"
#include <algorithm>



namespace hiddenpiggy {
class UI {
public:
  void OnCreate(VkContext *context, GLFWwindow *window, VkRenderPass renderPass, VkCommandBuffers* cmdPool) {
    
    m_device = context->getDevice();
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    assert(vkCreateDescriptorPool(context->getDevice(), &pool_info, nullptr, &m_imguiPool) == VK_SUCCESS);

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context->getInstance();
    init_info.PhysicalDevice = context->getPhysicalDevice();
    init_info.Device = context->getDevice();
    init_info.Queue = context->getGraphicsQueue();
    init_info.DescriptorPool = m_imguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, renderPass);

    // execute a gpu command to upload imgui font textures
    auto cmd = cmdPool->beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    cmdPool->endSingleTimeCommands(cmd);


    // clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

  }

  void OnCommandRecord() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    this->ShowWindow();
    ImGui::Render();
  }

  void ShowWindow() {
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

    if(!ImGui::Begin("Debug Window", nullptr, 0)) {
      ImGui::End();
      return ;
    }

    ImGui::Text("FPS: %f", m_uivars.fps);
    ImGui::End();
  }

  void OnDraw(VkCommandBuffer cmdBuf) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
  }

  void OnDestroy() {
      vkDestroyDescriptorPool(m_device, m_imguiPool, nullptr);
      ImGui_ImplVulkan_Shutdown();
  }

  void setFPS(float fps) {
    this->m_uivars.fps = fps;
  }

private:
    VkDevice m_device;
    VkDescriptorPool m_imguiPool;

    struct UIVariables {
      float fps = 0.0f;
    } m_uivars;

};
} // namespace hiddenpiggy
#endif