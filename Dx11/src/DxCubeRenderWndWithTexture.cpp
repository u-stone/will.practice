#include "DxCubeRenderWndWithTexture.h"
 #include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

using namespace DirectX;

struct SimpleVertex {
  XMFLOAT3 Pos;
  XMFLOAT2 Tex;
};

struct CBNeverChanges {
  XMMATRIX mView;
};

struct CBChangeOnResize {
  XMMATRIX mProjection;
};

struct CBChangesEveryFrame {
  XMMATRIX mWorld;
  XMFLOAT4 vMeshColor;
};

DxCubeRenderWndWithTexture::DxCubeRenderWndWithTexture(HINSTANCE hInst,
                                                       int width, int height)
    : DxCubeRenderWnd(hInst, width, height) {
  vertex_shader_file_ = L"shader/cube_with_texture.fx";
  vertex_entry_ = "VS";
  pixel_shader_file_ = L"shader/cube_with_texture.fx";
  pixel_entry_ = "PS";
}

void DxCubeRenderWndWithTexture::SetUpVertexLayout(DxPtr<ID3DBlob> blob) {
  D3D11_INPUT_ELEMENT_DESC layout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  UINT num = ARRAYSIZE(layout);
  HRESULT hr = d3d11_device_->CreateInputLayout(
      layout, num, blob->GetBufferPointer(), blob->GetBufferSize(), &layout_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  d3d11_context_->IASetInputLayout(layout_);
}

void DxCubeRenderWndWithTexture::LoadData() {
  SimpleVertex vertices[] = {
      {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
      {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
      {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},

      {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
      {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
      {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},

      {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
      {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
      {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},

      {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
      {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
      {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)},
      {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},

      {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f)},
      {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f)},
      {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f)},

      {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f)},
      {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f)},
      {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f)},
  };

  D3D11_BUFFER_DESC bd{0};
  bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA data{0};
  data.pSysMem = vertices;
  HRESULT hr = d3d11_device_->CreateBuffer(&bd, &data, &vertex_buffer_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  UINT stride = sizeof(SimpleVertex);
  UINT offset = 0;
  d3d11_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

  WORD indices[] = {3,  1,  0,  2,  1,  3,  6,  4,  5,  7,  4,  6,
                    11, 9,  8,  10, 9,  11, 14, 12, 13, 15, 12, 14,
                    19, 17, 16, 18, 17, 19, 22, 20, 21, 23, 20, 22};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
  bd.CPUAccessFlags = 0;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  data.pSysMem = indices;
  hr = d3d11_device_->CreateBuffer(&bd, &data, &index_buffer_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  d3d11_context_->IASetIndexBuffer(index_buffer_.get(), DXGI_FORMAT_R16_UINT,
                                   0);
  d3d11_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.ByteWidth = sizeof(CBNeverChanges);
  bd.CPUAccessFlags = 0;

  hr = d3d11_device_->CreateBuffer(&bd, NULL, &cb_never_change_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  bd.ByteWidth = sizeof(CBChangeOnResize);
  hr = d3d11_device_->CreateBuffer(&bd, NULL, &cb_change_on_resize_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  bd.ByteWidth = sizeof(CBChangesEveryFrame);
  hr = d3d11_device_->CreateBuffer(&bd, NULL, &cb_change_on_frame_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  SetUpTextureFilter();
}

void DxCubeRenderWndWithTexture::SetUpProjection() {
  DxCubeRenderWnd::SetUpProjection();

  CBNeverChanges cbNeverChange;
  cbNeverChange.mView = XMMatrixTranspose(view_);
  d3d11_context_->UpdateSubresource(cb_never_change_, 0, NULL, &cbNeverChange,
                                    0, 0);

  CBChangeOnResize cbChangesOnResize;
  cbChangesOnResize.mProjection = XMMatrixTranspose(projection_);
  d3d11_context_->UpdateSubresource(cb_change_on_resize_, 0, NULL,
                                    &cbChangesOnResize, 0, 0);
}

void DxCubeRenderWndWithTexture::Render() {
  using namespace DirectX;
  static ULONGLONG start = ::GetTickCount64();
  static float t = 0.0f;
  t = (::GetTickCount64() - start) / 1000.0f;

  world_ = XMMatrixRotationY(t);

  static XMFLOAT4 vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);
  vMeshColor.x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
  vMeshColor.y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
  vMeshColor.z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

  float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f};
  d3d11_context_->ClearRenderTargetView(target_view_.get(), ClearColor);

  d3d11_context_->ClearDepthStencilView(depth_stencil_view_.get(),
                                        D3D11_CLEAR_DEPTH, 1.0f, 0);

  CBChangesEveryFrame cb;
  cb.mWorld = XMMatrixTranspose(world_);
  cb.vMeshColor = vMeshColor;
  d3d11_context_->UpdateSubresource(cb_change_on_frame_.get(), 0, NULL, &cb, 0,
                                    0);

  // Render cube
  d3d11_context_->VSSetShader(vertex_shader_.get(), NULL, 0);
  d3d11_context_->VSSetConstantBuffers(0, 1, &cb_never_change_);
  d3d11_context_->VSSetConstantBuffers(1, 1, &cb_change_on_resize_);
  d3d11_context_->VSSetConstantBuffers(2, 1, &cb_change_on_frame_);
  d3d11_context_->PSSetShader(pixel_shader_.get(), NULL, 0);
  d3d11_context_->PSSetConstantBuffers(2, 1, &cb_change_on_frame_);
  d3d11_context_->PSSetShaderResources(0, 1, &texture_rv_);
  d3d11_context_->PSSetSamplers(0, 1, &sampler_state_);
  d3d11_context_->DrawIndexed(36, 0, 0);

  swap_chain_->Present(1, 0);

  d3d11_context_->OMSetRenderTargets(1, &target_view_, NULL);
}

void DxCubeRenderWndWithTexture::SetUpTextureFilter() {
  // HRESULT hr =
  //     CreateDDSTextureFromFile(d3d11_device_.get(),
  //     L"E:/Practice/will.practice/Dx11/res/seafloor.dds",
  //                              &texture_, &texture_rv_, NULL, NULL);
  HRESULT hr = CreateWICTextureFromFile(
      d3d11_device_.get(), L"E:/Practice/will.practice/Dx11/res/cyun.jpg",
      &texture_, &texture_rv_, 0);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  D3D11_SAMPLER_DESC sampler{};
  sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampler.MipLODBias = 0;
  sampler.MinLOD = 0;
  sampler.MaxLOD = D3D11_FLOAT32_MAX;
  sampler.ComparisonFunc = D3D11_COMPARISON_NEVER;

  hr = d3d11_device_->CreateSamplerState(&sampler, &sampler_state_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }
}
