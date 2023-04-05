#ifndef APP_RASTERIZER_H
#define APP_RASTERIZER_H
#include <FrameworkWindows.h>
#include "Renderer.hpp"
#include "UI.hpp"

class RasterizerApp : public CAULDRON_VK::FrameworkWindows
{
public:
    RasterizerApp(LPCSTR name);
    //~RasterizerApp() override;
    void OnParseCommandLine(LPSTR lpCmdLine, uint32_t* pWidth, uint32_t* pHeight) override;
    void OnCreate() override;
    void OnDestroy() override;
    void OnRender() override;
    bool OnEvent(MSG msg) override;
    void OnResize(bool resizeRender) override;
    void OnUpdateDisplay() override;

    void BuildUI();
    void OnUpdate();
    //void HandleInput(const ImGuiIO &io);
private:
    Renderer * m_pRenderer;
    float m_fontSize;
    UIState m_UIState;


};

#endif