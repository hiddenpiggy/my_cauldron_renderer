#ifndef RENDERER_HPP
#define RENDERER_HPP
#include "CommandListRing.h"
#include "DynamicBufferRing.h"
#include "GBuffer.h"
#include "GPUTimestamps.h"
#include "GltfBBoxPass.h"
#include "GltfPbrPass.h"
#include "ResourceViewHeaps.h"
#include "StaticBufferPool.h"
#include "Swapchain.h"
#include "UploadHeap.h"
#include <Device.h>
#include <Swapchain.h>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <GBuffer.h>
#include "imgui.h"

class Renderer
{
public:
    void OnCreate(CAULDRON_VK::Device *pDevice, CAULDRON_VK::SwapChain *pSwapchain, float fontSize);
    void OnDestroy();
    void OnRender(CAULDRON_VK::SwapChain *pSwapchain);
    void OnCreateWindowSizeDependentResources(CAULDRON_VK::SwapChain *pSwapchain, uint32_t Width, uint32_t Height);
    void OnDestroyWindowSizeDependentResources();

    void OnUpdateDisplayDependentResources(CAULDRON_VK::SwapChain *pSwapchain, bool bUseMagnifier);

private:
    CAULDRON_VK::Device* m_pDevice;

    uint32_t m_Width, m_Height;
    VkRect2D  m_RectScissor;
    VkViewport  m_Viewport;

    // helper classes for initialization
    //CAULDRON_VK::ResourceViewHeaps m_ResourceViewHeaps;
    CAULDRON_VK::UploadHeap        m_UploadHeap;
    CAULDRON_VK::DynamicBufferRing m_ConstantBufferRing;
    CAULDRON_VK::StaticBufferPool  m_VidMemBufferPool;
    CAULDRON_VK::StaticBufferPool  m_SysMemBufferPool;
    CAULDRON_VK::CommandListRing   m_CommandListRing;
    //CAULDRON_VK::GPUTimestamps     m_GPUTimer;
    
    //Gbuffer and render passes
    //CAULDRON_VK::GBuffer                      m_GBuffer;
    //CAULDRON_VK::GBufferRenderPass            m_RenderPassFullGBufferWithClear; 
    //CAULDRON_VK::GBufferRenderPass            m_RenderPassJustDepthAndHdr;
    //CAULDRON_VK::GBufferRenderPass            m_RenderPassFullGBuffer;

    //UI resources 
    CAULDRON_VK::ImGUI                                    m_ImGUI;



};
#endif