#include <Windows.h>
#include <msclr/marshal_cppstd.h>
#include <msclr/marshal.h>
#include <vcclr.h>

using namespace System;
using namespace Microsoft::Office::Interop::PowerPoint;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

int GetCurrentSlideNumber()
{
  // 创建 PowerPoint 应用程序对象
  Application^ pptApp = gcnew Application();

  // 获取当前演示文稿对象
  Presentation^ ppt = pptApp->ActivePresentation;

  // 获取幻灯片播放窗口对象
  SlideShowWindow^ slideShow = ppt->SlideShowWindow;

  // 获取当前幻灯片页码
  int slideNumber = slideShow->View->CurrentShowPosition;

  // 释放对象
  Marshal::ReleaseComObject(slideShow);
  Marshal::ReleaseComObject(ppt);
  Marshal::ReleaseComObject(pptApp);

  return slideNumber;
}

int main()
{
  int slideNumber = GetCurrentSlideNumber();

  //MessageBox::Show("当前页面页码为：" + slideNumber);
  return 0;
}