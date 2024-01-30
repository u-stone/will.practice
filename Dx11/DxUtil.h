#pragma once
#include <string>
#include <winerror.h>


std::wstring TranslateError(HRESULT hr, std::string file, int line);

#define DxError(hr) TranslateError(hr, __FILE__, __LINE__)