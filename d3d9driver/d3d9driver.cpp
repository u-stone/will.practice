// d3d9driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")

int main()
{
    LPDIRECT3D9 d3d9_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d9_) //初始化Direct3D接口对象，并进行DirectX版本协商
        return 0;

    int count = d3d9_->GetAdapterCount();
    D3DADAPTER_IDENTIFIER9* adapters = (D3DADAPTER_IDENTIFIER9*)malloc(sizeof(D3DADAPTER_IDENTIFIER9) * count);

    for (int index = 0; index < count; ++index) {
        d3d9_->GetAdapterIdentifier(index, 0, &(adapters[index]));
    }
    if (d3d9_) {
        d3d9_->Release();
        d3d9_ = NULL;
    }
}
