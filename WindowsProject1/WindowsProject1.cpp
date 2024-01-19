// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "WindowsProject1.h"
#include <sstream>
#include <shellapi.h>
#include <fstream>
#pragma comment(lib, "Msimg32.lib")
#include "WndShadow.h"
#include "tracker_wnd_win.h"
#include <thread>
#include <iostream>

#define MAX_LOADSTRING 100
using namespace webrtc;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
CWndShadow WndShadow;
TrackerWnd::TrackerWndPtr tracker_wnd;
HANDLE hEvent = NULL;
std::thread working_thread;

class Base : std::enable_shared_from_this<Base> {
public:
  std::shared_ptr<Base> getptr() {
    return shared_from_this();
  }

  struct Options {

    int thickness = 5;
    COLORREF hight_light_clr = RGB(255, 128, 0);
    bool start_on_create = true;
  };

  static std::shared_ptr<Base> create() {
    return std::shared_ptr<Base>(new Base(1));
  }

  // Destructor MUST be public.
  ~Base() {}
private:
  explicit Base(int) {}

  static int c;

  friend class TrackerWnd;
};

int Base::c = 0;

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
    SetProcessDPIAware();
    //SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    // TODO: Place code here.
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
    hEvent = CreateEvent(&sa, TRUE, FALSE, NULL);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

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
    auto base = Base::create();

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
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
   //CWndShadow::Initialize(hInst);
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   std::unique_ptr<std::string> sp = std::make_unique<std::string>("123");

#if 0
   __try {
     void* addr = (void*)0x1C472000;
     char buffer[128] = { 0 };
     memcpy(buffer, addr, 128);
   }
   __except (EXCEPTION_EXECUTE_HANDLER) {

   }
#endif

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void NonUIThread()
{
    if (tracker_wnd == nullptr) {
        TrackerWnd::Options g_options;
        g_options.type = TrackerWnd::TrackSourceType::kWindow;
        g_options.tracked_wnd = (HWND)0x00C40D5C;
        RECT rc = { 0,0,0,0 };
        //g_options.monitor = ::MonitorFromRect(&rc, MONITOR_DEFAULTTOPRIMARY);

        g_options.hight_light_clr = 0xFF00FFFF;// RGB(0, 222, 126);
        g_options.thickness = 6;
        g_options.start_on_create = false;
        tracker_wnd = TrackerWnd::create(g_options);
        tracker_wnd->start();
    } else {
        tracker_wnd->start();
    }

    WaitForSingleObject(hEvent, INFINITE);

    if (tracker_wnd) {
      tracker_wnd->stop();
      tracker_wnd = nullptr;
    }
}

bool SaveAsBmp(std::string file_path, int width, int height, uint8_t* pBmp) {
  int bitCount = 32;
  HANDLE hFile = CreateFileA(file_path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return false;
  }
  int bmp_size = width * height * (bitCount / 8);
  BITMAPFILEHEADER bmpHeader;
  bmpHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp_size;
  bmpHeader.bfType = 0x4D42;
  bmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmpHeader.bfReserved1 = 0;
  bmpHeader.bfReserved2 = 0;
  DWORD dwBytesWritten = 0;
  if (!WriteFile(hFile, (LPSTR)&bmpHeader, sizeof(bmpHeader), &dwBytesWritten, NULL)) {
    return false;
  }
  BITMAPINFOHEADER bmiHeader;
  bmiHeader.biSize = sizeof(bmiHeader);
  bmiHeader.biWidth = width;
  bmiHeader.biHeight = -height;
  bmiHeader.biPlanes = 1;
  bmiHeader.biBitCount = bitCount;
  bmiHeader.biCompression = 0;
  bmiHeader.biSizeImage = bmp_size;
  bmiHeader.biXPelsPerMeter = 0;
  bmiHeader.biYPelsPerMeter = 0;
  bmiHeader.biClrUsed = 0;
  bmiHeader.biClrImportant = 0;
  if (!WriteFile(hFile, (LPSTR)&bmiHeader, sizeof(bmiHeader), &dwBytesWritten, NULL)) {
    return false;
  }

  if (!WriteFile(hFile, pBmp, bmp_size, &dwBytesWritten, NULL)) {
    return false;
  }

  if (!CloseHandle(hFile)) {
    return false;
  }

  return true;
}

bool testAPI() {
  HWND  hWnd = NULL;
  //RECT rcTarget;
  //GetWindowRect(hWnd, &rcTarget);
  //long left = (long)GetSystemMetrics(SM_XVIRTUALSCREEN);
  //long top = (long)GetSystemMetrics(SM_YVIRTUALSCREEN);
  //long right = left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
  //long bottom = top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
  //rcTarget.left = std::max(rcTarget.left, left);
  //rcTarget.right = std::min(rcTarget.right, right);
  //rcTarget.top = std::max(rcTarget.top, top);
  //rcTarget.bottom = std::min(rcTarget.bottom, bottom);

  ////if (GetDesktopWindow() == hWnd) {
  ////  hWnd = GetWindow(GetTopWindow(hWnd), GW_HWNDLAST);
  ////}

  //auto isTransparent = [](HWND hWnd) {
  //  return (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_LAYERED) &&
  //    (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT);
  //};

  //bool bOccluded = false;
  //while (NULL != (hWnd = GetNextWindow(hWnd, GW_HWNDPREV))) {
  //  if (IsWindow(hWnd) && IsWindowVisible(hWnd) && IsWindowEnabled(hWnd) &&
  //    !isTransparent(hWnd) && (GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) &&
  //    (GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)) {
  //    RECT rcWnd;
  //    GetWindowRect(hWnd, &rcWnd);
  //    if (!((rcWnd.right < rcTarget.left) || (rcWnd.left > rcTarget.right) ||
  //      (rcWnd.bottom < rcTarget.top) || (rcWnd.top > rcTarget.bottom))) {
  //      if (!bOccluded) {
  //        bOccluded = true;
  //      }
  //      RECT interRect;
  //      if (IntersectRect(&interRect, &rcWnd, &rcTarget) &&
  //        !::IsRectEmpty(&interRect)) {
  //        bOccluded = true;
  //        break;
  //      }
  //    }
  //  }
  //}
  //std::cout << bOccluded;
  INFINITE;
  HANDLE h = CreateEvent(NULL, TRUE, TRUE, L"qwe123");
  //WaitForSingleObjectEx(h, 0x5678, TRUE);
  ::SendMessage(hWnd, 0xFF, 1, 2);

  int num_retries = 0;
  while (true) {
    HWND hwnd = nullptr;
    while ((hwnd = FindWindowEx(nullptr, hwnd, nullptr, nullptr))) {
      //if (hwnd == selected_window()) {
      //  // Windows are enumerated in top-down Z-order, so we can stop
      //  // enumerating upon reaching the selected window & report it's on top.
      //  return true;
      //}

      // Ignore the excluded window.
      //if (hwnd == excluded_window) {
      //  continue;
      //}

      // Ignore windows that aren't visible on the current desktop.
      //if (!window_capture_helper()->IsWindowVisibleOnCurrentDesktop(hwnd)) {
      //  continue;
      //}

      // Ignore Chrome notification windows, especially the notification for
      // the ongoing window sharing. Notes:
      // - This only works with notifications from Chrome, not other Apps.
      // - All notifications from Chrome will be ignored.
      // - This may cause part or whole of notification window being cropped
      // into the capturing of the target window if there is overlapping.
      //if (window_capture_helper()->IsWindowChromeNotification(hwnd)) {
      //  continue;
      //}

      // Ignore windows owned by the selected window since we want to capture
      // them.
      //if (IsWindowOwnedBySelectedWindow(hwnd)) {
      //  continue;
      //}

      // Check whether this window intersects with the selected window.
      //if (IsWindowOverlappingSelectedWindow(hwnd)) {
      //  // If intersection is not empty, the selected window is not on top.
      //  return false;
      //}
      std::cout << hwnd;
    }

    DWORD lastError = GetLastError();
    if (lastError == ERROR_SUCCESS) {
      // The enumeration completed successfully without finding the selected
      // window (which may have been closed).
    }
    else if (lastError == ERROR_INVALID_WINDOW_HANDLE) {
      // This error may occur if a window is closed around the time it's
      // enumerated; retry the enumeration in this case up to 10 times
      // (this should be a rare race & unlikely to recur).
      if (++num_retries <= 10) {
        continue;
      }
      else {
      }
    }

    // The enumeration failed with an unexpected error (or more repeats of
    // an infrequently-expected error than anticipated). After logging this &
    // firing an assert when enabled, report that the selected window isn't
    // topmost to avoid inadvertent capture of other windows.
    return false;
  }
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
    case WM_CREATE: {
      //WndShadow.Create(hWnd);
      //WndShadow.SetSize(50);
      //WndShadow.SetSharpness(5);
      //WndShadow.SetDarkness(100);
      //WndShadow.SetPosition(10, 10);
      //WndShadow.SetColor(RGB(255, 0, 0));
      auto p = new char[1024];
      testAPI();
    } break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            HDC screen = GetDC(NULL);
            auto cap = GetDeviceCaps(screen, HORZRES);
            auto cap2 = GetDeviceCaps(screen, DESKTOPHORZRES);
            auto cap3 = cap2 * 96.0 / cap;
            std::ostringstream ss;
            ss << "cap = " << cap << ", cap2 = " << cap2 << ", cap3 = " << cap3;
            TextOutA(hdc, 20, 20, ss.str().c_str(), ss.str().size());

#if 0
            auto path = R"(E:\Practice\will.practice\x64\Debug\cpu.exe)";
            //auto path = R"(C:\Program Files (x86)\VideoLAN\VLC\vlc.exe)";
            SHFILEINFOA sfi = { 0 };
            DWORD_PTR hr = SHGetFileInfoA(path, -1, &sfi,
              sizeof(sfi), SHGFI_ICON | SHGFI_DISPLAYNAME);
            if (SUCCEEDED(hr)) {
              ICONINFOEXW info;
              info.cbSize = sizeof(info);
              GetIconInfoEx(sfi.hIcon, &info);

              {
                DWORD dwWidth = info.xHotspot * 40;
                DWORD dwHeight = info.yHotspot * 40;
                HDC hDc = ::GetDC(NULL);
                //SetBkMode(hDc, TRANSPARENT);
                HDC hMemDc = ::CreateCompatibleDC(hDc);
                HBITMAP hBitMap = ::CreateCompatibleBitmap(hDc, dwWidth, dwHeight);
                if (!hBitMap) return NULL;
                ::SelectObject(hMemDc, hBitMap);
                RECT rc{ 0, 0, dwWidth, dwHeight };
                ::DrawIconEx(hMemDc, 0, 0, sfi.hIcon, dwWidth, dwHeight, 0, NULL, DI_NORMAL);
                BitBlt(hdc, 200, 300, dwWidth, dwHeight, hMemDc, 0, 0, SRCCOPY);
                //TransparentBlt(hdc, 200, 300, dwWidth, dwHeight, hMemDc, 0, 0, dwWidth, dwHeight, 0);


                // Get the BITMAP from the HBITMAP.
                BITMAP bmp;
                GetObject(hBitMap, sizeof(BITMAP), &bmp);

                BITMAPINFOHEADER bi;
                bi.biSize = sizeof(BITMAPINFOHEADER);
                bi.biWidth = bmp.bmWidth;
                bi.biHeight = -bmp.bmHeight;
                bi.biPlanes = 1;
                bi.biBitCount = 32;
                bi.biCompression = BI_RGB;
                bi.biSizeImage = 0;
                bi.biXPelsPerMeter = 0;
                bi.biYPelsPerMeter = 0;
                bi.biClrUsed = 0;
                bi.biClrImportant = 0;

                auto len_ = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
                std::shared_ptr<uint8_t> data(new uint8_t[len_]);
                ::GetDIBits(hDc, hBitMap, 0, (UINT)bmp.bmHeight, data.get(),
                  (BITMAPINFO*)&bi, DIB_RGB_COLORS);

                SaveAsBmp("D:/tmp/1.bmp", bmp.bmWidth, bmp.bmHeight, data.get());

                ::DeleteDC(hMemDc);
                ::ReleaseDC(NULL, hDc);

              }
            }

            if (sfi.hIcon) {
              DestroyIcon(sfi.hIcon);
            }
#endif
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY: {
      SetEvent(hEvent);
      if (working_thread.joinable()) {
        working_thread.detach();
      }
      ResetEvent(hEvent);
      PostQuitMessage(0);
      break;
    }
    case WM_LBUTTONUP: {
      auto p = new char[2048];
        //HWND hwnd = reinterpret_cast<HWND>(0x00251042);
        //ShowWindow(hwnd, SW_RESTORE);
        //SetForegroundWindow(hwnd);
      //testAPI();
#if 0
      if (!working_thread.joinable()) {
        working_thread = std::move(std::thread(NonUIThread));
        //working_thread.detach();
      }
#endif
      //working_thread.detach();
#if 0
        HICON icon[10] = {0};
        HICON icon_s[10] = {0};
        ICONINFOEXW info;
        info.cbSize = sizeof(info);

        auto path = R"(E:\Practice\will.practice\x64\Release\dx9demo.exe)";
        SHFILEINFOA sfi = { 0 };
        DWORD_PTR hr = SHGetFileInfoA(path, -1, &sfi,
          sizeof(sfi), SHGFI_ICON | SHGFI_DISPLAYNAME);
        if (SUCCEEDED(hr)) {
          GetIconInfoEx(sfi.hIcon, &info);
        }

        int total = ExtractIconExA(path, -1, 0, 0, 1);
        int count = ExtractIconExA(path, 0, icon, icon_s, 1);
        auto w = GetSystemMetrics(SM_CXICON);
        auto h = GetSystemMetrics(SM_CYICON);
        for (auto item : icon) {
          if (item)
            GetIconInfoEx(item, &info);
          else
            break;
        }

        w = GetSystemMetrics(SM_CXSMICON);
        h = GetSystemMetrics(SM_CYSMICON);
        for (auto item : icon_s) {
          if (item)
            GetIconInfoEx(item, &info);
          else
            break;
        }
        break;
#endif
    }
                       break;
    case WM_RBUTTONUP: {
      SetEvent(hEvent);
      if (working_thread.joinable()) {
        working_thread.detach();
      }
      ResetEvent(hEvent);
    } break;
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
