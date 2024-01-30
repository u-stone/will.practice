#include "DxUtil.h"
#include <comdef.h>
#include "dxerr/dxerr.h"

std::wstring TranslateError(HRESULT hr, std::string file, int line)
{
  _com_error err(hr);
  if (0 == err.WCode()) {
    TCHAR buf[256] = { 0 };
    DXGetErrorDescription(hr, buf, 256);
    //OutputDebugString(DXGetErrorString(hr));
    TCHAR errstr[512] = { 0 };
    swprintf_s(errstr, L"Error: in file %hs : %d, %ws", file.c_str(), line, buf);
    OutputDebugString(errstr);
  }
  else {
    TCHAR errstr[512] = { 0 };
    swprintf_s(errstr, L"Error: in file %hs, line: %d, %ws", file.c_str(), line, err.ErrorMessage());
    OutputDebugString(errstr);
  }
  return err.ErrorMessage();
}
