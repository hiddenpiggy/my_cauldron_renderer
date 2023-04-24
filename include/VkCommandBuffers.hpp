#ifndef  VK_COMMAND_BUFFERS_HPP
#define VK_COMMAND_BUFFERS_HPP
#include "vulkan/vulkan.hpp"
namespace hiddenpiggy {
class VkCommandBuffers
{
public:
    VkCommandBuffers(vk::Device device, vk::Queue queue, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    ~VkCommandBuffers();

    void init(uint32_t numCommandBuffers);
    vk::CommandBuffer getCommandBuffer(uint32_t index);
    vk::Fence getFence(uint32_t index);
    void OnCreate(uint32_t numCommandBuffers);

    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);
    void OnDestroy();
    vk::CommandPool getCommandPool() const;

private:
    vk::Device m_Device;
    vk::Queue m_Queue;
    vk::CommandPool m_CommandPool;
    uint32_t m_QueueFamilyIndex;
    std::vector<vk::CommandBuffer> m_CommandBuffers;
    std::vector<vk::Fence>  m_commandBufferFences;
};
}

#endif