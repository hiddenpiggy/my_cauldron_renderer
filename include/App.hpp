#ifndef APP_HPP
#define APP_HPP
#include "Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <string>
namespace hiddenpiggy {
class App {
public:
  App(const std::string AppName, int width, int height)
      : m_AppName(AppName), m_width(width), m_height(height) {}
  void OnCreate();
  void run();
  void OnDestroy();
  void OnResize();
  void OnUpdate();
  void SetExtent(int width, int height);
  vk::SurfaceKHR CreateWindowSurface(vk::Instance instance);

private:
  std::string m_AppName;
  GLFWwindow *m_pWindow = nullptr;
  Renderer *m_pRenderer = nullptr;
  uint32_t m_width, m_height;

  // call on resize in glfw framebuffersize callback
  static void framebufferSizeCallback(GLFWwindow *window, int width,
                                      int height) {

    auto app = reinterpret_cast<App *>(glfwGetWindowUserPointer(window));
    app->SetExtent(width, height);
    app->OnResize();
  }
};
} // namespace hiddenpiggy
#endif
