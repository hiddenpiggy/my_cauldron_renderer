#include "App.hpp"
#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"
#include <cassert>
#include <iostream>
namespace hiddenpiggy {
void App::OnCreate() {
  assert(glfwInit() != 0);
  assert(glfwVulkanSupported() != 0);
  m_pWindow =
      glfwCreateWindow(m_width, m_height, m_AppName.c_str(), nullptr, nullptr);

  glfwSetFramebufferSizeCallback(m_pWindow, App::framebufferSizeCallback);
  assert(m_pWindow != nullptr);

  // set window's user ptr to this App object so that I can call OnResize()
  glfwSetWindowUserPointer(m_pWindow, this);
  m_pRenderer = new Renderer(m_AppName, m_width, m_height);
  m_pRenderer->OnCreate();
}

void App::run() {
  while (!glfwWindowShouldClose(m_pWindow)) {
    glfwPollEvents();
    // do something
    this->OnUpdate();
  }
}

void App::OnDestroy() {
  // destroy all required
  m_pRenderer->OnDestroy();
  delete m_pRenderer;
  m_pRenderer = nullptr;
  glfwSetFramebufferSizeCallback(m_pWindow, nullptr);
  glfwDestroyWindow(m_pWindow);
  glfwTerminate();
}

void App::OnResize() {}

vk::SurfaceKHR App::CreateWindowSurface(vk::Instance instance) {
  VkSurfaceKHR surface{};
  VkResult res =
      glfwCreateWindowSurface(instance, m_pWindow, nullptr, &surface);
  assert(res == VK_SUCCESS);
  return surface;
}

void App::OnUpdate() { m_pRenderer->OnUpdate(); }

void App::SetExtent(int width, int height) {
  this->m_width = width;
  this->m_height = height;
}

}; // namespace hiddenpiggy
