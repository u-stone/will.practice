#if !defined WEBRTC_DISABLE_VIDEO
#include "WindowListHelper.h"
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#define WM_UNHOOKWIN (WM_USER + 8809)

namespace lava {

static HWND gCurrentCapturedWnd{ NULL };

bool inline MatchTitleAndClassName(WindowInfo const& window, std::wstring const& /*title*/, std::wstring const& className)
{
    return wcscmp(window.ClassName.c_str(), className.c_str()) == 0;
}

bool IsKnownBlockedWindow(WindowInfo const& window)
{
    return
        // Task View
        //MatchTitleAndClassName(window, L"Task View", L"Windows.UI.Core.CoreWindow") ||
        // XAML Islands
        MatchTitleAndClassName(window, L"DesktopWindowXamlSource", L"Windows.UI.Core.CoreWindow") ||
        // XAML Popups
        MatchTitleAndClassName(window, L"PopupHost", L"Xaml_WindowedPopupClass");
}

bool IsCapturableWindow(WindowInfo const& window)
{
    if (window.WindowHandle == GetShellWindow() || !IsWindowVisible(window.WindowHandle) || GetAncestor(window.WindowHandle, GA_ROOT) != window.WindowHandle) {
        return false;
    }

    RECT rc;
    GetWindowRect(window.WindowHandle, &rc);
    if (IsRectEmpty(&rc)) {
        return false;
    }

    auto style = GetWindowLongW(window.WindowHandle, GWL_STYLE);
    if ((style & WS_POPUP) && gCurrentCapturedWnd != NULL) {
        // All kind of popup window should be filtered, including :
        // popup menu, dwm preview window, tooltips, SysShadow(the shadow of dwm window), and so on.
        // An exception is the owner is current captured window.
        if (GetWindow(window.WindowHandle, GW_OWNER) == gCurrentCapturedWnd) {
            return false;
        }

        return true;
    }

    if (style & WS_DISABLED) {
        return false;
    }

    /// ComboBox has style WS_EX_TOOLWINDOW, but it should be filtered.
    /// and tool tips can be excluded by WS_POPUP style feature as above.
    //auto exStyle = GetWindowLongW(window.WindowHandle, GWL_EXSTYLE);
    //if (exStyle & WS_EX_TOOLWINDOW)    // No tooltips
    //{
    //    return false;
    //}

    // Check to see if the window is cloaked if it's a UWP
    //if (wcscmp(window.ClassName.c_str(), L"Windows.UI.Core.CoreWindow") == 0 || wcscmp(window.ClassName.c_str(), L"ApplicationFrameWindow") == 0) {
    //    DWORD cloaked = FALSE;
    //    if (SUCCEEDED(DwmGetWindowAttribute(window.WindowHandle, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && (cloaked == DWM_CLOAKED_SHELL)) {
    //        return false;
    //    }
    //}

    // Unfortunate work-around. Not sure how to avoid this.
    if (IsKnownBlockedWindow(window)) {
        return false;
    }

    return true;
}

static thread_local WindowList* WindowListForThread;

// Some applications that has shadow window, like popo, has no title, but these windows should be excluded.
WindowList::WindowList()
{
    if (!IsGUIThread(FALSE)) {
        throw std::exception("Current thread is not a GUI thread, process will be blocked");
    }

    if (WindowListForThread) {
        throw std::exception("WindowList already exists for this thread!");
    }

    EnumWindows([](HWND hwnd, LPARAM lParam) {
        {
            auto window = WindowInfo(hwnd);

            if (!IsCapturableWindow(window)) {
                return TRUE;
            }

            auto windowList = reinterpret_cast<WindowList*>(lParam);
            windowList->AddWindow(window);
        }

        return TRUE;
    },
        reinterpret_cast<LPARAM>(this));

    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
    event_quit_ = CreateEvent(&sa, TRUE, FALSE, NULL);
    work_thread_ = CreateThread(&sa, 0, WindowList::WorkProc, this, 0, nullptr);
    CloseHandle(work_thread_);
}

WindowList::~WindowList()
{
    PostMessage(msg_wnd_, WM_UNHOOKWIN, (WPARAM)forewnd_change_hook_, 0);
    PostMessage(msg_wnd_, WM_UNHOOKWIN, (WPARAM)wnd_hook_, 0);
    PostMessage(msg_wnd_, WM_QUIT, 0, 0);

    if (event_quit_ != 0) {
        WaitForSingleObject(event_quit_, INFINITE);
        CloseHandle(event_quit_);
        event_quit_ = 0;
    }

    msg_wnd_ = 0;
}

const std::vector<WindowInfo> WindowList::GetCurrentWindows()
{
    changed_ = false;
    std::vector<WindowInfo> ret;
    {
        std::lock_guard<std::mutex> lock(wnd_list_mutex_);
        ret = windows_;
    }
    return ret;
}

bool WindowList::WindowsChangedSinceLastGet()
{
    return changed_;
}

void WindowList::SetCurrentCapturedWnd(HWND wnd)
{
    gCurrentCapturedWnd = wnd;
}

LRESULT WindowList::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_UNHOOKWIN: {
        auto hook = reinterpret_cast<HWINEVENTHOOK>(wParam);
        if (hook) {
            UnhookWinEvent(hook);
        }
    } break;
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD __stdcall WindowList::WorkProc(LPVOID pv)
{
    auto self = static_cast<WindowList*>(pv);

    WindowListForThread = self;

    WNDCLASSEX wcex = {};

    constexpr wchar_t kClassName[] = L"{E4708904-BCA9-4C97-839F-D801E32F1F3A}";

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
    wcex.lpszClassName = kClassName;

    RegisterClassEx(&wcex);

    self->msg_wnd_ = CreateWindow(kClassName, L"", WS_CLIPCHILDREN, 0, 0, 0, 0, HWND_MESSAGE, 0, wcex.hInstance, NULL);

    self->wnd_hook_ = SetWinEventHook(EVENT_OBJECT_DESTROY, /*EVENT_OBJECT_SHOW*/ EVENT_OBJECT_UNCLOAKED, nullptr,
        [](HWINEVENTHOOK eventHook, DWORD event, HWND hwnd, LONG objectId, LONG childId, DWORD eventThreadId, DWORD eventTimeInMilliseconds) {
            if (objectId == OBJID_WINDOW && childId == CHILDID_SELF && hwnd != nullptr && GetAncestor(hwnd, GA_ROOT) == hwnd && event == EVENT_OBJECT_DESTROY) {
                WindowListForThread->RemoveWindow(WindowInfo(hwnd));
                return;
            }

            if (objectId == OBJID_WINDOW && childId == CHILDID_SELF && hwnd != nullptr && GetAncestor(hwnd, GA_ROOT) == hwnd && (event == EVENT_OBJECT_SHOW || event == EVENT_OBJECT_UNCLOAKED)) {
                auto window = WindowInfo(hwnd);

                if (IsCapturableWindow(window)) {
                    WindowListForThread->AddWindow(window);
                }
            }
        },
        0, 0, WINEVENT_OUTOFCONTEXT);

    self->forewnd_change_hook_ = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr,
        [](HWINEVENTHOOK eventHook, DWORD event, HWND hwnd, LONG objectId, LONG childId, DWORD eventThreadId, DWORD eventTimeInMilliseconds) {
            if (objectId == OBJID_WINDOW && childId == CHILDID_SELF && hwnd != nullptr && GetAncestor(hwnd, GA_ROOT) == hwnd) {
                WindowListForThread->SetWindowsChanged();
            }
        },
        0, 0, WINEVENT_OUTOFCONTEXT);

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, self->msg_wnd_, 0, 0)) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WindowListForThread = nullptr;
    UnregisterClass(kClassName, wcex.hInstance);
    SetEvent(self->event_quit_);

    return 0;
}

void WindowList::AddWindow(WindowInfo const& info)
{
    std::lock_guard<std::mutex> lock(wnd_list_mutex_);

    auto search = visible_wnds_.find(info.WindowHandle);
    if (search == visible_wnds_.end()) {
        changed_ = true;
        windows_.push_back(info);
        visible_wnds_.insert(info.WindowHandle);
    }
}

bool WindowList::RemoveWindow(WindowInfo const& info)
{
    std::lock_guard<std::mutex> lock(wnd_list_mutex_);

    auto search = visible_wnds_.find(info.WindowHandle);
    if (search != visible_wnds_.end()) {
        visible_wnds_.erase(search);
        auto index = 0;
        for (auto& window : windows_) {
            if (window.WindowHandle == info.WindowHandle) {
                changed_ = true;
                break;
            }
            index++;
        }
        windows_.erase(windows_.begin() + index);
        return true;
    }
    return false;
}

void WindowList::SetWindowsChanged()
{
    changed_ = true;
}
}
#endif // defined WEBRTC_DISABLE_VIDEO