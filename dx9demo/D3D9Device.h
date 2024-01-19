#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class D3D9Device {
public:
    explicit D3D9Device(HWND hWnd);
    ~D3D9Device();

    LPDIRECT3DDEVICE9 device9()
    {
        if (!device9_) {
            init();
        }
        return device9_;
    }

private:
    void init();
    void currentAdapterMode();
    void availableAdatperModes();
    void checkAllFormat();
private:
    HWND hWnd_ = NULL;
    LPDIRECT3D9 d3d9_;
    LPDIRECT3DDEVICE9 device9_ = NULL;
};
