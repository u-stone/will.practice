// dshow-cap-by-grab.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "dshow-cap-by-grab.h"
#include <dshow.h>
#pragma comment(lib, "strmiids.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

class SampleGrabberCallback : public ISampleGrabberCB
{
public:
  STDMETHODIMP_(ULONG) AddRef() { return 1; }
  STDMETHODIMP_(ULONG) Release() { return 2; }
  STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)
  {
    if (riid == IID_IUnknown || riid == IID_ISampleGrabberCB)
    {
      *ppvObject = static_cast<ISampleGrabberCB *>(this);
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  // 回调函数用于处理每一帧的视频数据
  HRESULT STDMETHODCALLTYPE SampleGrabberCallback::SampleCB(double Time, IMediaSample* pSample)
  {
    // 获取视频数据指针
    BYTE* pBuffer = nullptr;
    pSample->GetPointer(&pBuffer);

    // 获取视频数据大小
    long bufferSize = pSample->GetSize();

    // 在这里可以对视频数据进行处理，比如保存、显示等

    return S_OK;
  }

  STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
  {
    // 在这里处理每个 buffer 的回调
    return S_OK;
  }
};


int entry()
{
  CoInitialize(nullptr);

  // 创建Filter Graph Manager对象
  IGraphBuilder* pGraph = nullptr;
  CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);

  // 添加视频捕获设备（如USB摄像头）
  ICaptureGraphBuilder2* pBuilder = nullptr;
  CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuilder);
  pBuilder->SetFiltergraph(pGraph);

  // 创建SampleGrabber和Null Renderer
  IBaseFilter* pSampleGrabber = nullptr;
  CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSampleGrabber);

  IBaseFilter* pNullRenderer = nullptr;
  CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pNullRenderer);

  // 添加Filter到Graph
  pGraph->AddFilter(pSampleGrabber, L"Sample Grabber");
  pGraph->AddFilter(pNullRenderer, L"Null Renderer");

  // 连接Filter
  pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCaptureFilter, pSampleGrabber, pNullRenderer);

  // 配置SampleGrabber回调函数
  ISampleGrabber* pSampleGrabberInterface = nullptr;
  pSampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&pSampleGrabberInterface);
  SampleGrabberCallback* pSampleGrabberCallback = new SampleGrabberCallback();
  pSampleGrabberInterface->SetCallback(pSampleGrabberCallback, 0);

  // 启动Graph
  IMediaControl* pControl = nullptr;
  pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);
  pControl->Run();

  // 等待用户输入
  std::cout << "Press enter to stop" << std::endl;
  std::cin.get();

  // 停止Graph
  pControl->StopWhenReady();

  // 释放资源
  pSampleGrabberInterface->Release();
  pSampleGrabber->Release();
  pNullRenderer->Release();
  pControl->Release();
  pBuilder->Release();
  pGraph->Release();

  CoUninitialize();

  return 0;
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DSHOWCAPBYGRAB, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DSHOWCAPBYGRAB));

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

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DSHOWCAPBYGRAB));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DSHOWCAPBYGRAB);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
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

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
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
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
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
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
