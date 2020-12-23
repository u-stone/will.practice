// inject-dll.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "inject-dll.h"
#include "ProcessEnumerator.h"
#include <memory>

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
std::shared_ptr<ProcessEnumerator> proc_enum_ = std::make_shared<ProcessEnumerator>();

void inject(ProcInfo pinfo);
void eject(ProcInfo pinfo);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_INJECTDLL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INJECTDLL));

    MSG msg;

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



ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INJECTDLL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_INJECTDLL);
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
        //eject();
        PostQuitMessage(0);
        break;

    case WM_CREATE:
    {
        proc_enum_->grab();
        auto pinfo = proc_enum_->get_proc(L"seh-cpp-exception-win.exe");
        inject(pinfo);

    }
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


void inject(ProcInfo pinfo)
{
    char* remote_bytes = nullptr;
    HANDLE proc = NULL;
    HANDLE thread = NULL;
    while (1) {
        proc = ::OpenProcess(PROCESS_ALL_ACCESS,
            FALSE,
            pinfo.pid);

        if (proc == NULL)
            break;

        const char* lib = "DXHook.dll";
        unsigned int szlib = 1 + strlen(lib);
        remote_bytes = (char*)::VirtualAllocEx(
            proc,
            NULL,
            szlib,
            MEM_COMMIT,
            PAGE_READWRITE
        );

        if (remote_bytes == nullptr)
            break;

        if (!::WriteProcessMemory(
            proc,
            remote_bytes,
            lib,
            szlib,
            NULL
        ))
            break;

        PTHREAD_START_ROUTINE thread_func = (PTHREAD_START_ROUTINE)
            ::GetProcAddress(::GetModuleHandleA("Kernel32"), "LoadLibraryA");
        if (thread_func == nullptr) 
            break;

        thread = ::CreateRemoteThread(
            proc,
            NULL,
            0,
            thread_func,
            remote_bytes,
            0,
            NULL
        );
        if (thread == NULL)
            break;

        ::WaitForSingleObject(thread, INFINITE);
        break;
    }

        if (remote_bytes) {
            ::VirtualFreeEx(proc, remote_bytes, 0, MEM_RELEASE);
        }
        if (thread) {
            CloseHandle(thread);
        }
        if (proc) {
            CloseHandle(proc);
        }
}

void eject(ProcInfo pinfo)
{

}
