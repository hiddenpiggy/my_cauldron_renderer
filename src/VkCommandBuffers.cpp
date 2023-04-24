#include "VkCommandBuffers.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace hiddenpiggy {
VkCommandBuffers::VkCommandBuffers(vk::Device device, vk::Queue queue, uint32_t queueFamilyIndex, vk::CommandPoolCreateFlags flags)
    : m_Device(device),m_Queue(queue),m_QueueFamilyIndex(queueFamilyIndex)
{
    vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer , m_QueueFamilyIndex);
    createInfo.flags = flags;
    m_CommandPool = m_Device.createCommandPool(createInfo);
}

VkCommandBuffers::~VkCommandBuffers()
{
}

void VkCommandBuffers::OnCreate(uint32_t numCommandBuffers)
{
    vk::CommandBufferAllocateInfo allocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, numCommandBuffers);
    m_CommandBuffers = m_Device.allocateCommandBuffers(allocateInfo);

    m_commandBufferFences.resize(m_CommandBuffers.size());
    for(size_t i = 0; i < m_CommandBuffers.size(); ++i) {
        vk::FenceCreateInfo fenceCreateInfo {
        };
        m_commandBufferFences[i] = m_Device.createFence(fenceCreateInfo);
    }
}

vk::CommandBuffer VkCommandBuffers::beginSingleTimeCommands() {
    std::vector<vk::CommandBuffer> commandBuffers;
    vk::CommandBufferAllocateInfo allocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 1, nullptr);
    commandBuffers = m_Device.allocateCommandBuffers(allocateInfo);

    vk::CommandBufferBeginInfo beginInfo{};

    commandBuffers[0].begin(beginInfo);
    return commandBuffers[0];
}

void VkCommandBuffers::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1; 
    submitInfo.pCommandBuffers = &commandBuffer;

    m_Queue.submit(submitInfo);
    m_Queue.waitIdle();

    m_Device.freeCommandBuffers(m_CommandPool, commandBuffer);
}

vk::CommandBuffer VkCommandBuffers::getCommandBuffer(uint32_t index)
{
    return m_CommandBuffers[index];
}

vk::CommandPool VkCommandBuffers::getCommandPool() const {
    return m_CommandPool;
}


vk::Fence VkCommandBuffers::getFence(uint32_t index) {
    return m_commandBufferFences[index];
}

void VkCommandBuffers::OnDestroy()
{
    if(m_commandBufferFences.size() > 0) {
        for(const auto &fence : m_commandBufferFences) {
            m_Device.destroyFence(fence);
        }
    }
    
    if (m_CommandBuffers.size() > 0)
    {
        m_Device.freeCommandBuffers(m_CommandPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
        m_CommandBuffers.clear();
    }
    m_Device.destroyCommandPool(m_CommandPool);
} 

}// namespace hiddenpiggy