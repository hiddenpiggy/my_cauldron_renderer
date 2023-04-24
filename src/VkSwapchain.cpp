#include "VkSwapchain.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_handles.hpp"
namespace hiddenpiggy {
void VkSwapchain::OnCreate(VkContext *context, GLFWwindow *pWindow,
                           uint32_t width, uint32_t height) {
  // Create Window Surface so that we can create a swapchain
  assert(context != nullptr && pWindow != nullptr && width > 0 && height > 0);
  m_context = context;
  m_surface = CreateWindowSurface(context->getInstance(), pWindow);

  // Get the surface capabilities
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->getPhysicalDevice(),
                                            m_surface, &surfaceCapabilities);

  if (surfaceCapabilities.currentExtent.width == UINT32_MAX) {
    m_Extent.width = width;
    m_Extent.height = height;
  } else {
    m_Extent = surfaceCapabilities.currentExtent;
  }

  // Get queue family indices
  auto indices = context->getQueueFamilyIndices();
  std::vector<uint32_t> queueFamilyIndices{indices.graphicsFamilyIndex.value(),
                                           indices.presentFamilyIndex.value()};

  // create VkSwapchain
  vk::SwapchainCreateInfoKHR swapchainCreateInfo{
      {},
      m_surface,
      3,
      m_swapchainImageFormat,
      m_swapchainColorSpace,
      m_Extent,
      1,
      VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eColorAttachment,
      VULKAN_HPP_NAMESPACE::SharingMode::eExclusive,
      static_cast<uint32_t>(queueFamilyIndices.size()),
      queueFamilyIndices.data()};

  if (indices.graphicsFamilyIndex.value() !=
      indices.graphicsFamilyIndex.value()) {
    swapchainCreateInfo.imageSharingMode =
        VULKAN_HPP_NAMESPACE::SharingMode::eConcurrent;

    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  } else {
    swapchainCreateInfo.imageSharingMode =
        VULKAN_HPP_NAMESPACE::SharingMode::eExclusive;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  }

  swapchainCreateInfo.presentMode =
      VULKAN_HPP_NAMESPACE::PresentModeKHR::eMailbox;

  m_swapchain =
      context->getDevice().createSwapchainKHR(swapchainCreateInfo, nullptr);

  // get swapchain images
  auto swapchainImages =
      context->getDevice().getSwapchainImagesKHR(m_swapchain);

  m_swapchainImages = swapchainImages;

  // create swapchain image views
  m_swapchainImageViews.resize(swapchainImages.size());
  for (size_t i = 0; i < swapchainImages.size(); ++i) {
    vk::ImageViewCreateInfo imageViewCreateInfo{
        {},
        swapchainImages[i],
        vk::ImageViewType::e2D,
        m_swapchainImageFormat,
        {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
         vk::ComponentSwizzle::eIdentity},
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
        nullptr};

    m_swapchainImageViews[i] =
        context->getDevice().createImageView(imageViewCreateInfo, nullptr);
  }

  //create semaphores
  vk::SemaphoreCreateInfo semaphoreCreateInfo{};
  m_imageAvailableSemaphore = m_context->getDevice().createSemaphore(semaphoreCreateInfo);
  m_renderFinishedSemaphore = m_context->getDevice().createSemaphore(semaphoreCreateInfo);
}

void VkSwapchain::OnDestroy() {
  m_context->getDevice().waitIdle();
  m_context->getDevice().destroySemaphore(m_imageAvailableSemaphore);
  m_context->getDevice().destroySemaphore(m_renderFinishedSemaphore);
  
  for (auto imageView : m_swapchainImageViews) {
    m_context->getDevice().destroyImageView(imageView);
  }
  m_context->getDevice().destroySwapchainKHR(m_swapchain);
  m_context->getInstance().destroySurfaceKHR(m_surface);
}

vk::SurfaceKHR VkSwapchain::CreateWindowSurface(vk::Instance instance,
                                                GLFWwindow *pWindow) {
  VkSurfaceKHR surface;
  auto res = glfwCreateWindowSurface(instance, pWindow, nullptr, &surface);
  assert(res == VK_SUCCESS);
  return surface;
}

VULKAN_HPP_NAMESPACE::Format VkSwapchain::getFormat() {
  return m_swapchainImageFormat;
}

uint32_t VkSwapchain::getImageCount() { return m_swapchainImages.size(); }

vk::Extent2D VkSwapchain::getExtent() { return m_Extent; }

vk::ImageView VkSwapchain::getImageView(uint32_t index) {
  return m_swapchainImageViews[index];
}

void VkSwapchain::OnRecreate(int width, int height) {
  vk::Device device = m_context->getDevice();
  device.waitIdle();
  for(const auto &imageView : m_swapchainImageViews) {
    device.destroyImageView(imageView, nullptr);
  }
  device.destroySwapchainKHR(m_swapchain);

    // Get queue family indices
  auto indices = m_context->getQueueFamilyIndices();
  std::vector<uint32_t> queueFamilyIndices{indices.graphicsFamilyIndex.value(),
                                           indices.presentFamilyIndex.value()};

  m_Extent.setWidth(width);
  m_Extent.setHeight(height);
  // create VkSwapchain
  vk::SwapchainCreateInfoKHR swapchainCreateInfo{
      {},
      m_surface,
      3,
      m_swapchainImageFormat,
      m_swapchainColorSpace,
      m_Extent,
      1,
      VULKAN_HPP_NAMESPACE::ImageUsageFlagBits::eColorAttachment,
      VULKAN_HPP_NAMESPACE::SharingMode::eExclusive,
      static_cast<uint32_t>(queueFamilyIndices.size()),
      queueFamilyIndices.data()};

  if (indices.graphicsFamilyIndex.value() !=
      indices.graphicsFamilyIndex.value()) {
    swapchainCreateInfo.imageSharingMode =
        VULKAN_HPP_NAMESPACE::SharingMode::eConcurrent;

    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  } else {
    swapchainCreateInfo.imageSharingMode =
        VULKAN_HPP_NAMESPACE::SharingMode::eExclusive;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  }

  swapchainCreateInfo.presentMode =
      VULKAN_HPP_NAMESPACE::PresentModeKHR::eMailbox;

  m_swapchain =
      m_context->getDevice().createSwapchainKHR(swapchainCreateInfo, nullptr);

  // get swapchain images
  auto swapchainImages =
      m_context->getDevice().getSwapchainImagesKHR(m_swapchain);

  m_swapchainImages = swapchainImages;

  // create swapchain image views
  m_swapchainImageViews.resize(swapchainImages.size());
  for (size_t i = 0; i < swapchainImages.size(); ++i) {
    vk::ImageViewCreateInfo imageViewCreateInfo{
        {},
        swapchainImages[i],
        vk::ImageViewType::e2D,
        m_swapchainImageFormat,
        {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity,
         vk::ComponentSwizzle::eIdentity},
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1},
        nullptr};

    m_swapchainImageViews[i] =
        m_context->getDevice().createImageView(imageViewCreateInfo, nullptr);
  }
}

} // namespace hiddenpiggy
