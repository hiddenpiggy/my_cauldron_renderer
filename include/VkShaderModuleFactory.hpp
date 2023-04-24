#ifndef SHADER_MODULE_HPP
#define SHADER_MODULE_HPP
#include "vulkan/vulkan.hpp"
#include <fstream>
namespace hiddenpiggy {
class VkShaderModuleFactory {
public:
  static std::vector<char> ReadFile(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
  };

  static vk::ShaderModule CreateShaderModule(vk::Device device,
                                             const char *fileName) {
    std::vector<char> buffer = ReadFile(fileName);
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer.data());
    vk::ShaderModule shaderModule = device.createShaderModule(createInfo);
    return shaderModule;
  }
};

} // namespace hiddenpiggy
#endif