#pragma once
#include <d3d11.h>
#include <string>

#include "RenderWnd.h"
#include "DxPtr.h"

#pragma comment(lib, "D3D11.lib")

class DxRenderWnd :
  public RenderWnd
{
public:
  DxRenderWnd(HINSTANCE hInst, int width, int height);

private:
  void InitDevice();
  void InitRenderView();
  void SetUpViewPort();
  void LoadHLSL(std::wstring filename, std::string entry_point, std::string target);

private:
  DxPtr<IDXGISwapChain> swap_chain_;
  DxPtr<ID3D11Device> d3d11_device_;
  D3D_FEATURE_LEVEL feature_level_selected_ = D3D_FEATURE_LEVEL_11_1;
  DxPtr<ID3D11DeviceContext> d3d11_context_;
  DxPtr<ID3D11RenderTargetView> target_view_;
};

