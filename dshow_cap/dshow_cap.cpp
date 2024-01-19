// dshow_cap.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "dshow_cap.h"
#include <dshow.h>
#include <string>
#include <comutil.h>
#pragma comment(lib, "strmiids")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Quartz.lib")
#pragma comment(lib, "comsuppw.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

IGraphBuilder *pGraph = nullptr;
ICaptureGraphBuilder2 *pBuild = nullptr;
IBaseFilter *pVideoCap = NULL;
IBaseFilter *pMux = NULL;
IMediaControl* pMediaControl = nullptr;
IBaseFilter* pRendererFilter = nullptr;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DSHOWCAP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DSHOWCAP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    CoUninitialize();
    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DSHOWCAP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DSHOWCAP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
HRESULT InitCaptureGraphBuilder(
  IGraphBuilder **ppGraph,  // Receives the pointer.
  ICaptureGraphBuilder2 **ppBuild  // Receives the pointer.
)
{
  if (!ppGraph || !ppBuild)
  {
    return E_POINTER;
  }
  IGraphBuilder *pGraph = NULL;
  ICaptureGraphBuilder2 *pBuild = NULL;

  // Create the Capture Graph Builder.
  HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
    CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
  if (SUCCEEDED(hr))
  {
    // Create the Filter Graph Manager.
    hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
      IID_IGraphBuilder, (void**)&pGraph);
    if (SUCCEEDED(hr))
    {
      // Initialize the Capture Graph Builder.
      pBuild->SetFiltergraph(pGraph);

      // Return both interface pointers to the caller.
      *ppBuild = pBuild;
      *ppGraph = pGraph; // The caller must release both interfaces.
      return S_OK;
    }
    else
    {
      pBuild->Release();
    }
  }
  return hr; // Failed
}

void enum_cap() {
  printf("in enum_cap\n");
  IAMStreamConfig *pConfig = NULL;
  auto hr = pBuild->FindInterface(
    //&PIN_CATEGORY_PREVIEW, // Preview pin.
    &PIN_CATEGORY_CAPTURE, // capture pin.
    0,    // Any media type.
    pVideoCap, // Pointer to the capture filter.
    IID_IAMStreamConfig, (void**)&pConfig);

  int iCount = 0, iSize = 0;
  hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);

  // Check the size to make sure we pass in the correct structure.
  if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
  {
    // Use the video capabilities structure.

    for (int iFormat = 0; iFormat < iCount; iFormat++)
    {
      VIDEO_STREAM_CONFIG_CAPS scc;
      AM_MEDIA_TYPE *pmtConfig;
      hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
      if (SUCCEEDED(hr))
      {
        /* Examine the format, and possibly use it. */
        // 遍历并找到想要的格式。
        hr = pConfig->SetFormat(pmtConfig);
        // Delete the media type when you are done.
        //DeleteMediaType(pmtConfig);
      }
    }
  }
}

void install_render() {
  // 为视频渲染创建一个渲染器过滤器
  IBaseFilter* pRendererFilter = nullptr;
  auto hr = CoCreateInstance(CLSID_VideoRenderer, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pRendererFilter);

  // 将渲染器过滤器添加到图形中
  hr = pGraph->AddFilter(pRendererFilter, L"Renderer Filter");
}

void start_preview() {
  auto hr = pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
    pVideoCap, NULL, NULL);
}

void capture_to_file() {
  auto hr = pBuild->SetOutputFileName(
    &MEDIASUBTYPE_Avi,  // Specifies AVI for the target file.
    L"D:\\tmp\\Example.avi", // File name.
    &pMux,              // Receives a pointer to the mux.
    NULL);              // (Optional) Receives a pointer to the file sink.
}

void start_capture() {
  auto hr = pBuild->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pVideoCap,
      NULL, pMux);

  // 为视频渲染创建一个渲染器过滤器
  hr = CoCreateInstance(CLSID_VideoRenderer, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pRendererFilter);

  // 连接摄像头设备的输出到渲染器过滤器的输入
  //IPin* pSourceOutputPin = nullptr;
  //IPin* pRendererInputPin = nullptr;
  //hr = pBuild->FindPin(pVideoCap, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, nullptr, FALSE, 0, &pSourceOutputPin);
  //hr = pBuild->FindPin(pRendererFilter, PINDIR_INPUT, nullptr, nullptr, FALSE, 0, &pRendererInputPin);
  //hr = pGraph->ConnectDirect(pSourceOutputPin, pRendererInputPin, nullptr);

  // 运行图形
  hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pMediaControl);
  hr = pMediaControl->Run();
}

void select_device(IMoniker *pMoniker) {
  auto hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVideoCap);
  if (SUCCEEDED(hr))
  {
    hr = pGraph->AddFilter(pVideoCap, L"Capture Filter");
  }
  enum_cap();
  if (SUCCEEDED(hr)) {
    start_preview();
    //capture_to_file();
    start_capture();
  }
}

bool match_device(std::wstring device_name) {
  /*
  Logi C270 HD WebCam
  FaceTime HD Camera (Built-in)
  */
#if 0
  return device_name == L"FaceTime HD Camera (Built-in)";
#else
  return device_name == L"Logi C270 HD WebCam";
#endif
}


void DisplayDeviceInformation(IEnumMoniker *pEnum)
{
  IMoniker *pMoniker = NULL;

  while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
  {
    IPropertyBag *pPropBag;
    HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
    if (FAILED(hr))
    {
      pMoniker->Release();
      continue;
    }

    VARIANT var;
    VariantInit(&var);

    // Get description or friendly name.
    hr = pPropBag->Read(L"Description", &var, 0);
    if (FAILED(hr))
    {
      hr = pPropBag->Read(L"FriendlyName", &var, 0);
    }
    std::wstring device_name;
    if (SUCCEEDED(hr))
    {
      printf("%S\n", var.bstrVal);
      _bstr_t bstrWrapper(var.bstrVal);
      device_name = static_cast<const wchar_t*>(bstrWrapper);
      VariantClear(&var);
    }

    hr = pPropBag->Write(L"FriendlyName", &var);

    // WaveInID applies only to audio capture devices.
    hr = pPropBag->Read(L"WaveInID", &var, 0);
    if (SUCCEEDED(hr))
    {
      printf("WaveIn ID: %d\n", var.lVal);
      VariantClear(&var);
    }

    hr = pPropBag->Read(L"DevicePath", &var, 0);
    if (SUCCEEDED(hr))
    {
      // The device path is not intended for display.
      printf("Device path: %S\n", var.bstrVal);
      VariantClear(&var);
    }

    if (match_device(device_name)) {
      select_device(pMoniker);
    }


    pPropBag->Release();
    pMoniker->Release();
  }
}


HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
  // Create the System Device Enumerator.
  ICreateDevEnum *pDevEnum;
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
    CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

  if (SUCCEEDED(hr))
  {
    // Create an enumerator for the category.
    hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
    if (hr == S_FALSE)
    {
      hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    }
    pDevEnum->Release();
  }
  return hr;
}

void do_capture() {
  //HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  HRESULT hr;
  //if (SUCCEEDED(hr))
  {
    IEnumMoniker *pEnum;

    hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
    if (SUCCEEDED(hr))
    {
      DisplayDeviceInformation(pEnum);
      pEnum->Release();
    }
    //hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
    //if (SUCCEEDED(hr))
    //{
    //  DisplayDeviceInformation(pEnum);
    //  pEnum->Release();
    //}


    //CoUninitialize();
  }
}

void do_stop() {
  pMediaControl->Release();
  pGraph->Release();
  pBuild->Release();
  pVideoCap->Release();
  pRendererFilter->Release();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_ACTION_CAPTURE:
                do_capture();
                break;
            case ID_ACTION_STOP:
              do_stop();
              break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CREATE: {
      InitCaptureGraphBuilder(&pGraph, &pBuild);
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    //UNREFERENCED_PARAMETER(lParam);
    //switch (message)
    //{
    //case WM_INITDIALOG:
    //    return (INT_PTR)TRUE;

    //case WM_COMMAND:
    //    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    //    {
    //        EndDialog(hDlg, LOWORD(wParam));
    //        return (INT_PTR)TRUE;
    //    }
    //    break;
    //}
    return (INT_PTR)FALSE;
}
