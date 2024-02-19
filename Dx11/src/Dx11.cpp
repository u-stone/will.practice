// Dx11.cpp : Defines the entry point for the application.
//

#include "../framework.h"
#include "Dx11.h"
#include "DxRenderWnd.h"
#include "DxCubeRenderWnd.h"
#include "DxCubeRenderWndWithLights.h"
#include "DxCubeRenderWndWithTexture.h"
#include <memory>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  auto render =
      std::make_shared<DxCubeRenderWndWithTexture>(hInstance, 1920, 1080);
  render->Init();

  // Main message loop:
  MSG msg = { 0 };
  while (WM_QUIT != msg.message)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      render->Render();
    }
  }

  return (int)msg.wParam;
}
