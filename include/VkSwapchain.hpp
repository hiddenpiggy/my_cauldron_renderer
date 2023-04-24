#ifndef VK_SWAPCHAIN_HPP
#define VK_SWAPCHAIN_HPP
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "VkContext.hpp"
#include "vulkan/vulkan.hpp"
namespace hiddenpiggy {
class VkSwapchain {
public:
  void OnCreate(VkContext *context, GLFWwindow *pWindow, uint32_t width,
                uint32_t height);
  void OnRecreate(int width, int height);
  void OnDestroy();
  vk::SurfaceKHR CreateWindowSurface(vk::Instance instance,
                                     GLFWwindow *pWindow);

  VULKAN_HPP_NAMESPACE::Format getFormat();
  uint32_t getImageCount();
  vk::Extent2D getExtent();
  vk::ImageView getImageView(uint32_t index);
  vk::SwapchainKHR getSwapchain() const { return m_swapchain; }


  vk::Semaphore& getimageAvailableSemaphore() {
    return m_imageAvailableSemaphore;
  }

  vk::Semaphore& getrenderFinishedSemaphore() {
    return m_renderFinishedSemaphore;
  }

private:
  vk::SwapchainKHR m_swapchain;
  vk::SurfaceKHR m_surface;
  VULKAN_HPP_NAMESPACE::Format m_swapchainImageFormat =
      VULKAN_HPP_NAMESPACE::Format::eB8G8R8A8Srgb;
  VULKAN_HPP_NAMESPACE::ColorSpaceKHR m_swapchainColorSpace =
      VULKAN_HPP_NAMESPACE::ColorSpaceKHR::eSrgbNonlinear;

  vk::Semaphore m_imageAvailableSemaphore;
  vk::Semaphore m_renderFinishedSemaphore;

  vk::Extent2D m_Extent;
  std::vector<vk::Image> m_swapchainImages;
  std::vector<vk::ImageView> m_swapchainImageViews;
  VkContext *m_context;
};

} // namespace hiddenpiggy
#endif
