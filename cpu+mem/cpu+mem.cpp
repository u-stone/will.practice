// cpu+mem.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//

#include <Windows.h>
#include <iostream>
#include <thread>
#include "pdh.h"
#include "psapi.h"

#pragma comment(lib, "Pdh.lib")
#include <pdh.h>
#include <pdhmsg.h>
#include <stdio.h>
#include <windows.h>


void cur_per_cpu() {
  PDH_HQUERY hQuery = NULL;
  PDH_STATUS status = ERROR_SUCCESS;
  PDH_HCOUNTER hCounter = NULL;
  DWORD dwBufferSize = 0;  // Size of the pItems buffer
  DWORD dwItemCount = 0;   // Number of items in the pItems buffer
  PDH_FMT_COUNTERVALUE_ITEM* pItems =
      NULL;  // Array of PDH_FMT_COUNTERVALUE_ITEM structures

  if (status = PdhOpenQuery(NULL, 0, &hQuery)) {
    wprintf(L"PdhOpenQuery failed with 0x%x.\n", status);
    goto cleanup;
  }

  // Specify a counter object with a wildcard for the instance.
  if (status = PdhAddCounter(hQuery, L"\\Processor(*)\\% Processor Time", 0,
                             &hCounter)) {
    wprintf(L"PdhAddCounter failed with 0x%x.\n", status);
    goto cleanup;
  }

  // Some counters need two sample in order to format a value, so
  // make this call to get the first value before entering the loop.
  if (status = PdhCollectQueryData(hQuery)) {
    wprintf(L"PdhCollectQueryData failed with 0x%x.\n", status);
    goto cleanup;
  }

  for (;;) {
    Sleep(1000);

    if (status = PdhCollectQueryData(hQuery)) {
      wprintf(L"PdhCollectQueryData failed with 0x%x.\n", status);
      goto cleanup;
    }

    // Get the required size of the pItems buffer.
    status = PdhGetFormattedCounterArray(hCounter, PDH_FMT_DOUBLE,
                                         &dwBufferSize, &dwItemCount, pItems);
    if (PDH_MORE_DATA == status) {
      pItems = (PDH_FMT_COUNTERVALUE_ITEM*)malloc(dwBufferSize);
      if (pItems) {
        status = PdhGetFormattedCounterArray(
            hCounter, PDH_FMT_DOUBLE, &dwBufferSize, &dwItemCount, pItems);
        if (ERROR_SUCCESS == status) {
          // Loop through the array and print the instance name and counter
          // value.
          for (DWORD i = 0; i < dwItemCount; i++) {
            wprintf(L"counter: %s, value %.20g\n", pItems[i].szName,
                    pItems[i].FmtValue.doubleValue);
          }
        } else {
          wprintf(
              L"Second PdhGetFormattedCounterArray call failed with 0x%x.\n",
              status);
          goto cleanup;
        }

        free(pItems);
        pItems = NULL;
        dwBufferSize = dwItemCount = 0;
      } else {
        wprintf(L"malloc for PdhGetFormattedCounterArray failed.\n");
        goto cleanup;
      }
    } else {
      wprintf(L"PdhGetFormattedCounterArray failed with 0x%x.\n", status);
      goto cleanup;
    }
  }

cleanup:

  if (pItems)
    free(pItems);

  if (hQuery)
    PdhCloseQuery(hQuery);  // Closes all counter handles and the query handle
}

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

void cur_pro_init() {
  SYSTEM_INFO sysInfo;
  FILETIME ftime, fsys, fuser;

  GetSystemInfo(&sysInfo);
  numProcessors = sysInfo.dwNumberOfProcessors;

  GetSystemTimeAsFileTime(&ftime);
  memcpy(&lastCPU, &ftime, sizeof(FILETIME));

  self = GetCurrentProcess();
  GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
  memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
  memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}

double cur_pro_getCurrentValue() {
  FILETIME ftime, fsys, fuser;
  ULARGE_INTEGER now, sys, user;
  double percent;

  GetSystemTimeAsFileTime(&ftime);
  memcpy(&now, &ftime, sizeof(FILETIME));

  GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
  memcpy(&sys, &fsys, sizeof(FILETIME));
  memcpy(&user, &fuser, sizeof(FILETIME));
  percent = (sys.QuadPart - lastSysCPU.QuadPart) +
            (user.QuadPart - lastUserCPU.QuadPart);
  percent /= (now.QuadPart - lastCPU.QuadPart);
  percent /= numProcessors;
  lastCPU = now;
  lastUserCPU = user;
  lastSysCPU = sys;

  return percent * 100;
}

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void total_cpu_init() {
  PdhOpenQuery(NULL, NULL, &cpuQuery);
  // You can also use L"\\Processor(*)\\% Processor Time" and get individual CPU
  // values with PdhGetFormattedCounterArray()
  PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL,
                       &cpuTotal);
  PdhCollectQueryData(cpuQuery);
}

double total_cpu_getCurrentValue() {
  PDH_FMT_COUNTERVALUE counterVal;

  PdhCollectQueryData(cpuQuery);
  PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
  return counterVal.doubleValue;
}

int main() {
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);
  DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;

  std::cout << "Total Virtual Memory = " << totalVirtualMem << std::endl;
  DWORDLONG virtualMemUsed =
      memInfo.ullTotalPageFile - memInfo.ullAvailPageFile;
  std::cout << "Virtual Memory currently used = " << virtualMemUsed
            << std::endl;

  PROCESS_MEMORY_COUNTERS_EX pmc;
  GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc,
                       sizeof(pmc));
  SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
  std::cout << "Virtual Memory currently used by current process = "
            << virtualMemUsedByMe << std::endl;

  DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
  std::cout << "Total Physical Memory (RAM) = " << totalPhysMem << std::endl;

  DWORDLONG physMemUsed = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
  std::cout << "Physical Memory currently used (= Total Virtual Memory) = "
            << physMemUsed << std::endl;

  SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
  std::cout << "Physical Memory currently used by current process ( = Virtual "
               "Memory currently used by current process) = "
            << physMemUsedByMe << std::endl;

  auto th1 = std::thread([]() {
    cur_pro_init();
    total_cpu_init();

    while (true) {
      std::cout << "total cpu = " << total_cpu_getCurrentValue() << "\t"
                << "cur cpu = " << cur_pro_getCurrentValue() << std::endl;
      Sleep(1000);
    }
  });

  auto th2 = std::thread(cur_per_cpu);

  th1.join();
  th2.join();

  getchar();
  getchar();
}
