// wmi-info.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <Wbemidl.h>
#include <comdef.h>
#include "device_win.h"
#include <regex>
#include <conio.h> 
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Mmdevapi.lib")
#pragma comment(lib, "Ole32.lib")



#include <Windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <Audiosessiontypes.h>

void output_err(HRESULT hr) {
  if (FAILED(hr))
  {
    LPWSTR errorMessage = nullptr;
    HRESULT result = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMessage, 0, nullptr);
    if (SUCCEEDED(result))
    {
      std::wcout << "Error message: " << errorMessage << std::endl;
      LocalFree(errorMessage);
    }
  }
}

void trigger() {
  HRESULT hr = CoInitialize(nullptr);
  if (SUCCEEDED(hr))
  {
    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (SUCCEEDED(hr))
    {
      IMMDevice* pDevice = nullptr;
      hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
      if (SUCCEEDED(hr))
      {
        IAudioClient* pAudioClient = nullptr;
        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient);
        if (SUCCEEDED(hr))
        {
          hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, nullptr, nullptr);
          if (SUCCEEDED(hr))
          {
            std::cout << "Microphone permission is granted" << std::endl;
          }
          else
          {
            output_err(hr);
            std::cout << "Failed to request microphone permission" << std::endl;
          }
          pAudioClient->Release();
        }
        pDevice->Release();
      }
      pEnumerator->Release();
    }
    CoUninitialize();
  }
}

bool hasMicrophonePermission()
{
  HRESULT hr;
  bool hasPermission = false;

  // Initialize COM
  hr = CoInitialize(NULL);
  if (hr != S_OK) {
    return false;
  }

  // Create an instance of the device enumerator
  IMMDeviceEnumerator* pEnumerator = NULL;
  hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
    CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
    (void**)&pEnumerator);
  if (hr != S_OK) {
    CoUninitialize();
    return false;
  }

  // Get the default audio capture endpoint
  IMMDevice* pDevice = NULL;
  hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &pDevice);
  if (hr != S_OK) {
    pEnumerator->Release();
    CoUninitialize();
    return false;
  }

  // Get the device state
  DWORD state = 0;
  hr = pDevice->GetState(&state);
  if (hr == S_OK && (state & DEVICE_STATEMASK_ALL) == DEVICE_STATE_ACTIVE) {
    hasPermission = true;
  }

  // Release resources
  pDevice->Release();
  pEnumerator->Release();
  CoUninitialize();

  return hasPermission;
}

bool CheckMicrophonePermissions() {
  HRESULT hr;
  IMMDeviceEnumerator* pEnumerator = NULL;
  IMMDevice* pDevice = NULL;
  IAudioClient2* pAudioClient = NULL;
  bool result = false;
  BOOL offloadCapable = FALSE;
  auto hres = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(hres)) {
    cout << "Failed to initialize COM library. Error code = 0x"
      << hex << hres << endl;
    return 1; // Program has failed.
  }
  // 创建设备枚举器
  hr = CoCreateInstance(
    __uuidof(MMDeviceEnumerator),
    NULL,
    CLSCTX_ALL,
    __uuidof(IMMDeviceEnumerator),
    (void**)&pEnumerator
  );
  if (FAILED(hr)) goto Exit;

  // 获取默认的音频捕获设备
  hr = pEnumerator->GetDefaultAudioEndpoint(
    eCapture,
    eCommunications,
    &pDevice
  );
  if (FAILED(hr)) goto Exit;

  // 获取音频客户端
  hr = pDevice->Activate(
    __uuidof(IAudioClient2),
    CLSCTX_ALL,
    NULL,
    (void**)&pAudioClient
  );
  if (FAILED(hr)) goto Exit;

  // 判断是否可以使用音频卸载功能
  //AUDCLNT_STREAMFLAGS streamFlags = AUDCLNT_STREAMFLAGS_NONE;
  hr = pAudioClient->IsOffloadCapable(
    AudioCategory_Speech,
    &offloadCapable
  );
  if (FAILED(hr)) goto Exit;

  // 如果可以使用音频卸载功能，则表明当前进程有麦克风的使用权限
  result = offloadCapable == TRUE;

Exit:
  if (pAudioClient) pAudioClient->Release();
  if (pDevice) pDevice->Release();
  if (pEnumerator) pEnumerator->Release();
  return result;
}

#if 0
int main(int argc, char** argv) {
  nertc::DeviceInterface* device = new nertc::DeviceWin();
  std::cout << "Clock: " << device->GetCpuClock() << std::endl;
  std::cout << "Cores:" << device->GetCpuCores() << std::endl;
  std::cout << "Score:" << device->GetCpuScore() << std::endl;
  std::cout << "Vendor:" << device->GetCpuVender() << std::endl;
  std::cout << "Level:" << device->GetDeviceLevel() << std::endl;
  std::cout << "Ram:" << device->GetRam() << " KB" << std::endl;
  std::cin.ignore();
  std::cin.get();
  return 0;
}

#elif 0
int main(int argc, char** argv)
{
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        cout << "Failed to initialize COM library. Error code = 0x"
             << hex << hres << endl;
        return 1; // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------

    hres = CoInitializeSecurity(
        NULL,
        -1, // COM authentication
        NULL, // Authentication services
        NULL, // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL, // Authentication info
        EOAC_NONE, // Additional capabilities
        NULL // Reserved
    );

    if (FAILED(hres)) {
        cout << "Failed to initialize security. Error code = 0x"
             << hex << hres << endl;
        CoUninitialize();
        return 1; // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        cout << "Failed to create IWbemLocator object."
             << " Err code = 0x"
             << hex << hres << endl;
        CoUninitialize();
        return 1; // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices* pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL, // User name. NULL = current user
        NULL, // User password. NULL = current
        0, // Locale. NULL indicates current
        NULL, // Security flags.
        0, // Authority (for example, Kerberos)
        0, // Context object
        &pSvc // pointer to IWbemServices proxy
    );

    if (FAILED(hres)) {
        cout << "Could not connect. Error code = 0x"
             << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return 1; // Program has failed.
    }

    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;

    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        pSvc, // Indicates the proxy to set
        RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE, // RPC_C_AUTHZ_xxx
        NULL, // Server principal name
        RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL, // client identity
        EOAC_NONE // proxy capabilities
    );

    if (FAILED(hres)) {
        cout << "Could not set proxy blanket. Error code = 0x"
             << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1; // Program has failed.
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM CIM_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        cout << "Query for operating system name failed."
             << " Error code = 0x"
             << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1; // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        wcout << " Name : " << vtProp.bstrVal << endl;
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0; // Program successfully completed.
}

#else

int main() {
  hasMicrophonePermission();
  trigger();
  CheckMicrophonePermissions();
}

#endif