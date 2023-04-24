#ifndef APP_HPP
#define APP_HPP
#include "Renderer.hpp"
#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace hiddenpiggy {
class App {
public:
  void OnCreate(const std::string AppName, uint32_t width, uint32_t height);
  void run();
  void OnDestroy();
  void OnResize();
  void OnUpdate();
  void SetExtent(int width, int height);

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
