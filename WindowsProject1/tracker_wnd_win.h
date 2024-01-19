#pragma once
#include <Windows.h>
#include <mutex>
#include <wingdi.h>
#include <shellscalingapi.h>

#include <vector>

namespace webrtc {

/*
    Tracker Window for Desktop capturer.
    Class features:
    1. Thread-safe
    2. Two kind of thread: UI thread or non-UI thread.
    3. Target window that is DPI-aware or not.
    4. Window event transparent
    5. Try best effet to make the tracking window size and pos right.
      And currently, Netease Meeting and popo meeting is DPI aware.
    6. Most Important: tracking specified window or monitor.
    7. Tracking window type: 
      GUI app window. (Using Win Event Hook)
      Console application. (Hook is not help, by win32 WM_TIMER)

    Precondition:
    1. Host process HAS TO BE DPI-aware, if not, 
      tracker window will takes an incorrect size and pos,
      i.e. GetWindowScaledRect function does not work.
      If Host process is NOT DPI-aware, TrackerWnd refuse to work.

    Sample Code:

    // Track screen contain tracked_wnd
    TrackerWnd::Options g_options;
    g_options.type = TrackerWnd::TrackSourceType::kScreen;
    g_options.tracked_wnd = (HWND)0x002D0962;
    g_options.hight_light_clr = RGB(0, 222, 126);
    g_options.thickness = 6;
    // start to track when TrackerWnd created.
    g_options.start_on_create = true;
    tracker_wnd = TrackerWnd::create(g_options);

    // Track screen specified
    TrackerWnd::Options g_options;
    g_options.type = TrackerWnd::TrackSourceType::kScreen;
    g_options.monitor = (HMONITOR)0x1f150d9f;
    g_options.hight_light_clr = RGB(0, 222, 126);
    g_options.thickness = 6;
    g_options.start_on_create = true;
    tracker_wnd = TrackerWnd::create(g_options);

    // Track window specified
    TrackerWnd::Options g_options;
    g_options.type = TrackerWnd::TrackSourceType::kWindow;
    g_options.tracked_wnd = (HWND)0x002D0962;
    g_options.hight_light_clr = RGB(0, 222, 126);
    g_options.thickness = 6;
    g_options.start_on_create = true;
    tracker_wnd = TrackerWnd::create(g_options);
 */

class TrackerWnd : public std::enable_shared_from_this<TrackerWnd> {
public:
    using TrackerWndPtr = std::shared_ptr<TrackerWnd>;

    enum class TrackSourceType {
        kWindow,
        kScreen
    };

    struct Options {
        TrackSourceType type = TrackSourceType::kWindow;
        HWND tracked_wnd = NULL;
        HMONITOR monitor = NULL;

        int thickness = 5;
        COLORREF hight_light_clr = RGB(255, 128, 0);
        bool start_on_create = true;
    };


protected:
  // Use TrackerWnd::create to get heap object.
  // Disable stack object.
  explicit TrackerWnd(Options options);

public:
  ~TrackerWnd();

  std::shared_ptr<TrackerWnd> getptr() {
    return shared_from_this();
  }

  static std::shared_ptr<TrackerWnd> create(Options options)
  {
    return std::shared_ptr<TrackerWnd>(new TrackerWnd(options));
  }

    void start();
    void stop();

private:
    RECT GetWindowScaledRect(HWND wnd, int cx, int cy);

    void InstallEventHook();
    void UninstallEventHook();
    void InstallTimerForConsole(HWND hwnd);
    void UninstallTimerForConsole(HWND hwnd);
    void UpdateSizeAndPos();
    void UpdateZOrder();
    void ShowTrackerWindow();
    void OnClearResource();

    void OnTrackerWindowCreate(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void OnTrackerMonitor(HWND hwnd, LPARAM lp);
    void OnPaint(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void OnTimerConsoleSizeDetect(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);


private:
    std::recursive_mutex track_wnd_mutex_;
    bool  tracking_ = false;
    HANDLE event_quit_ = NULL;
    HANDLE worker_gui_thread_ = NULL;

    TrackerWnd::Options options_;

    const COLORREF COLOR_KEY = RGB(255, 129, 0);
    HPEN   pen_ = NULL;
    HBRUSH brush_ = NULL;

    HWND tracker_wnd_ = NULL;
    DWORD thread_id_tracked_wnd_ = 0;
    DWORD process_id_tracked_wnd_ = 0;
    bool enable_location_check_ = false;

    // Normal GUI window tracking stuff
    HWINEVENTHOOK win_event_hook_ = NULL;

    // Console window tracking stuff
    struct ConsoleWndTrackInfo {
      GUITHREADINFO gui_thread_info_ = { sizeof(GUITHREADINFO) };
      RECT rc_console_ = { 0 };
      bool console_hide_ = false;
      int console_moving_count_ = 10;
    } console_track_info_;

    static bool class_registered_;

    friend void WinEventHookProc(
      HWINEVENTHOOK hWinEventHook,
      DWORD event,
      HWND hwnd,
      LONG idObject,
      LONG idChild,
      DWORD idEventThread,
      DWORD dwmsEventTime);
    friend LRESULT CALLBACK TrackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    friend DWORD WINAPI WorkThreadProc(void* pv);
};


class SetThreadDpiAwareContextWrapper {
public:
    SetThreadDpiAwareContextWrapper()
    {
        // ref:
        // https://learn.microsoft.com/en-us/windows/win32/hidpi/dpi-awareness-context
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setthreaddpiawarenesscontext
        HMODULE module = LoadLibrary(L"User32.dll");
        if (module) {
            setThreadDpiAwarenessContext = reinterpret_cast<SETTHREADDPIAWARENESSCONTEXT>(
                GetProcAddress(module, "SetThreadDpiAwarenessContext"));
        }

        if (setThreadDpiAwarenessContext) {
            // Or DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2?
            setThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
        }

        if (module) {
            FreeLibrary(module);
            module = NULL;
        }
    }
    ~SetThreadDpiAwareContextWrapper()
    {
        setThreadDpiAwarenessContext = nullptr;
    }

private:
    // SetThreadDpiAwarenessContext  proto type.
    using SETTHREADDPIAWARENESSCONTEXT = DPI_AWARENESS_CONTEXT(WINAPI*)(DPI_AWARENESS_CONTEXT);
    SETTHREADDPIAWARENESSCONTEXT setThreadDpiAwarenessContext = nullptr;
};

class ProcessDPIChecker {
public:
    ProcessDPIChecker() {
        HMODULE shcore = LoadLibrary(L"Shcore.dll");
        if (shcore) {
            getProcessDpiAwareness = reinterpret_cast<GETPROCESSDPIAWARENESS>(GetProcAddress(shcore, "GetProcessDpiAwareness"));
        }
        // Above Window 8.1
        if (getProcessDpiAwareness) {
            PROCESS_DPI_AWARENESS pda = PROCESS_DPI_UNAWARE;
            auto hr = getProcessDpiAwareness(NULL, &pda);
            if (hr == S_OK) {
                dpi_aware_ = pda != PROCESS_DPI_UNAWARE;
            }
        }
        else {
          // Above Vista OS
            dpi_aware_ = ::IsProcessDPIAware();
        }

        if (shcore) {
            FreeLibrary(shcore);
            shcore = NULL;
        }
    }
    ~ProcessDPIChecker() {
      getProcessDpiAwareness = nullptr;
    }

    bool isDpiAware() {  return dpi_aware_; }

private:
    bool dpi_aware_ = false;

    using GETPROCESSDPIAWARENESS = HRESULT(WINAPI*)(HANDLE, PROCESS_DPI_AWARENESS*);
    GETPROCESSDPIAWARENESS getProcessDpiAwareness = nullptr;
};

}