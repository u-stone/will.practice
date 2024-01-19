#include "CMyDShow.h"

HRESULT CMyDShow::InitCaptureGraphBuilder(
    IGraphBuilder** ppGraph, // Receives the pointer.
    ICaptureGraphBuilder2** ppBuild // Receives the pointer.
)
{
    if (!ppGraph || !ppBuild) {
        return E_POINTER;
    }
    IGraphBuilder* pGraph = NULL;
    ICaptureGraphBuilder2* pBuild = NULL;

    // Create the Capture Graph Builder.
    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
        CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
    if (SUCCEEDED(hr)) {
        // Create the Filter Graph Manager.
        hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
            IID_IGraphBuilder, (void**)&pGraph);
        if (SUCCEEDED(hr)) {
            // Initialize the Capture Graph Builder.
            pBuild->SetFiltergraph(pGraph);

            // Return both interface pointers to the caller.
            *ppBuild = pBuild;
            *ppGraph = pGraph; // The caller must release both interfaces.
            return S_OK;
        } else {
            pBuild->Release();
        }
    }
    return hr; // Failed
}