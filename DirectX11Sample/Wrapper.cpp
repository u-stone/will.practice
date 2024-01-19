#include "Wrapper.h"
#include <d3d11.h>
#include <memory>

HRESULT Wrapper::InitDevice(HWND hWnd)
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] = {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    IDXGISwapChain* g_pSwapChain = NULL;
    ID3D11Device* g_pd3dDevice = NULL;
    ID3D11DeviceContext* g_pImmediateContext = NULL;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // get all adapters.
    auto adapters = EnumerateAdapterAndMore();

    ID3D11Texture2D* pBackBuffer;
    // Get a pointer to the back buffer
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
      (LPVOID*)&pBackBuffer);

    // Create a render-target view
    ID3D11RenderTargetView* g_pRenderTargetView = NULL;
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL,
      &g_pRenderTargetView);

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = 640;
    vp.Height = 480;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    return E_NOTIMPL;
}

std::vector<IDXGIAdapter*> Wrapper::EnumerateAdapterAndMore()
{
    IDXGIAdapter* pAdapter;
    std::vector<IDXGIAdapter*> vAdapters;
    IDXGIFactory* pFactory = NULL;

    // Create a DXGIFactory object.
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory))) {
        return vAdapters;
    }

    for (UINT i = 0;
         pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
         ++i) {
        vAdapters.push_back(pAdapter);

        IDXGIOutput* pOutput = NULL;
        HRESULT hr = pAdapter->EnumOutputs(0, &pOutput);
        if (FAILED(hr)) {
            continue;
        }
        DXGI_ADAPTER_DESC desc;
        hr = pAdapter->GetDesc(&desc);

        UINT numModes = 0;

        for (int index = 0; index < DXGI_FORMAT_V408; ++index) {
            DXGI_FORMAT format = static_cast<DXGI_FORMAT>(index);

            // Get the number of elements
            hr = pOutput->GetDisplayModeList(format, 0, &numModes, NULL);
            if (FAILED(hr) || numModes == 0) {
                continue;
            }
            std::unique_ptr<DXGI_MODE_DESC[]> displayModes(new DXGI_MODE_DESC[numModes]);

            // Get the list
            hr = pOutput->GetDisplayModeList(format, 0, &numModes, displayModes.get());
            if (SUCCEEDED(hr)) {
                for (int index = 0; index < numModes; ++index) {
                    auto mode = displayModes.get()[index];
                    mode.Height = mode.Height;
                }
            }
        }
    }

    if (pFactory) {
        pFactory->Release();
    }

    return vAdapters;
}
