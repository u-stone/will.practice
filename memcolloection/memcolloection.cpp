// memcolloection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <windows.h>
#include <psapi.h>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

void PrintMemoryInfo(DWORD processID) {
  HANDLE hProcess;
  PROCESS_MEMORY_COUNTERS pmc;

  // Print the process identifier.

  printf("\nProcess ID: %u\n", processID);

  // Print information about the memory usage of the process.

  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                         processID);
  if (NULL == hProcess)
    return;

  if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
    printf("\tPageFaultCount: 0x%08d\n", pmc.PageFaultCount);
    printf("\tPeakWorkingSetSize: 0x%08d\n", pmc.PeakWorkingSetSize >> 20);
    printf("\tWorkingSetSize: 0x%08d\n", pmc.WorkingSetSize >> 20);
    printf("\tQuotaPeakPagedPoolUsage: 0x%08d\n",
           pmc.QuotaPeakPagedPoolUsage >> 20);
    printf("\tQuotaPagedPoolUsage: 0x%08d\n", pmc.QuotaPagedPoolUsage >> 20);
    printf("\tQuotaPeakNonPagedPoolUsage: 0x%08d\n",
           pmc.QuotaPeakNonPagedPoolUsage >> 20);
    printf("\tQuotaNonPagedPoolUsage: 0x%08d\n",
           pmc.QuotaNonPagedPoolUsage >> 20);
    printf("\tPagefileUsage: 0x%08d\n", pmc.PagefileUsage >> 20);
    printf("\tPeakPagefileUsage: 0x%08d\n", pmc.PeakPagefileUsage >> 20);
  }

  CloseHandle(hProcess);
}

int main(void) {
  // Get the list of process identifiers.

  DWORD aProcesses[1024], cbNeeded, cProcesses;
  unsigned int i;

  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    return 1;
  }

  // Calculate how many process identifiers were returned.

  cProcesses = cbNeeded / sizeof(DWORD);

  // Print the memory usage for each process

  for (i = 0; i < cProcesses; i++) {
    PrintMemoryInfo(aProcesses[i]);
  }

  return 0;
}