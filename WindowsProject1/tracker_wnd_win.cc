#include "tracker_wnd_win.h"
#include <dwmapi.h>
#include <oleacc.h>
#include <algorithm>
#include <cassert>
#include <array>

#if 0
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Oleacc.lib")
#endif

namespace webrtc {

#define USE_DWM

const auto kClassNameTrackingWnd = L"TrackerWnd";
const int kTimerElapsedMS = 20;
const int kMaxTimesMovingDelay = 10;

thread_local std::shared_ptr<TrackerWnd> tls_tracker_wnd;

#define ID_TIMER_CONSOLE 0x0DAD

void WinEventHookProc(
  HWINEVENTHOOK hWinEventHook,
  DWORD event,
  HWND hwnd,
  LONG idObject,
  LONG idChild,
  DWORD idEventThread,
  DWORD dwmsEventTime);
LRESULT CALLBACK TrackWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI WorkThreadProc(void* pv);


bool TrackerWnd::class_registered_ = false;


// TrackerWnd member functions
TrackerWnd::TrackerWnd(TrackerWnd::Options opt)
{
    std::lock_guard<std::recursive_mutex> lock(track_wnd_mutex_);

    options_ = opt;

    if (opt.start_on_create) {
        start();
    }
}

TrackerWnd::~TrackerWnd()
{
    std::lock_guard<std::recursive_mutex> lock(track_wnd_mutex_);

    stop();
}

void TrackerWnd::start()
{
    ProcessDPIChecker dpi_checker;
    if (!dpi_checker.isDpiAware()) {
        // Log to file.
        return;
    }

    std::lock_guard<std ::recursive_mutex> lock(track_wnd_mutex_);

    if (tracking_) {
        return;
    }

    tracking_ = true;

    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, FALSE };
    if (event_quit_ == NULL) {
        event_quit_ = CreateEvent(&sa, TRUE, FALSE, NULL);
    }

#if 0
    // TODO: Make a GUI thread check, to detemine whether create new thread.
  if (::IsGUIThread(FALSE)) {
  }
#endif

    // Running in a seperated GUI thread.
    if (worker_gui_thread_ == NULL) {
        worker_gui_thread_ = CreateThread(&sa, 0, WorkThreadProc, this, 0, nullptr);
        CloseHandle(worker_gui_thread_);
        worker_gui_thread_ = NULL;
    }

    throw 2;
}

void TrackerWnd::stop()
{
    std::lock_guard<std::recursive_mutex> lock(track_wnd_mutex_);

    if (!tracking_) {
        return;
    }

    // Quit the tracking window message queue.
    // i.e. Make GetMessage return 0 to quit.
    PostMessage(tracker_wnd_, WM_QUIT, 0, 0);

    if (event_quit_) {
        // Sync with tracking window message queue.
        WaitForSingleObject(event_quit_, INFINITE);
        CloseHandle(event_quit_);
        event_quit_ = NULL;
    }

    tracking_ = false;
}

LRESULT CALLBACK TrackWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        auto self = reinterpret_cast<TrackerWnd*>(cs->lpCreateParams);
        tls_tracker_wnd = self->getptr();
        tls_tracker_wnd->tracker_wnd_ = hwnd;
        tls_tracker_wnd->OnTrackerWindowCreate(hwnd, msg, wp, 0);
        tls_tracker_wnd->OnTrackerMonitor(hwnd, 0);
    } break;
    case WM_PAINT: {
        tls_tracker_wnd->OnPaint(hwnd, msg, wp, lp);
    } break;
    case WM_TIMER: {
        tls_tracker_wnd->OnTimerConsoleSizeDetect(hwnd, msg, wp, lp);
    } break;
    case WM_DISPLAYCHANGE: {
        tls_tracker_wnd->enable_location_check_ = true;
    } break;
    case WM_DPICHANGED: {
        tls_tracker_wnd->enable_location_check_ = true;
    } break;

    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }

    return 0;
}

DWORD WINAPI WorkThreadProc(void* arg)
{
    SetThreadDpiAwareContextWrapper dpi_guard;

    auto self = static_cast<TrackerWnd*>(arg);

    assert(("self should not be nullptr.", self));

    auto module = GetModuleHandle(NULL);
    if (!TrackerWnd::class_registered_) {
        TrackerWnd::class_registered_ = true;

        WNDCLASSW wc{ sizeof(wc) };
        wc.hInstance = module;
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        wc.lpszClassName = kClassNameTrackingWnd;
        wc.lpfnWndProc = &TrackWndProc;
        RegisterClassW(&wc);
    }

    // WS_EX_NOACTIVATE make tracker window disappear on task bar.
    DWORD ex_style = WS_EX_LAYERED | WS_EX_TRANSPARENT;// | WS_EX_NOACTIVATE;
    if (self->options_.type == TrackerWnd::TrackSourceType::kScreen) {
        ex_style |= WS_EX_TOPMOST;
    }

    self->tracker_wnd_ = CreateWindowExW(ex_style, kClassNameTrackingWnd, L"", WS_POPUP,
        0, 0, 0, 0, nullptr, nullptr, module, arg);
    if (self->tracker_wnd_ == NULL) {
      auto last_error = ::GetLastError();
      printf("Fatal error, Create tracker window failed: %d", last_error);
      return 1;
    }

    // In case of resulting totally transparent window.
    if (self->COLOR_KEY == self->options_.hight_light_clr) {
        *const_cast<DWORD*>(&self->COLOR_KEY) += 1;
    }

    SetLayeredWindowAttributes(self->tracker_wnd_, self->COLOR_KEY, (self->options_.hight_light_clr >> 24 & 0xFF), LWA_COLORKEY | LWA_ALPHA);

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, self->tracker_wnd_, 0, 0)) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    self->OnClearResource();

    SetEvent(self->event_quit_);

    return 0;
}

RECT TrackerWnd::GetWindowScaledRect(HWND wnd, int cx, int cy)
{
    RECT rect = {};
    DwmGetWindowAttribute(options_.tracked_wnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(rect));

    // Extend rect than trcke
    ::InflateRect(&rect, cx, cy);

    return rect;
}

void TrackerWnd::InstallEventHook()
{
    if (win_event_hook_ == nullptr) {
        // Focus on the tracked window process and thread.
      // WinEventHookProc runs on current thread.
        win_event_hook_ = SetWinEventHook(
          EVENT_MIN, 
          EVENT_MAX, 
          nullptr, 
          reinterpret_cast<WINEVENTPROC>(WinEventHookProc), 
          process_id_tracked_wnd_, 
          thread_id_tracked_wnd_, 
          WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNTHREAD);
    }
}

void TrackerWnd::UninstallEventHook()
{
    if (win_event_hook_) {
        UnhookWinEvent(win_event_hook_);
        win_event_hook_ = NULL;
    }
}

void TrackerWnd::InstallTimerForConsole(HWND hwnd) {
  // Reset console rect.
  console_track_info_.rc_console_ = { 0,0,0,0 };
  SetTimer(hwnd, ID_TIMER_CONSOLE, kTimerElapsedMS, NULL);
}

void TrackerWnd::UninstallTimerForConsole(HWND hwnd) {
  KillTimer(hwnd, ID_TIMER_CONSOLE);
}

void TrackerWnd::UpdateSizeAndPos()
{
    RECT rc = GetWindowScaledRect(options_.tracked_wnd, options_.thickness, options_.thickness);
    if (::IsRectEmpty(&rc)) {
        return;
    }

    /// Setting position/size.
    SetWindowPos(tracker_wnd_, 0, rc.left, rc.top, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
}

void TrackerWnd::UpdateZOrder()
{
    /// and Setting z-order.
    ::SetWindowPos(tracker_wnd_, options_.tracked_wnd, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void TrackerWnd::ShowTrackerWindow()
{
    if (::IsZoomed(options_.tracked_wnd) || !::IsWindowVisible(options_.tracked_wnd)) {
        ShowWindow(tracker_wnd_, SW_HIDE);
        return;
    }
    ShowWindow(tracker_wnd_, SW_SHOWNOACTIVATE);
}

void TrackerWnd::OnTrackerWindowCreate(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (options_.type != TrackerWnd::TrackSourceType::kWindow || options_.tracked_wnd == NULL) {
        return;
    }

    WINDOWPLACEMENT windowPlacement = {
        sizeof(WINDOWPLACEMENT),
    };

    if (!::GetWindowPlacement(options_.tracked_wnd, &windowPlacement)) {
        return;
    }

    if (windowPlacement.showCmd == SW_SHOWMINIMIZED
        || !IsWindowVisible(options_.tracked_wnd)) {
        // Minimized or Hide.
        ShowWindow(hwnd, SW_HIDE);
    } else if (::IsZoomed(options_.tracked_wnd)) {
        // Maxmized
        ShowWindow(hwnd, SW_HIDE);
    } else {
        // Normal
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        UpdateSizeAndPos();
        UpdateZOrder();
        //ShowTrackerWindow();
    }
    
    thread_id_tracked_wnd_ = GetWindowThreadProcessId(options_.tracked_wnd, &process_id_tracked_wnd_);

    constexpr int kMaxClassNameLenth = 256;
    CHAR className[kMaxClassNameLenth] = { 0 };
    GetClassNameA(options_.tracked_wnd, className, kMaxClassNameLenth);
    std::array<std::string, 2> console_list {
      "ConsoleWindowClass", // cmd.exe, msvc console, powershell
      "mintty"
    };
    auto iter = std::find(std::begin(console_list), std::end(console_list), className);
    if (iter != std::end(console_list)) {
      InstallTimerForConsole(hwnd);
    }
    else {
      InstallEventHook();
    }
}

void TrackerWnd::OnTrackerMonitor(HWND hwnd, LPARAM lp)
{
  if (options_.type == TrackerWnd::TrackSourceType::kScreen && options_.tracked_wnd && options_.monitor == NULL) {
    options_.monitor = ::MonitorFromWindow(options_.tracked_wnd, MONITOR_DEFAULTTONEAREST);
    }

    if (options_.monitor == nullptr) {
      return;
    }

    MONITORINFO mi;
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(options_.monitor, &mi);

    RECT& rc = mi.rcMonitor;

    /// Setting position/size.
    SetWindowPos(hwnd, 0, rc.left, rc.top, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

    /// and Setting z-order.
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
}

void WinEventHookProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD event,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD idEventThread,
    DWORD dwmsEventTime)
{
    if (tls_tracker_wnd->win_event_hook_ != hWinEventHook || hwnd != tls_tracker_wnd->options_.tracked_wnd) {
        return;
    }
#if 0
    std::wstring name;
    bool is_normal_state = false;
    IAccessible* pAcc = NULL;
    VARIANT varChild;
    HRESULT hr = AccessibleObjectFromEvent(hwnd, idObject, idChild, &pAcc, &varChild);
    if ((hr == S_OK) && (pAcc != NULL))
    {
      BSTR bstrName;
      hr = pAcc->get_accName(varChild, &bstrName);
      if (hr == S_OK) {
        name = bstrName;
        SysFreeString(bstrName);
      }
      //state = GetAccState(pAcc, varChild);

      LONG stateBits = 0;
      VARIANT varResult;
      hr = pAcc->get_accState(varChild, &varResult);
      if ((hr == S_OK) && (varResult.vt == VT_I4))
      {
        stateBits = (LONG)varResult.lVal;
        is_normal_state = stateBits == 0;
      }

      pAcc->Release();
    }
#endif
    // Constant value ref:
    // https://learn.microsoft.com/en-us/windows/win32/winauto/event-constants
    switch (event) {
        // EVENT_OBJECT_
    case EVENT_OBJECT_DESTROY: {
        ShowWindow(tls_tracker_wnd->tracker_wnd_, SW_HIDE);
    } break;
    case EVENT_OBJECT_SHOW: {
      tls_tracker_wnd->UpdateSizeAndPos();
      tls_tracker_wnd->UpdateZOrder();
      tls_tracker_wnd->ShowTrackerWindow();
    } break;
    case EVENT_OBJECT_HIDE: {
        ShowWindow(tls_tracker_wnd->tracker_wnd_, SW_HIDE);
    } break;
    case EVENT_OBJECT_REORDER: {
        //UpdateSizeAndPos(g_hTrackerWnd);
      tls_tracker_wnd->UpdateZOrder();
        //ShowTrackerWindow();
    } break;
    case EVENT_OBJECT_FOCUS: {
        //UpdateSizeAndPos(g_hTrackerWnd);
      tls_tracker_wnd->UpdateZOrder();
      tls_tracker_wnd->ShowTrackerWindow();
    } break;
    case EVENT_OBJECT_STATECHANGE: {
        // Click Min, Max, Restore Button on title bar.
        // And EVENT_OBJECT_LOCATIONCHANGE will be invoked.
      tls_tracker_wnd->enable_location_check_ = true;
        //ShowTrackerWindow();
    } break;
    case EVENT_OBJECT_LOCATIONCHANGE: {
        if (tls_tracker_wnd->enable_location_check_) {
            if (::IsZoomed(tls_tracker_wnd->options_.tracked_wnd) || !::IsWindowVisible(tls_tracker_wnd->options_.tracked_wnd)) {
                ShowWindow(tls_tracker_wnd->tracker_wnd_, SW_HIDE);
                return;
            } else {
              tls_tracker_wnd->UpdateSizeAndPos();
              tls_tracker_wnd->UpdateZOrder();
              tls_tracker_wnd->ShowTrackerWindow();
            }
            tls_tracker_wnd->enable_location_check_ = false;
        }
    } break;
        // EVENT_SYSTEM_
    case EVENT_SYSTEM_FOREGROUND: {
        // Activate window
        //if (::IsWindowVisible(g_hTrackerWnd)) {
        //    UpdateZOrder(g_hTrackerWnd);
        //} else {
        //    ShowWindow(g_hTrackerWnd, SW_HIDE);
        //}
    } break;
    case EVENT_SYSTEM_MINIMIZESTART: {
        // Really minimized the window
        ShowWindow(tls_tracker_wnd->tracker_wnd_, SW_HIDE);
    } break;
    case EVENT_SYSTEM_MINIMIZEEND: {
      tls_tracker_wnd->UpdateSizeAndPos();
      tls_tracker_wnd->UpdateZOrder();
      tls_tracker_wnd->ShowTrackerWindow();
    } break;
    case EVENT_SYSTEM_MOVESIZESTART: {
        // Hide tracker window
        ShowWindow(tls_tracker_wnd->tracker_wnd_, SW_HIDE);
        // Disable location event response
        tls_tracker_wnd->enable_location_check_ = false;
    } break;
    case EVENT_SYSTEM_MOVESIZEEND: {
      tls_tracker_wnd->UpdateSizeAndPos();
      tls_tracker_wnd->UpdateZOrder();
      tls_tracker_wnd->ShowTrackerWindow();
    } break;
    case EVENT_SYSTEM_CAPTURESTART: {
        //ShowWindow(g_hTrackerWnd, SW_HIDE);
      tls_tracker_wnd->enable_location_check_ = false;
    } break;
    case EVENT_SYSTEM_CAPTUREEND: {
        // Double-click title bar to make window maximized or restore.
      tls_tracker_wnd->enable_location_check_ = true;
    } break;
    default:
        break;
    }
}

void TrackerWnd::OnPaint(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc{};
    GetClientRect(hwnd, &rc);

    // Rectangle funtion will paint center line as setting coordinate.
    // So here rect reduce half-thickness size.
    ::InflateRect(&rc, -options_.thickness / 2, -options_.thickness / 2);
    if (pen_ == NULL) {
        LOGBRUSH lb;
        lb.lbStyle = BS_SOLID;
        lb.lbColor = options_.hight_light_clr & 0x00FFFFFF;
        lb.lbHatch = 0;
        pen_ = ExtCreatePen(PS_GEOMETRIC | PS_INSIDEFRAME | PS_ENDCAP_SQUARE | PS_JOIN_BEVEL, options_.thickness, &lb, 0, NULL);
    }
    if (brush_ == NULL) {
      brush_ = CreateSolidBrush(COLOR_KEY);
    }
    HGDIOBJ hOldPen = SelectObject(hdc, pen_);
    HGDIOBJ hOldBrush = SelectObject(hdc, brush_);

    //Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    ::InflateRect(&rc, options_.thickness, options_.thickness);
    ::FillRect(hdc, &rc, brush_);
    ::InflateRect(&rc, -options_.thickness, -options_.thickness);

    const int kLength = 120;

    ::MoveToEx(hdc, rc.left, rc.top + kLength, NULL);
    ::LineTo(hdc, rc.left, rc.top);
    ::LineTo(hdc, rc.left + kLength, rc.top);

    ::MoveToEx(hdc, rc.right - kLength, rc.top, NULL);
    ::LineTo(hdc, rc.right, rc.top);
    ::LineTo(hdc, rc.right, rc.top + kLength);

    ::MoveToEx(hdc, rc.left, rc.bottom - kLength, NULL);
    ::LineTo(hdc, rc.left, rc.bottom);
    ::LineTo(hdc, rc.left + kLength, rc.bottom);

    ::MoveToEx(hdc, rc.right - kLength, rc.bottom, NULL);
    ::LineTo(hdc, rc.right, rc.bottom);
    ::LineTo(hdc, rc.right, rc.bottom - kLength);
    

    if (hOldPen)
        SelectObject(hdc, hOldPen);
    if (hOldBrush)
        SelectObject(hdc, hOldBrush);

    EndPaint(hwnd, &ps);
}

void TrackerWnd::OnTimerConsoleSizeDetect(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  RECT rc = GetWindowScaledRect(options_.tracked_wnd, options_.thickness, options_.thickness);
  if (::IsRectEmpty(&rc)) {
    ShowWindow(hwnd, SW_HIDE);
    return;
  }

  if (::EqualRect(&rc, &console_track_info_.rc_console_)) {
    // Moving stopped.
    if (--console_track_info_.console_moving_count_ > 0) {
      return;
    }
    if (console_track_info_.console_hide_) {
      console_track_info_.console_hide_ = false;
      //ShowWindow(hwnd, SW_SHOWNOACTIVATE);
      /// Setting position/size.
      SetWindowPos(hwnd, 0, rc.left, rc.top, rc.right - rc.left,
        rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

      UpdateZOrder();
      ShowTrackerWindow();
    }

    {
        // Check tracking window gui thread info changed
        GUITHREADINFO gti = { sizeof(GUITHREADINFO) };
        if (!GetGUIThreadInfo(thread_id_tracked_wnd_, &gti)) {
          return;
        }

        // Check if gui thread info changed.
        // Skip hwndMoveSize (the window in a move or size loop.) compare.
        auto gui_thread_info = console_track_info_.gui_thread_info_;
        bool gui_changed =
          (gui_thread_info.hwndActive != gti.hwndActive) ||   // active window
          (gui_thread_info.hwndFocus != gti.hwndFocus) ||     // has keyboard focus
          (gui_thread_info.hwndCapture != gti.hwndCapture) || // capture the mouse
          (gui_thread_info.hwndCaret != gti.hwndCaret) ||     // window that owns any active menus
          (gui_thread_info.hwndCaret != gti.hwndCaret);       // window to display caret

        if (!gui_changed) {
          return;
        }

        // Check if tracked window gui thread info changed.
        HWND& tracked_wnd = options_.tracked_wnd;
        bool console_z_order_changed =
          (gui_thread_info.hwndActive  != gti.hwndActive  && (gui_thread_info.hwndActive == tracked_wnd  || gti.hwndActive == tracked_wnd))  || // active window
          (gui_thread_info.hwndFocus   != gti.hwndFocus   && (gui_thread_info.hwndFocus == tracked_wnd   || gti.hwndFocus == tracked_wnd))   || // has keyboard focus
          (gui_thread_info.hwndCapture != gti.hwndCapture && (gui_thread_info.hwndCapture == tracked_wnd || gti.hwndCapture == tracked_wnd)) || // capture the mouse
          (gui_thread_info.hwndCaret   != gti.hwndCaret   && (gui_thread_info.hwndCaret == tracked_wnd   || gti.hwndCaret == tracked_wnd))   || // window that owns any active menus
          (gui_thread_info.hwndCaret   != gti.hwndCaret   && (gui_thread_info.hwndCaret == tracked_wnd   || gti.hwndCaret == tracked_wnd));     // window to display caret
        if (console_z_order_changed) {
          UpdateZOrder();
          ShowTrackerWindow();
          ::InvalidateRect(tracker_wnd_, NULL, FALSE);
          console_track_info_.gui_thread_info_ = gti;
        }
    }
  }
  else {
    // Moving
    if (!console_track_info_.console_hide_ && !::IsRectEmpty(&console_track_info_.rc_console_)) {
      ShowWindow(hwnd, SW_HIDE);
      console_track_info_.console_hide_ = true;
    }
    console_track_info_.console_moving_count_ = kMaxTimesMovingDelay;
    console_track_info_.rc_console_ = rc;
  }
}

void TrackerWnd::OnClearResource()
{
    if (pen_) {
        DeleteObject(pen_);
        pen_ = NULL;
    }
    if (brush_) {
        DeleteObject(brush_);
        brush_ = NULL;
    }

    UninstallEventHook();

    UninstallTimerForConsole(tracker_wnd_);
}


} // namespace webrtc