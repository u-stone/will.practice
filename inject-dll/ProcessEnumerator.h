#pragma once
#include <Windows.h>
#include <map>
#include "ProcDef.h"


class ProcessEnumerator
{
public:
    ProcessEnumerator();
    ~ProcessEnumerator();

    void grab();
    void grab_modules(DWORD pid);
    void release();

    ProcInfo get_proc(std::wstring proc_name);

private:
    std::map<long, ProcInfo> procs_info_;
};



