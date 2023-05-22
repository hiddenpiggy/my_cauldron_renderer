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

  struct WindowParams {
    App *pApp;
    bool leftButtonPressed = false;
    bool isMoving = true;
    glm::vec2 prevCursorPos = glm::vec2(0.0f, 0.0f);
    glm::vec2 delta;
  } m_windowParams;

private:
  std::string m_AppName;
  GLFWwindow *m_pWindow = nullptr;
  Renderer *m_pRenderer = nullptr;
  uint32_t m_width, m_height;

  // call on resize in glfw framebuffersize callback
  static void framebufferSizeCallback(GLFWwindow *window, int width,
                                      int height) {

    auto params =
        reinterpret_cast<WindowParams *>(glfwGetWindowUserPointer(window));
    params->pApp->SetExtent(width, height);
    params->pApp->OnResize();
  }

  static void mouseButtonCallBack(GLFWwindow *window, int button, int action,
                                  int mods) {
    auto params =
        reinterpret_cast<WindowParams *>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (action == GLFW_PRESS) {
        params->leftButtonPressed = true;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        params->prevCursorPos = glm::vec2(xpos, ypos);
      } else if (action == GLFW_RELEASE) {
        params->leftButtonPressed = false;
        params->delta = glm::vec2(0, 0);
      }
    }
  }

  static void cursorPositionCallBack(GLFWwindow *window, double xpos,
                                     double ypos) {
    auto params =
        reinterpret_cast<WindowParams *>(glfwGetWindowUserPointer(window));
    if (params->leftButtonPressed) {
      params->isMoving = true;
      glm::vec2 nowPos(xpos, ypos);
      params->delta = nowPos - params->prevCursorPos;
      params->prevCursorPos = nowPos;
    }
  }
};

} // namespace hiddenpiggy
#endif
