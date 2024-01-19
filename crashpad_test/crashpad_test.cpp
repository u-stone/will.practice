// crashpad_test.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#define NOMINMAX

#include <iostream>
#include <memory>
// On Windows, define NOMINMAX prior to including Crashpad to avoid
// unwanted definitions of macros MIN and MAX from WinAPI

#include <client/crash_report_database.h>
#include <client/crashpad_client.h>
#include <client/settings.h>

static bool startCrashHandler() {
  using namespace crashpad;

  std::map<std::string, std::string> annotations;
  std::vector<std::string> arguments;
  CrashpadClient client;
  bool rc;

  /*
   * ENSURE THIS VALUE IS CORRECT.
   *
   * This is the directory you will use to store and
   * queue crash data.
   */
  std::wstring db_path(L"E:/crashpad");

  /*
   * ENSURE THIS VALUE IS CORRECT.
   *
   * Crashpad has the ability to support crashes both in-process
   * and out-of-process. The out-of-process handler is
   * significantly more robust than traditional in-process crash
   * handlers. This path may be relative.
   */
  std::wstring handler_path(L"E:/Practice/will.practice/crashpad_test/x64/Debug/crashpad_handlerd.exe");

  /*
   * YOU MUST CHANGE THIS VALUE.
   *
   * Update the <Universe> and <Token> in the below
   */
  std::string url("http://127.0.0.1:8000");

  /*
   * REMOVE THIS FOR ACTUAL BUILD.
   *
   * To disable crashpad¡¯s default limitation of
   * 1 upload per hour, pass the --no-rate-limit
   * argument to the handler
   *
   */
  arguments.push_back("--no-rate-limit");

  base::FilePath db(db_path);
  base::FilePath handler(handler_path);

  //std::unique_ptr<crashpad::CrashReportDatabase> database =
  //    crashpad::CrashReportDatabase::Initialize(db);

  //if (database == nullptr || database->GetSettings() == NULL)
  //  return false;

  ///* Enable automated uploads. */
  //database->GetSettings()->SetUploadsEnabled(true);

  rc = client.StartHandler(handler, db, db, url, annotations, arguments, true,
                           true);
  if (rc == false)
    return false;

  /* Optional, wait for Crashpad to initialize. */
  rc = client.WaitForHandlerStart(INFINITE);
  if (rc == false)
    return false;

  return true;
}

std::string recursive(std::string str) {
  char ar[1024] = {0};
  recursive(std::string(ar));
  return ar;
}

int main() {
  startCrashHandler();
  //Sleep(100000);
  recursive("13123uyasdfiuasjdhfgh");
  int* p = nullptr;
  *p = 8;
}
