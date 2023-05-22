#include "App.hpp"
#include "GLFW/glfw3.h"
#include "Renderer.hpp"
#include "vulkan/vulkan_core.h"
#include <cassert>
#include <iostream>
#include <imgui_impl_glfw.h>
namespace hiddenpiggy {

void App::OnCreate(const std::string AppName, uint32_t width, uint32_t height) {
  m_AppName = AppName;
  m_width = width;
  m_height = height;
  
  assert(glfwInit() != 0);
  assert(glfwVulkanSupported() != 0);
  //it is necessary for vulkan use
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  
  m_pWindow =
      glfwCreateWindow(m_width, m_height, m_AppName.c_str(), nullptr, nullptr);
  

  glfwSetFramebufferSizeCallback(m_pWindow, App::framebufferSizeCallback);
  glfwSetMouseButtonCallback(m_pWindow, App::mouseButtonCallBack);
  glfwSetCursorPosCallback(m_pWindow, App::cursorPositionCallBack);
  assert(m_pWindow != nullptr);

  // set window's user ptr to this App object so that I can call OnResize()
  glfwSetWindowUserPointer(m_pWindow, &(this->m_windowParams));
  m_pRenderer = new Renderer();
  m_pRenderer->OnCreate(m_AppName, m_width, m_height, m_pWindow);
  m_windowParams.pApp = this;
}

void App::run() {
  while (!glfwWindowShouldClose(m_pWindow)) {
    glfwPollEvents();
    
    // do something
    this->OnUpdate();
    m_pRenderer->OnDraw();
  }
}

void App::OnDestroy() {
  // destroy all required
  m_pRenderer->OnDestroy();
  delete m_pRenderer;
  m_pRenderer = nullptr;
  glfwSetFramebufferSizeCallback(m_pWindow, nullptr);
  glfwSetMouseButtonCallback(m_pWindow, nullptr);
  glfwSetCursorPosCallback(m_pWindow, nullptr);
  glfwDestroyWindow(m_pWindow);
  glfwTerminate();
}

void App::OnResize() {
  m_pRenderer->OnResize();
}

void App::OnUpdate() { m_pRenderer->OnUpdate(); }

void App::SetExtent(int width, int height) {
  this->m_width = width;
  this->m_height = height;
}

}; // namespace hiddenpiggy
