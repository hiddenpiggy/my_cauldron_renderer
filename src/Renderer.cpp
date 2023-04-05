#include "ColorConversion.h"
#include "ExtValidation.h"
#include "GBuffer.h"
#include "Swapchain.h"
#include <Renderer.hpp>
#include <iostream>
#include <mutex>
#include <vulkan/vulkan_core.h>

const uint32_t backBufferCount = 3;

void Renderer::OnCreate(CAULDRON_VK::Device *pDevice, CAULDRON_VK::SwapChain *pSwapchain, float fontSize)
{
    m_pDevice = pDevice;

    //init helpers
    const uint32_t cbvDescriptorCount = 2000;
    const uint32_t srvDescriptorCount = 8000;
    const uint32_t uavDescriptorCount = 10;
    const uint32_t samplerDescriptiorCount = 20;


   //m_ResourceViewHeaps.OnCreate(pDevice, cbvDescriptorCount, srvDescriptorCount,  uavDescriptorCount, samplerDescriptiorCount);


    //create a commandlist ring buffer for Direct queue info,ation
    uint32_t commandListsPerBackBuffer = 3;
   m_CommandListRing.OnCreate(pDevice, backBufferCount, commandListsPerBackBuffer);

   //create  a dynamic constant buffer 
   const uint32_t constantBufferMemSize = 200 * 1024 * 1024;
   m_ConstantBufferRing.OnCreate(pDevice, backBufferCount, constantBufferMemSize, "Uniforms"); 

   // create a static pool for vertices and indices
   const uint32_t staticGeometryMemSize = (1 * 128) * 1024 * 1024;
   m_VidMemBufferPool.OnCreate(pDevice, staticGeometryMemSize, true, "StaticGeom");

   //create a static pool for vertices and indices in system memory
   const uint32_t systemGeometryMemsize = 32 * 1024;
   m_SysMemBufferPool.OnCreate(pDevice, systemGeometryMemsize, false, "PostProcGeom");

   //init GPU time stamps module
  //m_GPUTimer.OnCreate(pDevice, backBufferCount);

   //helper to upload resources
   const uint32_t uploadHeapMemSize = 1000 * 1024 * 1024;
   m_UploadHeap.OnCreate(pDevice, uploadHeapMemSize);

   //create Gbuffer
//    {
//         m_GBuffer.OnCreate(
//             pDevice,
//             &m_ResourceViewHeaps,
//             {
//                 {CAULDRON_VK::GBUFFER_DEPTH, VK_FORMAT_D32_SFLOAT},
//                 {CAULDRON_VK::GBUFFER_FORWARD, VK_FORMAT_R16G16B16A16_SFLOAT},
//                 {CAULDRON_VK::GBUFFER_MOTION_VECTORS, VK_FORMAT_R16G16_SFLOAT},
//             },
//             1
//         );

//         CAULDRON_VK::GBufferFlags fullGBuffer = CAULDRON_VK::GBUFFER_DEPTH |  CAULDRON_VK::GBUFFER_FORWARD | CAULDRON_VK::GBUFFER_MOTION_VECTORS;
//         bool bClear = true;
//         m_RenderPassFullGBufferWithClear.OnCreate(&m_GBuffer ,fullGBuffer, bClear, "m_RenderPassFullGBufferWithClear");
//         m_RenderPassFullGBuffer.OnCreate(&m_GBuffer, fullGBuffer, !bClear, "m_RenderPassFullGBuffer");
//         m_RenderPassJustDepthAndHdr.OnCreate(&m_GBuffer, CAULDRON_VK::GBUFFER_DEPTH | CAULDRON_VK::GBUFFER_FORWARD, !bClear, "m_RenderPassJustDepthAndHdr");
//    }


   //init UI rendering resources
   m_ImGUI.OnCreate(pDevice, pSwapchain->GetRenderPass(), &m_UploadHeap, &m_ConstantBufferRing, fontSize);

   //Make sure upload heap has finished uploading before continuing
   m_VidMemBufferPool.UploadData(m_UploadHeap.GetCommandList());
   m_UploadHeap.FlushAndFinish();
    
}




void Renderer::OnDestroy()
{
    m_ImGUI.OnDestroy();

    //m_RenderPassFullGBufferWithClear.OnDestroy();
    //m_RenderPassJustDepthAndHdr.OnDestroy();
    //m_RenderPassFullGBuffer.OnDestroy();
    //m_GBuffer.OnDestroy();
    
    m_UploadHeap.OnDestroy();
    //m_GPUTimer.OnDestroy();
    m_VidMemBufferPool.OnDestroy();
    m_SysMemBufferPool.OnDestroy();
    m_ConstantBufferRing.OnDestroy();
    //m_ResourceViewHeaps.OnDestroy();
    m_CommandListRing.OnDestroy();
    //m_pDevice -> OnDestroy();
}

void Renderer::OnCreateWindowSizeDependentResources(CAULDRON_VK::SwapChain *pSwapchain, uint32_t Width, uint32_t Height)
{
    m_Width = Width;
    m_Height = Height;

    //View port setting
    m_Viewport.x = 0;
    m_Viewport.y = (float)Height;
    m_Viewport.width = (float)Width;
    m_Viewport.height = -(float)(Height);
    m_Viewport.minDepth = (float)0.0f;
    m_Viewport.maxDepth = (float)1.0f;

    //create scissor rectangle
    m_RectScissor.extent.width = Width;
    m_RectScissor.extent.height = Height;
    m_RectScissor.offset.x = 0;
    m_RectScissor.offset.y = 0;

    //create GBuffer
    //m_GBuffer.OnCreateWindowSizeDependentResources(pSwapchain, Width,  Height);

    //create framebuffers fir Gbuffer render passes
    //m_RenderPassFullGBufferWithClear.OnCreateWindowSizeDependentResources(Width, Height);
    //m_RenderPassJustDepthAndHdr.OnCreateWindowSizeDependentResources(Width, Height);
    //m_RenderPassFullGBuffer.OnCreateWindowSizeDependentResources(Width, Height);

    //TODO: update post processing passes

}

void Renderer::OnUpdateDisplayDependentResources(CAULDRON_VK::SwapChain *pSwapchain, bool bUseMagnifier)
{

    //m_ImGUI.UpdatePipeline((pSwapchain->GetDisplayMode() == DISPLAYMODE_SDR) ? pSwapchain -> GetRenderPass(): m_RenderPassJustDepthAndHdr.GetRenderPass());
    m_ImGUI.UpdatePipeline(pSwapchain->GetRenderPass());
}


void Renderer::OnDestroyWindowSizeDependentResources()
{
    //todo: destroy post process


    //m_RenderPassFullGBuffer.OnDestroyWindowSizeDependentResources();
    //m_RenderPassJustDepthAndHdr.OnDestroyWindowSizeDependentResources();
    //m_RenderPassFullGBuffer.OnDestroyWindowSizeDependentResources();
    //m_GBuffer.OnDestroyWindowSizeDependentResources();
}


void Renderer::OnRender(CAULDRON_VK::SwapChain *pSwapchain)
{
    m_CommandListRing.OnBeginFrame();
    VkCommandBuffer commandList = m_CommandListRing.GetNewCommandList();
    int imageIndex = pSwapchain -> WaitForSwapChain();
    //Here to Begin Command Buffer
    {
        VkCommandBufferBeginInfo cmd_buf_begin_info;
        cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buf_begin_info.pNext = nullptr;
        cmd_buf_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        cmd_buf_begin_info.pInheritanceInfo = nullptr;
        assert(vkBeginCommandBuffer(commandList, &cmd_buf_begin_info) == VK_SUCCESS);
    }

    VkRect2D renderArea {0, 0, m_Width - 20, m_Height - 20};
    //prepare render pass
    {
        VkRenderPassBeginInfo rp_begin = {};
        rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rp_begin.pNext = NULL;
        rp_begin.renderPass = pSwapchain->GetRenderPass();
        rp_begin.framebuffer = pSwapchain->GetFramebuffer(imageIndex);
        rp_begin.renderArea.offset.x = 0;
        rp_begin.renderArea.offset.y = 0;
        rp_begin.renderArea.extent.width = renderArea.extent.width;
        rp_begin.renderArea.extent.height = renderArea.extent.height;
        rp_begin.clearValueCount = 0;
        rp_begin.pClearValues = NULL;
        vkCmdBeginRenderPass(commandList, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
    }
    
    //m_RenderPassJustDepthAndHdr.BeginPass(commandList, renderArea);
    vkCmdSetScissor(commandList,  0, 1, &m_RectScissor);
    vkCmdSetViewport(commandList, 0, 1, &m_Viewport);
    m_ImGUI.Draw(commandList);
    //m_RenderPassJustDepthAndHdr.EndPass(commandList);

    vkCmdEndRenderPass(commandList);

    // submit command buffer
    {
        VkResult res = vkEndCommandBuffer(commandList);
        assert(res == VK_SUCCESS);

        VkSemaphore ImageAvailableSemaphore;
        VkSemaphore RenderFinishedSemaphores;
        VkFence CmdBufExecutedFences;
        pSwapchain->GetSemaphores(&ImageAvailableSemaphore, &RenderFinishedSemaphores, &CmdBufExecutedFences);
        VkPipelineStageFlags submitWaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = NULL;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &ImageAvailableSemaphore;
        submit_info.pWaitDstStageMask = &submitWaitStage;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &commandList;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &RenderFinishedSemaphores;
        res = vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submit_info, CmdBufExecutedFences);
        assert(res == VK_SUCCESS);
    }
}







