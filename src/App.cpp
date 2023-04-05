#include "DXCHelper.h"
#include "ImguiHelper.h"
#include "ShaderCompilerHelper.h"
#include <App_rasterizer.hpp>
#include <Imgui.h>

RasterizerApp::RasterizerApp(LPCSTR name) : CAULDRON_VK::FrameworkWindows(name)
{
}



void RasterizerApp::OnParseCommandLine(LPSTR lpCmdLine, uint32_t* pWidth, uint32_t* pHeight)
{
    *pWidth = 1280;
    *pHeight = 1080;
    this->m_Width = 1290;
    this->m_Height = 1080;
}

void RasterizerApp::OnCreate()
{
    //Create Instance of renderer and init it
    InitDirectXCompiler();
    CAULDRON_VK::CreateShaderCache();

    m_pRenderer = new Renderer();
    m_pRenderer -> OnCreate(&m_device, &m_swapChain, m_fontSize);
    m_pRenderer -> OnCreateWindowSizeDependentResources(&m_swapChain, m_Width, m_Height);

    //init non-GFX part of imgui
    ImGUI_Init((void *)m_windowHwnd);
    m_UIState.initialize();

    OnResize(true);
    OnUpdateDisplay();

}

void RasterizerApp::OnDestroy()
{
    ImGUI_Shutdown();
    m_device.GPUFlush();

    m_pRenderer->OnDestroy();
    delete m_pRenderer;
    m_pRenderer = nullptr;

     // shut down the shader compiler 
    DestroyShaderCache(&m_device);
}

void RasterizerApp::OnRender()
{
    BeginFrame();

    ImGUI_UpdateIO(m_Width, m_Height);
    ImGui::NewFrame();

    BuildUI();
    m_pRenderer -> OnRender(&m_swapChain);
    EndFrame();
}

bool RasterizerApp::OnEvent(MSG msg)
{
    if(ImGUI_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam))
        return true;
    
    //TODO: handle function keys
    return true;

}


void RasterizerApp::OnResize(bool resizeRender)
{

}

void RasterizerApp::OnUpdateDisplay()
{
    if(m_pRenderer != nullptr)
    {
        m_pRenderer->OnUpdateDisplayDependentResources(&m_swapChain, false);
    }
}

void RasterizerApp::BuildUI()
{
    ImGui::Text("Hello, world %d", 123);
    if (ImGui::Button("Save"))
    {
    }

}

void RasterizerApp::OnUpdate()
{
    
}

//--------------------------------------------------------------------------------------
//
// WinMain
//
//--------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    LPCSTR Name = "HelloWorld";

    // create new Vulkan sample
    return RunFramework(hInstance, lpCmdLine, nCmdShow, new RasterizerApp(Name));
}
