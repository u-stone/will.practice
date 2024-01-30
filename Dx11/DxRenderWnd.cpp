#include "DxRenderWnd.h"
#include "DxUtil.h"
#include <d3dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

DxRenderWnd::DxRenderWnd(HINSTANCE hInst, int width, int height)
  : RenderWnd(hInst, width, height)
{
  InitDevice();
  InitRenderView();
  SetUpViewPort();
  LoadHLSL(L"shader/simple.fx", "VS", "");
}

void DxRenderWnd::InitDevice()
{
  IDXGIAdapter* adapter = NULL;

  D3D_DRIVER_TYPE driver_types[] = {
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE,
  };
  D3D_DRIVER_TYPE driver_type = driver_types[0];

  HMODULE module = NULL;

  UINT flags = 0;
#ifdef _DEBUG
  flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_FEATURE_LEVEL feature_levels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0
  };
  UINT feature_level_num = sizeof(feature_levels) / sizeof(feature_levels[0]);

  UINT sdk_version = D3D11_SDK_VERSION;

  DXGI_SWAP_CHAIN_DESC sd = { 0 };
  sd.SampleDesc.Count = 1; // The number of multisamples per pixel.
  sd.SampleDesc.Quality = 0; // The image quality level
  sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  sd.BufferDesc.Width = width_;
  sd.BufferDesc.Height = height_;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = 2;
  sd.OutputWindow = wnd_;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Options for handling pixels in a display surface after calling IDXGISwapChain1::Present1.
  sd.Flags = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // over win 8, for better performance.


  HRESULT hr = ::D3D11CreateDeviceAndSwapChain(adapter, driver_type, module, flags, feature_levels, feature_level_num, sdk_version, &sd, &swap_chain_, &d3d11_device_, &feature_level_selected_, &d3d11_context_);
  if (FAILED(hr)) {
    auto err = DxError(hr);
    return;
  }
}

void DxRenderWnd::InitRenderView()
{
  DxPtr<ID3D11Texture2D> pBackBuffer = nullptr;
  HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  if (FAILED(hr)) {
    auto err = DxError(hr);
    return;
  }

  hr = d3d11_device_->CreateRenderTargetView(pBackBuffer.get(), NULL, &target_view_);
  if (FAILED(hr)) {
    auto err = DxError(hr);
    return;
  }

  d3d11_context_->OMSetRenderTargets(1, &target_view_, NULL);
}

void DxRenderWnd::SetUpViewPort()
{
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)width_;
  vp.Height = (FLOAT)height_;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;

  d3d11_context_->RSSetViewports(1, &vp);
}

void DxRenderWnd::LoadHLSL(std::wstring filename, std::string entry_point, std::string target)
{
  D3D10_SHADER_MACRO* shader_macro = NULL;
  LPD3D10INCLUDE include = NULL;
  UINT shader_flag1 = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  shader_flag1 |= D3DCOMPILE_DEBUG;
#endif
  UINT shader_flag2 = 0;
  DxPtr<ID3DBlob> blob;
  DxPtr<ID3DBlob> blob_result;


  HRESULT hr = D3DCompileFromFile(filename.c_str(), shader_macro, include, entry_point.c_str(), target.c_str(), shader_flag1, shader_flag2, &blob, &blob_result);
  if (FAILED(hr)) {
    auto err = DxError(hr);
    return;
  }
}
