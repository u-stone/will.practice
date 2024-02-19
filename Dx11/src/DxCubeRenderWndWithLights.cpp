#include "DxCubeRenderWndWithLights.h"

using namespace DirectX;

struct ConstantBuffer {
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMFLOAT4 vLightDir[2];
    XMFLOAT4 vLightColor[2];
    XMFLOAT4 vOutputColor;
};

DxCubeRenderWndWithLights::DxCubeRenderWndWithLights(HINSTANCE hInst, int width, int height)
    : DxCubeRenderWnd(hInst, width, height)
{
    vertex_shader_file_ = L"E:/Practice/will.practice/Dx11/shader/cube_with_light.fx";
    vertex_entry_ = "VS";
    pixel_shader_file_ = L"E:/Practice/will.practice/Dx11/shader/cube_with_light.fx";
    pixel_entry_ = "PS";
}

void DxCubeRenderWndWithLights::SetUpVertexLayout(DxPtr<ID3DBlob> blob)
{
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT num = ARRAYSIZE(layout);

    HRESULT hr = d3d11_device_->CreateInputLayout(layout, num, blob->GetBufferPointer(), blob->GetBufferSize(), &layout_);
    if (FAILED(hr)) {
        DxError(hr);
        return;
    }

    d3d11_context_->IASetInputLayout(layout_);
}

void DxCubeRenderWndWithLights::LoadPixelShader()
{
    DxRenderWnd::LoadPixelShader();

    DxPtr<ID3DBlob> blob_solid = CreateShader(pixel_shader_file_, "PSSolid", "ps_5_0");
    if (!blob_solid) {
        return;
    }

    HRESULT hr = d3d11_device_->CreatePixelShader(blob_solid->GetBufferPointer(), blob_solid->GetBufferSize(), NULL, &pixel_shader_solid_);
    if (FAILED(hr)) {
        DxError(hr);
        return;
    }
}

void DxCubeRenderWndWithLights::LoadData()
{
    struct SimpleVertex {
        XMFLOAT3 Pos;
        XMFLOAT3 Normal;
    };

    SimpleVertex vertices[] = {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC bd { 0 };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA data { 0 };
    data.pSysMem = vertices;
    HRESULT hr = d3d11_device_->CreateBuffer(&bd, &data, &vertex_buffer_);
    if (FAILED(hr)) {
        DxError(hr);
        return;
    }

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    d3d11_context_->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

    WORD indices[] = {
        3, 1, 0,
        2, 1, 3,

        6, 4, 5,
        7, 4, 6,

        11, 9, 8,
        10, 9, 11,

        14, 12, 13,
        15, 12, 14,

        19, 17, 16,
        18, 17, 19,

        22, 20, 21,
        23, 20, 22
    };
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    data.pSysMem = indices;
    hr = d3d11_device_->CreateBuffer(&bd, &data, &index_buffer_);
    if (FAILED(hr)) {
        DxError(hr);
        return;
    }

    d3d11_context_->IASetIndexBuffer(index_buffer_.get(), DXGI_FORMAT_R16_UINT, 0);

    d3d11_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = d3d11_device_->CreateBuffer(&bd, &data, &const_buffer_);
    if (FAILED(hr)) {
        DxError(hr);
        return;
    }
}

void DxCubeRenderWndWithLights::Render()
{
    using namespace DirectX;
    static ULONGLONG start = ::GetTickCount64();
    static float t = 0.0f;
    t = (::GetTickCount64() - start) / 1000.0f;

    world_ = DirectX::XMMatrixRotationY(t);

    XMFLOAT4 vLightDirs[2] = {
        XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
        XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
    };

    XMFLOAT4 vLightColors[2] = {
        XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
        XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f)
    };

    XMMATRIX mRotate = XMMatrixRotationY(-t * 2.0f);
    XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
    vLightDir = XMVector3Transform(vLightDir, mRotate);
    XMStoreFloat4(&vLightDirs[1], vLightDir);

    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
    d3d11_context_->ClearRenderTargetView(target_view_.get(), ClearColor);
    d3d11_context_->ClearDepthStencilView(depth_stencil_view_.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    ConstantBuffer cb;
    cb.mWorld = XMMatrixTranspose(world_);
    cb.mView = XMMatrixTranspose(view_);
    cb.mProjection = XMMatrixTranspose(projection_);
    cb.vLightDir[0] = vLightDirs[0];
    cb.vLightDir[1] = vLightDirs[1];
    cb.vLightColor[0] = vLightColors[0];
    cb.vLightColor[1] = vLightColors[1];
    cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    d3d11_context_->UpdateSubresource(const_buffer_.get(), 0, NULL, &cb, 0, 0);

    d3d11_context_->VSSetShader(vertex_shader_.get(), NULL, 0);
    d3d11_context_->VSSetConstantBuffers(0, 1, &const_buffer_);
    d3d11_context_->PSSetShader(pixel_shader_.get(), NULL, 0);
    d3d11_context_->PSSetConstantBuffers(0, 1, &const_buffer_);
    d3d11_context_->DrawIndexed(36, 0, 0);

    for (int m = 0; m < 2; ++m) {
        XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&vLightDirs[m]));
        XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
        mLight = mLightScale * mLight;

        cb.mWorld = XMMatrixTranspose(mLight);
        cb.vOutputColor = vLightColors[m];

        d3d11_context_->UpdateSubresource(const_buffer_.get(), 0, NULL, &cb, 0, 0);

        d3d11_context_->PSSetShader(pixel_shader_solid_.get(), 0, 0);
        d3d11_context_->DrawIndexed(36, 0, 0);
    }

    swap_chain_->Present(1, 0);

    d3d11_context_->OMSetRenderTargets(1, &target_view_, NULL);
}
