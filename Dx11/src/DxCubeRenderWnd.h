#pragma once
#include "DxRenderWnd.h"
#include <DirectXMath.h> 

class DxCubeRenderWnd :
  public DxRenderWnd
{
public:
  DxCubeRenderWnd(HINSTANCE hInst, int width, int height);

  void Render() override;

private:
  void SetUpStencilView() override;
  void LoadVertexShader() override;
  void LoadPixelShader() override;
  void LoadData() override;
  void SetUpProjection() override;


protected:
  DirectX::XMMATRIX world_ {};
  DirectX::XMMATRIX view_ {};
  DirectX::XMMATRIX projection_ {};
  DirectX::XMMATRIX world2_{};

  DxPtr<ID3D11Texture2D> depth_texture_;
  DxPtr<ID3D11DepthStencilView> depth_stencil_view_;
};

