#pragma once
#include "DxCubeRenderWnd.h"
class DxCubeRenderWndWithLights 
  : public DxCubeRenderWnd {
public:
    DxCubeRenderWndWithLights(HINSTANCE hInst, int width, int height);

    void LoadVertexShader() override;
    void LoadPixelShader() override;
    void LoadData() override;
    void Render() override;

 private:
    DxPtr<ID3D11PixelShader> pixel_shader_solid_;
};
