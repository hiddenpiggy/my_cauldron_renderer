#ifndef VK_SWAPCHAIN_FRAMEBUFFERS_HPP
#define VK_SWAPCHAIN_FRAMEBUFFERS_HPP
#include "VkSwapchainRenderPass.hpp"
#include "vulkan/vulkan.hpp"
#include "VkSwapchain.hpp"
#include "vulkan/vulkan_handles.hpp"
#include <vector>
namespace hiddenpiggy {
    class VkSwapchainFramebuffers {
        public:
            VkSwapchainFramebuffers(vk::Device device, VkSwapchain *swapchain, SwapchainRenderPass *swapchainRenderPass) : m_device(device), m_swapchain(swapchain), m_swapchainRenderPass(swapchainRenderPass) {}
            void OnCreate();
            void OnDestroy();
            vk::Framebuffer getFrameBuffer(uint32_t index) const { return m_framebuffers[index]; }
            size_t getSize() const { return m_framebuffers.size(); }
        private:
            std::vector<vk::Framebuffer> m_framebuffers;
            vk::Device m_device;
            VkSwapchain *m_swapchain = nullptr;
            SwapchainRenderPass *m_swapchainRenderPass = nullptr;
    };
}
#endif