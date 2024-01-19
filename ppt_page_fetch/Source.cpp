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
  // ���� PowerPoint Ӧ�ó������
  Application^ pptApp = gcnew Application();

  // ��ȡ��ǰ��ʾ�ĸ����
  Presentation^ ppt = pptApp->ActivePresentation;

  // ��ȡ�õ�Ƭ���Ŵ��ڶ���
  SlideShowWindow^ slideShow = ppt->SlideShowWindow;

  // ��ȡ��ǰ�õ�Ƭҳ��
  int slideNumber = slideShow->View->CurrentShowPosition;

  // �ͷŶ���
  Marshal::ReleaseComObject(slideShow);
  Marshal::ReleaseComObject(ppt);
  Marshal::ReleaseComObject(pptApp);

  return slideNumber;
}

int main()
{
  int slideNumber = GetCurrentSlideNumber();

  //MessageBox::Show("��ǰҳ��ҳ��Ϊ��" + slideNumber);
  return 0;
}