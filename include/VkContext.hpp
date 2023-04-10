#ifndef VK_CONTEXT_HPP
#define VK_CONTEXT_HPP

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <optional>
#include <string>
#include <vector>

namespace hiddenpiggy {

class VkContext {
public:
  void OnCreate(const std::string AppName);
  void OnDestroy();
  std::vector<const char *> getEnabledInstanceExtensions();
  std::vector<const char *> getEnabledInstanceLayers();
  vk::PhysicalDevice getPhysicalDevice();
  vk::Device getDevice();

  // queue family index definition
  struct QueueFamilyIndex {
    std::optional<uint32_t> graphicsFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;
    std::optional<uint32_t> computeFamilyIndex;
  };

protected:
  vk::Instance m_Instance;
  vk::PhysicalDevice m_PhysicalDevice;
  vk::Device m_Device;
  VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
  struct QueueFamilyIndex m_queueFamilyIndices;

  std::string m_AppName;
  std::vector<const char *> m_EnabledInstanceExtensions;
  std::vector<const char *> m_EnabledInstanceLayers;
  std::vector<const char *> m_EnabledDeviceLayers;
  std::vector<const char *> m_EnabledDeviceExtensions;
};
} // namespace hiddenpiggy
#endif // !VK_CONTEXT_HPP
