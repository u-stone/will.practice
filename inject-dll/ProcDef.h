#pragma once
#include <Windows.h>
#include <vector>

struct ProcModules {
    HMODULE hMod;
    std::wstring name;
};

using ProcModuleList = std::vector<ProcModules>;

struct ProcInfo {
    DWORD   pid;
    std::wstring name;
    ProcModuleList modules;
};
