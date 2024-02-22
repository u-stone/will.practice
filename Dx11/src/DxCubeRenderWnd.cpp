#include "DxCubeRenderWnd.h"

struct ConstantBuffer {
  DirectX::XMMATRIX mWorld;
  DirectX::XMMATRIX mView;
  DirectX::XMMATRIX mProjection;
};

DxCubeRenderWnd::DxCubeRenderWnd(HINSTANCE hInst, int width, int height)
    : DxRenderWnd(hInst, width, height) {
  vertex_shader_file_ = L"shader/cube.hlsl";
  vertex_entry_ = "VS";
  pixel_shader_file_ = L"shader/cube.hlsl";
  pixel_entry_ = "PS";
}

void DxCubeRenderWnd::Render() {
  using namespace DirectX;
  static ULONGLONG start = ::GetTickCount64();
  static float t = 0.0f;
  t = (::GetTickCount64() - start) / 1000.0f;

  world_ = DirectX::XMMatrixRotationY(t);

  XMMATRIX mspin = XMMatrixRotationZ(-t);
  XMMATRIX mOrbit = XMMatrixRotationY(-t * 2.0f);
  XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0, 0);
  XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
  world2_ = mScale * mspin * mTranslate * mOrbit;

  // Clear color
  float clear_color[] = {0.0f, 0.125f, 0.3f, 1.0f};
  d3d11_context_->ClearRenderTargetView(target_view_.get(), clear_color);

  d3d11_context_->ClearDepthStencilView(depth_stencil_view_.get(),
                                        D3D11_CLEAR_DEPTH, 1.0f, 0);

  // update first cube
  ConstantBuffer cb;
  cb.mWorld = DirectX::XMMatrixTranspose(world_);
  cb.mView = DirectX::XMMatrixTranspose(view_);
  cb.mProjection = DirectX::XMMatrixTranspose(projection_);

  d3d11_context_->UpdateSubresource(const_buffer_.get(), 0, NULL, &cb, 0, 0);

  // render first cube
  d3d11_context_->VSSetShader(vertex_shader_.get(), NULL, 0);
  d3d11_context_->VSSetConstantBuffers(0, 1, &const_buffer_);
  d3d11_context_->PSSetShader(pixel_shader_.get(), NULL, 0);
  d3d11_context_->DrawIndexed(36, 0, 0);

  // update second cube
  ConstantBuffer cb2;
  cb2.mWorld = XMMatrixTranspose(world2_);
  cb2.mView = XMMatrixTranspose(view_);
  cb2.mProjection = XMMatrixTranspose(projection_);

  d3d11_context_->UpdateSubresource(const_buffer_.get(), 0, NULL, &cb2, 0, 0);

  d3d11_context_->DrawIndexed(36, 0, 0);

  swap_chain_->Present(0, 0);

  d3d11_context_->OMSetRenderTargets(1, &target_view_, NULL);
}

void DxCubeRenderWnd::SetUpProjection() {
  using namespace DirectX;

  world_ = XMMatrixIdentity();
  world2_ = XMMatrixIdentity();

  XMVECTOR eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
  XMVECTOR at = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  view_ = XMMatrixLookAtLH(eye, at, up);
  projection_ = XMMatrixPerspectiveFovLH(XM_PIDIV2, width_ / (float)height_,
                                         0.01f, 100.0f);
}

void DxCubeRenderWnd::SetUpStencilView() {
  D3D11_TEXTURE2D_DESC descDepth{0};
  descDepth.Width = width_;
  descDepth.Height = height_;
  descDepth.MipLevels = 1;
  descDepth.ArraySize = 1;
  descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  descDepth.SampleDesc.Count = 1;
  descDepth.SampleDesc.Quality = 0;
  descDepth.Usage = D3D11_USAGE_DEFAULT;
  descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  descDepth.CPUAccessFlags = 0;
  descDepth.MiscFlags = 0;

  HRESULT hr =
      d3d11_device_->CreateTexture2D(&descDepth, NULL, &depth_texture_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
  descDSV.Format = descDepth.Format;
  descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  descDSV.Texture2D.MipSlice = 0;
  hr = d3d11_device_->CreateDepthStencilView(depth_texture_.get(), &descDSV,
                                             &depth_stencil_view_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  d3d11_context_->OMSetRenderTargets(1, &target_view_,
                                     depth_stencil_view_.get());
}

void DxCubeRenderWnd::SetUpVertexLayout(DxPtr<ID3DBlob> blob) {
  D3D11_INPUT_ELEMENT_DESC layout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
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

void DxCubeRenderWnd::LoadPixelShader() {
  DxPtr<ID3DBlob> blob = CreateShader(pixel_shader_file_ /*L"shader/cube.fx"*/,
                                      pixel_entry_ /*"PS"*/, "ps_5_0");
  if (!blob) {
    return;
  }

  HRESULT hr = d3d11_device_->CreatePixelShader(
      blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &pixel_shader_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }
}

void DxCubeRenderWnd::LoadData() {
  using namespace DirectX;

  struct SimpleVertex {
    XMFLOAT3 pos;
    XMFLOAT4 color;
  };

  SimpleVertex vertices[] = {
      {XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
      {XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
      {XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
      {XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)},
      {XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
      {XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)},
      {XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)},
  };

  D3D11_BUFFER_DESC bd{0};
  bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA InitData{0};
  InitData.pSysMem = vertices;

  HRESULT hr = d3d11_device_->CreateBuffer(&bd, &InitData, &vertex_buffer_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  UINT start = 0;
  UINT num = 1;
  UINT stride = sizeof(SimpleVertex);
  UINT offset = 0;

  d3d11_context_->IASetVertexBuffers(start, num, &vertex_buffer_, &stride,
                                     &offset);

  // Create index buffer
  WORD indices[] = {
      3, 1, 0, 2, 1, 3,

      0, 5, 4, 1, 5, 0,

      3, 4, 7, 0, 4, 3,

      1, 6, 5, 2, 6, 1,

      2, 7, 6, 3, 7, 2,

      6, 4, 5, 7, 4, 6,
  };

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;

  InitData.pSysMem = indices;
  hr = d3d11_device_->CreateBuffer(&bd, &InitData, &index_buffer_);
  if (FAILED(hr)) {
    DxError(hr);
    return;
  }

  offset = 0;
  d3d11_context_->IASetIndexBuffer(index_buffer_.get(), DXGI_FORMAT_R16_UINT,
                                   offset);

  d3d11_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Create const buffer
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(ConstantBuffer);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  d3d11_device_->CreateBuffer(&bd, NULL, &const_buffer_);
}
