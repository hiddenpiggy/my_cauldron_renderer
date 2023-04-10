#include "VkContext.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <set>

namespace hiddenpiggy {

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallBack(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {
  std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pCallback) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pCallback);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT callback,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, callback, pAllocator);
  }
}

// remove duplicates in vector
void removeDuplicates(std::vector<std::string> &vec) {
  std::sort(vec.begin(), vec.end());
  auto last = std::unique(vec.begin(), vec.end());
  vec.erase(last, vec.end());
}

bool checkLayersAvaliable(std::vector<std::string> desiredLayers) {

  // Check for the availability of the requested layers
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  bool allLayersAvailable = true;
  for (const auto &desiredLayer : desiredLayers) {
    bool layerAvailable = false;
    for (const auto &availableLayer : availableLayers) {
      if (desiredLayer == availableLayer.layerName) {
        layerAvailable = true;
        break;
      }
    }
    if (!layerAvailable) {
      std::cout << "Error: Layer not available: " << desiredLayer << std::endl;
      allLayersAvailable = false;
    }
  }
  return allLayersAvailable;
}

bool checkExtensionsAvaliable(std::vector<std::string> desiredExtensions) {

  // Check for the availability of the requested extensions
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         availableExtensions.data());

  bool allExtensionsAvailable = true;
  for (const auto &desiredExtension : desiredExtensions) {
    bool extensionAvailable = false;
    for (const auto &availableExtension : availableExtensions) {
      if (desiredExtension == availableExtension.extensionName) {
        extensionAvailable = true;
        break;
      }
    }
    if (!extensionAvailable) {
      std::cout << "Error: Extension not available: " << desiredExtension
                << std::endl;
      allExtensionsAvailable = false;
    }
  }
  return allExtensionsAvailable;
}

bool CheckPhysicalDeviceLayerSupport(VkPhysicalDevice physicalDevice,
                                     std::vector<std::string> layerNames) {

  uint32_t layerCount = 0;
  vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
  std::vector<VkLayerProperties> layers(layerCount);
  vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, layers.data());

  for (const auto &layerName : layerNames) {
    bool layerSupported = false;
    for (const auto &layer : layers) {
      if (layer.layerName == layerName) {
        layerSupported = true;
        break;
      }
    }
    if (!layerSupported) {
      return false;
    }
  }

  return true;
}

bool CheckPhysicalDeviceExtensionSupport(
    VkPhysicalDevice physicalDevice, std::vector<std::string> &extensionNames) {

  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       extensions.data());

  // change the extensions to set
  std::set<std::string> extensions_set;
  for (const auto &extension : extensions) {
    extensions_set.insert(extension.extensionName);
  }

  // compare the extension name with extension set
  for (const auto &extensionName : extensionNames) {
    if (!extensions_set.contains(extensionName)) {
      std::cerr << "extension name:" << extensionName << " not supported! "
                << std::endl;
      return false;
    }
  }
  return true;
}

vk::PhysicalDevice pickPhysicalDevice(vk::Instance instance) {
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    // Handle error: No available physical devices
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto &device : devices) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      // Select the first available discrete GPU
      physicalDevice = device;
      break;
    }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
    // Handle error: No available discrete GPUs
    std::cerr << "no available discrete gpu!" << std::endl;
    exit(-1);
  }

  return physicalDevice;
}

void FindQueueFamilyIndex(vk::PhysicalDevice physicalDevice,
                          VkContext::QueueFamilyIndex &indices) {
  // Get queue family properties
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilyProperties.data());

  // find one queue supports GRAPHICS
  int i = 0;
  for (const auto &queueFamily : queueFamilyProperties) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamilyIndex = i;
    }

    if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
      indices.presentFamilyIndex = i;
    }

    if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
      indices.computeFamilyIndex = i;
    }
    i++;
  }

  // Here graphics, presentFamilyIndex and computerFamilyIndex all should has
  // value
  assert(indices.graphicsFamilyIndex.has_value() &&
         indices.presentFamilyIndex.has_value() &&
         indices.computeFamilyIndex.has_value());
}

void VkContext::OnCreate(const std::string AppName) {
  m_AppName = AppName;
  const std::string engineName = "No Engine";

  // prepare for required layers
  std::vector<std::string> requiredLayers{"VK_LAYER_KHRONOS_validation"};
  assert(checkLayersAvaliable(requiredLayers) == true);

  for (const auto &x : requiredLayers) {
    m_EnabledInstanceLayers.push_back(x.data());
  }

  // prepare for required extensions
  std::vector<std::string> requiredInstanceExtensions{
      // VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
      // VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
      // VK_KHR_RAY_QUERY_EXTENSION_NAME,
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
      VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

  // get required extension name from glfw3
  uint32_t extensionCount;
  const char **extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
  for (size_t i = 0; i < extensionCount; ++i) {
    requiredInstanceExtensions.push_back(extensions[i]);
  }

  // remove duplicates in vector
  removeDuplicates(requiredInstanceExtensions);
  assert(checkExtensionsAvaliable(requiredInstanceExtensions) == true);

  // add required extensions to m_EnabledExtensions
  for (const auto &x : requiredInstanceExtensions) {
    m_EnabledInstanceExtensions.push_back(x.data());
  }

  vk::ApplicationInfo appInfo(AppName.c_str(), VK_MAKE_VERSION(1, 0, 0),
                              engineName.c_str(), VK_MAKE_VERSION(1, 0, 0),
                              VK_MAKE_VERSION(1, 2, 0), nullptr);

  vk::InstanceCreateInfo createInfo{
      vk::InstanceCreateFlags{0x0},
      &appInfo,
      static_cast<uint32_t>(m_EnabledInstanceLayers.size()),
      m_EnabledInstanceLayers.data(),
      static_cast<uint32_t>(m_EnabledInstanceExtensions.size()),
      m_EnabledInstanceExtensions.data()};

  vk::Result res = vk::createInstance(&createInfo, nullptr, &m_Instance);
  assert(res == vk::Result::eSuccess);

  // prepare for debuglayers
  auto debug_createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
      vk::DebugUtilsMessengerCreateFlagsEXT(),
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
      hiddenpiggy::debugCallBack, nullptr);

  // m_Instance.createDebugUtilsMessengerEXT(debug_createInfo);
  // this will cause link issues
  assert(CreateDebugUtilsMessengerEXT(
             m_Instance,
             reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(
                 &debug_createInfo),
             nullptr, &m_debugUtilsMessenger) == VK_SUCCESS);

  // pick physical device
  m_PhysicalDevice = pickPhysicalDevice(m_Instance);

  // add raytracing features to requiredExtensions
  std::vector<std::string> requiredDeviceExtensions{
      VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
      VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
      VK_KHR_RAY_QUERY_EXTENSION_NAME,
      VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME};

  std::vector<std::string> requiredDeviceLayers{"VK_LAYER_KHRONOS_validation"};

  assert(CheckPhysicalDeviceExtensionSupport(m_PhysicalDevice,
                                             requiredDeviceExtensions) == true);
  assert(CheckPhysicalDeviceLayerSupport(m_PhysicalDevice,
                                         requiredDeviceLayers) == true);

  // add required device extensions and layers to enabledextensions and layers
  for (const auto &layer : requiredDeviceLayers) {
    m_EnabledDeviceLayers.push_back(layer.c_str());
  }

  for (const auto &extension : requiredDeviceExtensions) {
    m_EnabledDeviceExtensions.push_back(extension.c_str());
  }

  // prepare for queue family
  FindQueueFamilyIndex(m_PhysicalDevice, m_queueFamilyIndices);

  // get queue family indexes
  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

  // handle graphics queue graphics queue family
  float queuePriority0 = 1.0f, queuePriority1 = 0.5f, queuePriority2 = 0.3f;
  vk::DeviceQueueCreateInfo queueCreateInfo{};
  queueCreateInfo.queueFamilyIndex =
      m_queueFamilyIndices.graphicsFamilyIndex.value();
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority0;
  queueCreateInfos.push_back(queueCreateInfo);

  // handle present queue
  if (queueCreateInfo.queueFamilyIndex !=
      m_queueFamilyIndices.presentFamilyIndex) {
    queueCreateInfo.queueFamilyIndex =
        m_queueFamilyIndices.presentFamilyIndex.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority1;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // handle compute queue
  if (m_queueFamilyIndices.computeFamilyIndex.value() !=
          m_queueFamilyIndices.presentFamilyIndex.value() &&
      m_queueFamilyIndices.computeFamilyIndex.value() !=
          m_queueFamilyIndices.graphicsFamilyIndex.value()) {
    queueCreateInfo.queueFamilyIndex =
        m_queueFamilyIndices.computeFamilyIndex.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority2;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  // enable device features for raytracing
  vk::PhysicalDeviceFeatures2 deviceFeatures2{};

  vk::PhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.geometryShader = VK_TRUE;
  deviceFeatures2.features = deviceFeatures;

  vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeature = {};
  accelFeature.accelerationStructure = VK_TRUE;
  deviceFeatures2.pNext = &accelFeature;

  vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtpipelineFeature = {};
  rtpipelineFeature.rayTracingPipeline = VK_TRUE;
  rtpipelineFeature.pNext = nullptr;
  accelFeature.pNext = &rtpipelineFeature;

  vk::DeviceCreateInfo deviceCreateInfo{
      {},
      static_cast<uint32_t>(queueCreateInfos.size()),
      queueCreateInfos.data(),
      static_cast<uint32_t>(m_EnabledDeviceLayers.size()),
      m_EnabledDeviceLayers.data(),
      static_cast<uint32_t>(m_EnabledDeviceExtensions.size()),
      m_EnabledDeviceExtensions.data(),
      nullptr};

  deviceCreateInfo.pNext = &deviceFeatures2;
  m_Device = m_PhysicalDevice.createDevice(deviceCreateInfo);
}

void VkContext::OnDestroy() {
  m_Device.destroy();
  DestroyDebugUtilsMessengerEXT(m_Instance, m_debugUtilsMessenger, nullptr);
  m_Instance.destroy();
}

vk::PhysicalDevice VkContext::getPhysicalDevice() { return m_PhysicalDevice; }

vk::Device VkContext::getDevice() { return m_Device; }

std::vector<const char *> VkContext::getEnabledInstanceExtensions() {
  return m_EnabledInstanceExtensions;
}

std::vector<const char *> VkContext::getEnabledInstanceLayers() {
  return m_EnabledInstanceLayers;
}

} // namespace hiddenpiggy
  //
  //
