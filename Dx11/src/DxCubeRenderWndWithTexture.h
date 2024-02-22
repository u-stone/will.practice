#pragma once
#include "DxCubeRenderWnd.h"
class DxCubeRenderWndWithTexture : public DxCubeRenderWnd {
 public:
  DxCubeRenderWndWithTexture(HINSTANCE hInst, int width, int height);
  void Render() override;

 protected:
  void SetUpVertexLayout(DxPtr<ID3DBlob> blob) override;
  void LoadData() override;
  void SetUpProjection() override;

 private:
  void SetUpTextureFilter();

 protected:
  DxPtr<ID3D11Buffer> cb_never_change_;
  DxPtr<ID3D11Buffer> cb_change_on_resize_;
  DxPtr<ID3D11Buffer> cb_change_on_frame_;
  DxPtr<ID3D11Resource> texture_;
  DxPtr<ID3D11ShaderResourceView> texture_rv_;
  DxPtr<ID3D11SamplerState> sampler_state_;
};
