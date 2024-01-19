// window-list.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "WindowListHelper.h"
#include <dwmapi.h>
#include <psapi.h>
#include <sstream>
#include <cctype>
#include <shlwapi.h>
#include <regex>

void findwnd();
void clear_screen(char fill = ' ');

typedef struct {
  DWORD ownerpid;
  DWORD childpid;
} windowinfo;
BOOL CALLBACK EnumChildWindowsCallback(HWND hWnd, LPARAM lp) {
  windowinfo* info = (windowinfo*)lp;
  DWORD pid = 0;
  GetWindowThreadProcessId(hWnd, &pid);
  if (pid != info->ownerpid) info->childpid = pid;
  return TRUE;
}
static std::wstring getImageFilePath(HWND window) {
  wchar_t filename[MAX_PATH] = { 0 };
  HANDLE process = NULL;

  windowinfo info = { 0 };
  GetWindowThreadProcessId(window, &info.ownerpid);
  info.childpid = info.ownerpid;
  EnumChildWindows(window, EnumChildWindowsCallback, (LPARAM)&info);
  process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
    info.childpid);
  DWORD bufsize = MAX_PATH;

  std::wstring ret = L"";
  if (process != NULL) {
    if (QueryFullProcessImageName(process, 0, filename, &bufsize) > 0) {
      ret = filename;
    }
    if (ret.find(L"rundll32.exe") != std::wstring::npos) {
      std::cout << "find rundll32.exe, try GetProcessImageFileName" << std::endl;
      if (GetProcessImageFileName(process, filename, bufsize) > 0) {
        ret = filename;
        std::wcout << "GetProcessImageFileName get " << filename << std::endl;
      }

      if (ret.find(L"rundll32.exe") != std::wstring::npos) {
        std::cout << "find rundll32.exe again" << std::endl;
        if (GetWindowText(window, filename, bufsize)) {
          ret = filename;
          std::wcout << "get window text: " << filename << std::endl;
          if (ret.find(L"- PowerPoint") != std::wstring::npos) {
            // set a fake path.
            ret = L"C:\\Program Files (x86)\\Microsoft Office\\root\\Office16\\POWERPNT.EXE";
          }
        }
      }
    }
    CloseHandle(process);
  }

  return ret;
}
#if 0
#include <uiautomationclient.h>
//#import "UIAutomationCore.dll"
using namespace UIAutomationClient;
int main()
{
  // initialize COM, needed for UIA
  CoInitialize(NULL);


  // initialize main UIA class
  IUIAutomationPtr pUIA(__uuidof(CUIAutomation));

  do
  {
    // get the Automation element for the foreground window
    IUIAutomationElementPtr foregroundWindow = pUIA->ElementFromHandle(GetForegroundWindow());
    wprintf(L"pid:%i\n", foregroundWindow->CurrentProcessId);

    // prepare a [class name = 'Windows.UI.Core.CoreWindow'] condition
    _variant_t prop = L"Windows.UI.Core.CoreWindow";
    IUIAutomationConditionPtr condition = pUIA->CreatePropertyCondition(UIA_ClassNamePropertyId, prop);

    // get the first element (window hopefully) that satisfies this condition
    IUIAutomationElementPtr coreWindow = foregroundWindow->FindFirst(TreeScope::TreeScope_Children, condition);
    if (coreWindow)
    {
      // get the process id property for that window
      wprintf(L"store pid:%i\n", coreWindow->CurrentProcessId);
    }

    Sleep(1000);
  } while (TRUE);

cleanup:
  CoUninitialize();
  return 0;
}
#endif

#if 1
int main()
{

  std::wstring path = getImageFilePath((HWND)0xD130E);
  std::wcout << "get: " << path.c_str() << std::endl;

  DWORD tid = 0;
  DWORD PID = GetWindowThreadProcessId((HWND)0xD130E, &tid);
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
      FALSE, PID);
  if (NULL != hProcess) {
      std::cout << "hProcess " << hProcess << "\n"
                << std::endl;
      TCHAR nameProc[1024] = { 0 };
      if (GetProcessImageFileName(hProcess, nameProc, sizeof(nameProc) / sizeof(*nameProc)) == 0) {
          std::cout << "GetProcessImageFileName Error" << std::endl;
      } else {
          std::wcout << "nameProcess:" << nameProc << std::endl;
      }
  }
  


  std::cin.ignore();
  std::cin.get();
  return 0;

  std::string str1 = u8"\u6709\u9053\u4E91\u7B14\u8BB0.exe";
  std::string str2 = "\u6709\u9053\u4E91\u7B14\u8BB0.exe";
  std::string str3 = u8"有道云笔记.exe";
  std::string str4 = "有道云笔记.exe";

  std::wstring utf16_str = L"有道云笔记.exe";
  int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
    utf16_str.length(), nullptr, 0,
    nullptr, nullptr);
  std::string utf8_str(utf8_size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
    utf16_str.length(), &utf8_str[0], utf8_size,
    nullptr, nullptr);

  std::cout << str1.c_str() << std::endl;
  std::cout << str2.c_str() << std::endl;
  std::cout << str3.c_str() << std::endl;
  std::cout << utf8_str.c_str() << std::endl;
  std::cout << str4.c_str() << std::endl;

    auto get = []() {
        HWND desktop = GetDesktopWindow();
        HWND startWnd = FindWindowExA(desktop, NULL, NULL, "Start");
        auto classNameLength = 256;
        std::string className(classNameLength, 0);
        GetClassNameA(startWnd, const_cast<LPSTR>(className.data()),
            classNameLength);
        std::cout << startWnd << ", " << className.c_str() << std::endl;
    };

    //lava::WindowList* wlist = new lava::WindowList();
    //auto wnds = wlist->GetCurrentWindows();
    //HWND sh = GetShellWindow();
    while (1) {
      get();
        //auto wnd = wlist->GetCurrentWindows();
        //if (wnd.size() != wnds.size()) {
        //    wnds.swap(wnd);
        //}
        //int index = 0;
        //for (auto item : wnds) {
        //    std::wcout << ++index << " : " << item.ClassName.c_str() << ", " << item.title.c_str() << std::endl;
        //}
        //findwnd();
        Sleep(2000);
    }
    std::cout << "Hello World!\n";
}
#endif


void clear_screen(char fill)
{
    COORD tl = { 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO s;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(console, &s);
    DWORD written, cells = s.dwSize.X * s.dwSize.Y;
    FillConsoleOutputCharacter(console, fill, cells, tl, &written);
    FillConsoleOutputAttribute(console, s.wAttributes, cells, tl, &written);
    SetConsoleCursorPosition(console, tl);
}

void findwnd()
{
    //clear_screen(' ');
    system("cls");
    HWND hwnd = GetDesktopWindow();
    HWND srcWnd = hwnd;
    int index = 0;
    hwnd = GetNextWindow(hwnd, GW_CHILD);
    WCHAR title[256] = { 0 };
    //GetWindowText((HWND)(0x10240), title, 256);

    while (hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) {
        ////过滤了非窗口，没有显示的，无效的，非windows工具栏，置顶的窗口。如果不过滤非工具栏的句柄，那么会得到一些奇怪的窗口，会很难判断是否被遮住。
        //int style = GetWindowLong(hwnd, GWL_STYLE);
        //int exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

        //// 有些视频播放窗口是  WS_EX_TOOLWINDOW 但是  WS_POPUP 类型，要包含进来
        //if (IsWindow(hwnd) && IsWindowVisible(hwnd) /*&& IsWindowEnabled(hwnd)*/ && (!(exStyle & WS_EX_TOOLWINDOW) || (style & WS_POPUP) > 0)) {
        //    if (GetAncestor(hwnd, GA_ROOTOWNER) == srcWnd) {
        //        // current window is children of source window
        //        continue;
        //    }
        //}
        auto classNameLength = 256;
        std::wstring className(classNameLength, 0);
        GetClassNameW(hwnd, const_cast<LPTSTR>(className.data()),
            classNameLength);
        WCHAR title[256] = { 0 };
        GetWindowText(hwnd, title, 256);
        if (title[0]) {
            //std::wcout << "handle: " << hwnd << ", title: " << title << ", class name: " << className.c_str() << std::endl;
        }
        //if (wcscmp(title, L"Start") == 0) {
        //if (wcscmp(className.c_str(), L"Windows.UI.Core.CoreWindow") == 0 || wcscmp(className.c_str(), L"ApplicationFrameWindow") == 0) {
        //    std::wcout << "handle: " << hwnd << ", " << className.c_str() << " title: " << title << std::endl;
        //}
        //}
        //if (hwnd == (HWND)(0x10240)) {
        //    std::wcout << index << ": " << className.c_str() << std::endl;
        //}
        ++index;
    }
    std::wcout << index << std::endl;
}