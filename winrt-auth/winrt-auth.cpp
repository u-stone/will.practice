// winrt-auth.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
//#include <winrt/Windows.Security.Authorization.AppCapabilityAccess.h>
//using namespace winrt::Windows::Security::Authorization::AppCapabilityAccess;

//#include <appmodel.h>
//#include <roapi.h>
//#include <wrl\client.h>
//
//#pragma comment(lib, "runtimeobject.lib")

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Security.Authorization.AppCapabilityAccess.h>
#include <iostream>

//C:\Program Files(x86)\Windows Kits\10\Include\10.0.22621.0\cppwinrt\winrt\impl\windows.security.authorization.appcapabilityaccess.2.h
int main()
{

  //// 检查网络访问权限
  //BOOL hasAccess = FALSE;
  //HRESULT hr = AppModelRuntime::GetActivationFactory(
  //  Microsoft::WRL::Wrappers::HStringReference(L"Windows.ApplicationModel.Activation.ActivationFunctions").Get(),
  //  __uuidof(IActivationFunctions),
  //  reinterpret_cast<void**>(ppActivationFunctions)
  //);
  //if (SUCCEEDED(hr))
  //{
  //  BOOL hasCapability = FALSE;
  //  hr = ppActivationFunctions->CheckCapability(L"internetClient", &hasCapability);
  //  if (SUCCEEDED(hr))
  //  {
  //    hasAccess = hasCapability;
  //  }
  //}

  //if (hasAccess)
  //{
  //  std::cout << "已授予网络访问权限" << std::endl;
  //}
  //else
  //{
  //  std::cout << "未授予网络访问权限" << std::endl;
  //}

  //return 0;

  //winrt::Windows::Security::Authorization::AppCapabilityAccess::AppCapability::InternetClient().CheckAccess(AppCapabilityAccess::Microphone);
  //auto accessStatus = AppCapability::CheckAccess(AppCapabilityAccess::Microphone);

  //  std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
