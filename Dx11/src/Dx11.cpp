// Dx11.cpp : Defines the entry point for the application.
//

#include "../framework.h"
#include "Dx11.h"
#include "DxRenderWnd.h"
#include "DxCubeRenderWnd.h"
#include "DxCubeRenderWndWithLights.h"
#include "DxCubeRenderWndWithTexture.h"
#include <memory>


#include <d3dcompiler.h>
#include <iostream>
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

std::string GetShaderCodeFromEffectFile(const std::wstring& filePath) {
  // Step 1: Load the Effect object file
  ID3DBlob* pEffectBlob = nullptr;
  HRESULT hr = D3DReadFileToBlob(filePath.c_str(), &pEffectBlob);
  if (FAILED(hr)) {
    // Handle error
    DxError(hr);
    return "";
  }

  // Step 2: Reflect the Effect object
  ID3D11ShaderReflection* pShaderReflection = nullptr;
  hr = D3DReflect(pEffectBlob->GetBufferPointer(), pEffectBlob->GetBufferSize(),
                  IID_ID3D11ShaderReflection, (void**)&pShaderReflection);
  if (FAILED(hr)) {
    // Handle error
    pEffectBlob->Release();
    DxError(hr);
    return "";
  }

  // Step 3: Get the shader code
  D3D11_SHADER_DESC shaderDesc;
  hr = pShaderReflection->GetDesc(&shaderDesc);
  if (FAILED(hr)) {
    // Handle error
    pShaderReflection->Release();
    pEffectBlob->Release();
    return "";
  }

  std::string shaderCode;
  // shaderCode.resize(shaderDesc.BytecodeLength);
  // memcpy(shaderCode.data(), pEffectBlob->GetBufferPointer() +
  // shaderDesc.BytecodeOffset, shaderDesc.BytecodeLength);

  // Clean up resources
  pShaderReflection->Release();
  pEffectBlob->Release();

  return shaderCode;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);
  std::wstring filePath =
      L"C:/Code/GitHub/will.practice/fxo_to_text/data/video_render.fxo";
  std::string shaderCode = GetShaderCodeFromEffectFile(filePath);

  auto render =
      std::make_shared<DxCubeRenderWndWithLights>(hInstance, 1920, 1080);
  render->Init();

  // Main message loop:
  MSG msg = { 0 };
  while (WM_QUIT != msg.message)
  {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      render->Render();
    }
  }

  return (int)msg.wParam;
}
