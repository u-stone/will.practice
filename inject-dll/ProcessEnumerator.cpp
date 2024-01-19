#include "ProcessEnumerator.h"
#include <windows.h>
#include <psapi.h>

ProcessEnumerator::ProcessEnumerator()
{
}

ProcessEnumerator::~ProcessEnumerator()
{
}

void ProcessEnumerator::grab()
{
    procs_info_.clear();

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        return;
    }

    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.
    for (i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            grab_modules(aProcesses[i]);
        }
    }


}

void ProcessEnumerator::grab_modules(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    ProcModuleList modules;
    // Get a handle to the process.
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, processID);

    // Get the process name.
    if (NULL == hProcess)
        return;

    {
        HMODULE hMods[1024];
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, hMods, sizeof(hMods),
            &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMods[0], szProcessName,
                sizeof(szProcessName) / sizeof(TCHAR));
            for (size_t i = 1; i < (cbNeeded / sizeof(HMODULE)); i++)
            {
                TCHAR szModName[MAX_PATH];

                // Get the full path to the module's file.
                if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
                    sizeof(szModName) / sizeof(TCHAR)))
                {
                    // Print the module name and handle value.
                    //_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
                    modules.push_back({ hMods[i], szModName });
                }
            }
        }
    }

    // Print the process name and identifier.
    //_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

    procs_info_[processID] = { processID, szProcessName, modules };
    // Release the handle to the process.
    CloseHandle(hProcess);
}

void ProcessEnumerator::release()
{
    // do nothing.
}

ProcInfo ProcessEnumerator::get_proc(std::wstring proc_name)
{
    for (auto iter : procs_info_) {
        if (iter.second.name == proc_name) {
            return iter.second;
        }
    }

    return ProcInfo();
}
