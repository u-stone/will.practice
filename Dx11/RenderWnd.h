#pragma once
#include <Windows.h>


class RenderWnd
{
public:
  RenderWnd(HINSTANCE hInst, int width, int height);

private:
  ATOM MyRegisterClass(HINSTANCE hInstance);

protected:
  HINSTANCE hinst_ = NULL;
  HWND wnd_ = NULL;

  int width_ = 800;
  int height_ = 600;
};


