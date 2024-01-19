#pragma once
#include <Windows.h>
#include <vector>
#include <dxgi.h>

class Wrapper {
public:
    HRESULT InitDevice(HWND hWnd);
    std::vector<IDXGIAdapter*> EnumerateAdapterAndMore();
};
