#include "Renderer.hpp"

namespace hiddenpiggy {
void Renderer::OnCreate() {
  m_Context = new VkContext();
  m_Context->OnCreate(m_AppName);
}

void Renderer::OnUpdate() {}

void Renderer::OnDestroy() {
  if (m_Context != nullptr) {
    m_Context->OnDestroy();
    delete m_Context;
    m_Context = nullptr;
  }
}
} // namespace hiddenpiggy
