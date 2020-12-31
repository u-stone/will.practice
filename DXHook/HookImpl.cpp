#include "pch.h"
#include "HookImpl.h"

void DummyFn() {

}

static HMODULE ModuleFromAddress(PVOID pv)
{
    MEMORY_BASIC_INFORMATION mbi;

    return ((::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0)
        ? (HMODULE)mbi.AllocationBase : NULL);
}

HookImpl::HookImpl()
{
}

HookImpl::~HookImpl()
{
}

HookImpl * HookImpl::instance()
{
    static HookImpl obj;
    return &obj;
}

void HookImpl::SetTargetProcId(DWORD pid)
{
    pid_ = pid;
}

void HookImpl::Install()
{
    //HANDLE hProcess = NULL;
    //char* param_remote = nullptr;
    //HANDLE thread = NULL;
    //__try {
    //    hProcess = OpenProcess(
    //        PROCESS_ALL_ACCESS,
    //        FALSE,
    //        pid_
    //    );

    //    char module_name[MAX_PATH];
    //    ::GetModuleFileNameA(
    //        ModuleFromAddress(DummyFn),
    //        module_name,
    //        MAX_PATH
    //    );

    //    int size_module_name = strnlen_s(module_name, MAX_PATH) + 1;

    //    /// prepare memory of parameter for remote thread.
    //    param_remote = (char*)VirtualAllocEx(
    //        hProcess,
    //        NULL,
    //        size_module_name,
    //        MEM_COMMIT,
    //        PAGE_READWRITE
    //    );

    //    if (!param_remote) {
    //        __leave;
    //    }

    //    if (!WriteProcessMemory(
    //        hProcess,
    //        param_remote,
    //        module_name,
    //        size_module_name,
    //        NULL)
    //        )
    //    {
    //        __leave;
    //    }

    //    PTHREAD_START_ROUTINE thread_routine = (PTHREAD_START_ROUTINE)::GetProcAddress(
    //        ::GetModuleHandleA("Kernel32"), "LoadLibrary");
    //    if (!thread_routine) {
    //        __leave;
    //    }

    //    thread = ::CreateRemoteThread(
    //        hProcess,
    //        NULL,
    //        0,
    //        thread_routine,
    //        (PVOID)param_remote,
    //        0,
    //        NULL
    //    );

    //    if (thread == NULL) {
    //        __leave;
    //    }

    //    ::WaitForSingleObject(thread, INFINITE);
    //}
    //__finally {
    //    if (param_remote) {
    //        ::VirtualFreeEx(hProcess, param_remote, 0, MEM_RELEASE);
    //    }

    //    if (thread) {
    //        ::CloseHandle(thread);
    //    }

    //    if (hProcess) {
    //        ::CloseHandle(hProcess);
    //    }
    //}
}

void HookImpl::Uninstall()
{
    HANDLE hProcess = NULL;
    HANDLE thread = NULL;
    __try {
        char module_name[MAX_PATH];
        ::GetModuleFileNameA(
            ModuleFromAddress(DummyFn),
            module_name,
            MAX_PATH
        );
        hProcess = ::OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE,
            pid_
        );
        if (!hProcess) {
            __leave;
        }

        PTHREAD_START_ROUTINE thread_routine = (PTHREAD_START_ROUTINE)
            ::GetProcAddress(::GetModuleHandleA("Kernel32"), "FreeLibrary");
        if (!thread_routine) {
            __leave;
        }

        thread = ::CreateRemoteThread(
            hProcess,
            NULL,
            0,
            thread_routine,
            0,
            0,
            NULL
        );
    }
    __finally {

    }
}

void HookImpl::DoSth()
{

}
