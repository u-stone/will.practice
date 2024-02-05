#pragma once
#include <string>
#include <winerror.h>


void TranslateError(HRESULT hr, std::string file, int line);

#define DxError(hr) TranslateError(hr, __FILE__, __LINE__)