// recursivecall.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

LONG MyUnhandledExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo);
#pragma comment(lib, "Dbghelp.lib")
#include <crtdbg.h>
#include <exception>
#include <minidumpapiset.h>

char test() {
    char buf[1024] = { 0 };
    buf[1000] = 1;
    std::cout << buf[1023];
    return test();
}

int main()
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MyUnhandledExceptionFilter);
    std::cout << "Hello World!\n";
    int i = 0;
    //std::cout << 100 / i;
    //test();
}


LONG MyUnhandledExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo)
{
    HANDLE hFile = CreateFileA(("D:/MiniDump.dmp"), GENERIC_READ | GENERIC_WRITE,
        0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
    {
        // Create the minidump 

        MINIDUMP_EXCEPTION_INFORMATION mdei;

        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = ExceptionInfo;
        mdei.ClientPointers = FALSE;

        //MINIDUMP_CALLBACK_INFORMATION mci;

        //mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback;
        //mci.CallbackParam = 0;

        MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

        BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            hFile, mdt, (ExceptionInfo != 0) ? &mdei : 0, 0, NULL);

        if (!rv)
            printf(("MiniDumpWriteDump failed. Error: %u \n"), GetLastError());
        else
            printf(("Minidump created.\n"));

        // Close the file 

        CloseHandle(hFile);

    }
    else
    {
        printf(("CreateFile failed. Error: %u \n"), GetLastError());
    }
    //MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, );
    return EXCEPTION_CONTINUE_SEARCH;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
