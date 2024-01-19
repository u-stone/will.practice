#if !defined WEBRTC_DISABLE_VIDEO
#pragma once
#include <unordered_set>
#include <Windows.h>
#include <mutex>

namespace lava {

    struct WindowInfo
    {
        WindowInfo(HWND windowHandle)
        {
            WindowHandle = windowHandle;
            auto classNameLength = 256;
            std::wstring className(classNameLength, 0);
            GetClassNameW(WindowHandle, const_cast<LPTSTR>(className.data()), classNameLength);
            ClassName = className;

            className.assign(classNameLength, 0);
            GetWindowTextW(WindowHandle, const_cast<LPTSTR>(className.data()), classNameLength);

            GetWindowRect(WindowHandle, &rc);
        }

        HWND WindowHandle;
        std::wstring ClassName;
        std::wstring title;
        RECT rc;

        bool operator==(const WindowInfo& info) { return WindowHandle == info.WindowHandle; }
        bool operator!=(const WindowInfo& info) { return !(*this == info); }
    };

    class WindowList
    {
    public:
        WindowList();
        ~WindowList();

        const std::vector<WindowInfo> GetCurrentWindows();
        bool WindowsChangedSinceLastGet();

        void SetCurrentCapturedWnd(HWND wnd);

        static LRESULT CALLBACK WindowProc(
            _In_ HWND   hwnd,
            _In_ UINT   uMsg,
            _In_ WPARAM wParam,
            _In_ LPARAM lParam
        );
        static DWORD WINAPI WorkProc(LPVOID pv);

    private:
        void AddWindow(WindowInfo const& info);
        bool RemoveWindow(WindowInfo const& info);
        void SetWindowsChanged();

    private:
        HWND                     msg_wnd_ = NULL;
        HANDLE                   work_thread_ = NULL;
        HANDLE                   event_quit_ = NULL;
        bool                     changed_ = false;
        std::vector<WindowInfo>  windows_;
        std::unordered_set<HWND> visible_wnds_;
        HWINEVENTHOOK            wnd_hook_ = NULL;
        HWINEVENTHOOK            forewnd_change_hook_ = NULL;
        std::mutex               wnd_list_mutex_;
    };

}
#endif  // defined WEBRTC_DISABLE_VIDEO