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
  void LoadVertexShader() override;
  void LoadPixelShader() override;
  void LoadData() override;
  void SetUpProjection() override;

private:
  DirectX::XMMATRIX world_;
  DirectX::XMMATRIX view_;
  DirectX::XMMATRIX projection_;
};

