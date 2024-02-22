#pragma once
#include <d3d11.h>
#include <string>

#include "../util/DxPtr.h"
#include "RenderWnd.h"

#pragma comment(lib, "D3D11.lib")

class DxRenderWnd : public RenderWnd {
 public:
  DxRenderWnd(HINSTANCE hInst, int width, int height);
  ~DxRenderWnd();

  void Init() override;

  virtual void Render();

 protected:
  virtual void InitDevice();
  virtual void InitRenderView();
  virtual void SetUpViewPort();
  virtual void SetUpStencilView();
  virtual void LoadVertexShader();
  virtual void SetUpVertexLayout(DxPtr<ID3DBlob> blob);
  virtual void LoadPixelShader();
  virtual void LoadData();
  virtual void SetUpProjection();

  DxPtr<ID3DBlob> CreateShader(std::wstring filename,
                               std::string entry_point,
                               std::string target);

 protected:
  DxPtr<IDXGISwapChain> swap_chain_;
  DxPtr<ID3D11Device> d3d11_device_;
  D3D_FEATURE_LEVEL feature_level_selected_ = D3D_FEATURE_LEVEL_11_1;
  DxPtr<ID3D11DeviceContext> d3d11_context_;
  DxPtr<ID3D11RenderTargetView> target_view_;
  DxPtr<ID3D11VertexShader> vertex_shader_;
  DxPtr<ID3D11InputLayout> layout_;
  DxPtr<ID3D11PixelShader> pixel_shader_;
  DxPtr<ID3D11Buffer> vertex_buffer_;
  DxPtr<ID3D11Buffer> index_buffer_;
  DxPtr<ID3D11Buffer> const_buffer_;

 protected:
  std::wstring vertex_shader_file_;
  std::string vertex_entry_;
  std::wstring pixel_shader_file_;
  std::string pixel_entry_;
};
