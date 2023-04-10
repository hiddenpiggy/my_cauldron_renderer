#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "VkContext.hpp"

namespace hiddenpiggy {
class Renderer {
public:
  Renderer(std::string AppName, int width, int height)
      : m_AppName(AppName), m_width(width), m_height(height) {}
  void OnCreate();
  void OnUpdate();
  void OnDestroy();

private:
  std::string m_AppName;
  int m_width, m_height;
  VkContext *m_Context;
};
} // namespace hiddenpiggy
#endif
