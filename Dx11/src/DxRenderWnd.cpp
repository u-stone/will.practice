#include "DxRenderWnd.h"

#include <d3dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

#include <DirectXMath.h>
using namespace DirectX;

/*
 * installed by cmd:
 * NuGet\Install-Package directxmath -Version 2024.1.1.1
 */
#include <DirectXMath.h>

DxRenderWnd::DxRenderWnd(HINSTANCE hInst, int width, int height)
  : RenderWnd(hInst, width, height)
    , vertex_shader_file_(L"shader/vertex_shader.fx")
    , vertex_entry_("VS")
    , pixel_shader_file_(L"shader/pixel_shader.fx")
    , pixel_entry_("PS")
{
}

DxRenderWnd::~DxRenderWnd()
{
  DxPtr<ID3D11Debug> debugInterface;
  HRESULT hr = d3d11_device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugInterface);
  if (SUCCEEDED(hr))
  {
    debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
  }
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
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // Options for handling pixels in a display surface after calling IDXGISwapChain1::Present1.
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_RESTRICTED_CONTENT; // over win 8, for better performance.


  HRESULT hr = ::D3D11CreateDeviceAndSwapChain(adapter, driver_type, module, flags, feature_levels, feature_level_num, sdk_version, &sd, &swap_chain_, &d3d11_device_, &feature_level_selected_, &d3d11_context_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }
}

void DxRenderWnd::InitRenderView()
{
  DxPtr<ID3D11Texture2D> pBackBuffer = nullptr;
  HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  hr = d3d11_device_->CreateRenderTargetView(pBackBuffer.get(), NULL, &target_view_);
  if (FAILED(hr)) {
    DxError(hr);
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

void DxRenderWnd::SetUpStencilView()
{
}

void DxRenderWnd::LoadVertexShader()
{
    DxPtr<ID3DBlob> blob = CreateShader(vertex_shader_file_, vertex_entry_, "vs_5_0");
  if (!blob) {
    return;
  }

  HRESULT hr = d3d11_device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &vertex_shader_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  SetUpVertexLayout(blob);
}

void DxRenderWnd::SetUpVertexLayout(DxPtr<ID3DBlob> blob)
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT num = ARRAYSIZE(layout);
    HRESULT hr = d3d11_device_->CreateInputLayout(layout, num, blob->GetBufferPointer(), blob->GetBufferSize(), &layout_);
    if (FAILED(hr)) {
        DxError(hr);
        return;
    }

    d3d11_context_->IASetInputLayout(layout_.get());
}

void DxRenderWnd::LoadPixelShader()
{
  DxPtr<ID3DBlob> blob = CreateShader(pixel_shader_file_, pixel_entry_, "ps_5_0");
  if (!blob) {
    return;
  }

  HRESULT hr = d3d11_device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &pixel_shader_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }
}

void DxRenderWnd::Init()
{
  RenderWnd::Init();

  InitDevice();
  InitRenderView();
  SetUpStencilView();
  SetUpViewPort();

  LoadVertexShader();
  LoadPixelShader();
  LoadData();

  SetUpProjection();
}

void DxRenderWnd::LoadData()
{
  struct SimpleVertex
  {
    XMFLOAT3 Pos;
  };

  // Create vertex buffer
  SimpleVertex vertices[] =
  {
      XMFLOAT3(0.0f, 0.5f, 0.5f),
      XMFLOAT3(0.5f, -0.5f, 0.5f),
      XMFLOAT3(-0.5f, -0.5f, 0.5f),
  };

  D3D11_BUFFER_DESC bd = { 0 };
  bd.ByteWidth = sizeof(SimpleVertex) * 3;
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData = { 0 };
  InitData.pSysMem = vertices;

  HRESULT hr = d3d11_device_->CreateBuffer(&bd, &InitData, &vertex_buffer_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  UINT stride = sizeof(SimpleVertex);
  UINT offset = 0;
  d3d11_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);
  d3d11_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxRenderWnd::SetUpProjection()
{
}

DxPtr<ID3DBlob> DxRenderWnd::CreateShader(std::wstring filename, std::string entry_point, std::string target)
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
    DxError(hr);
    return nullptr;
  }

  return blob;
}

void DxRenderWnd::Render()
{
  // Clear the back buffer 
  float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
  d3d11_context_->ClearRenderTargetView(target_view_.get(), ClearColor);

  // Render a triangle
  d3d11_context_->VSSetShader(vertex_shader_.get(), NULL, 0);
  d3d11_context_->PSSetShader(pixel_shader_.get(), NULL, 0);
  d3d11_context_->Draw(3, 0);

  // Present the information rendered to the back buffer to the front buffer (the screen)
  // for flip mode, we must set a syncinterval
  // for legcy mode, we set syncinterval as 0
  swap_chain_->Present(1, 0);

  // Using flip mode, we MUST rebind render target.
  d3d11_context_->OMSetRenderTargets(1, &target_view_, NULL);
}
