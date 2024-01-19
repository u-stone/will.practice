#include "device_win.h"
/*
 * Created by Qijiyue in 2022-12
 * Copyright (c) 2022 NETEASE RTC. All rights reserved.
 */

#include <WbemIdl.h>
#include <Windows.h>
#include <bcrypt.h>
#include <comdef.h>
#include <ntddscsi.h>
#include <powerbase.h>
#include <intrin.h>

#include <array>
#include <algorithm>
#include <cctype>
#include <string>
#include <type_traits>
#include <vector>
#include <locale>
#include <codecvt>
#include <regex>

#pragma comment(lib, "wbemuuid.lib")
#define MAX_INTEL_TOP_LVL 4
#define SSE_POS   0x02000000
#define SSE2_POS  0x04000000
#define SSE3_POS  0x00000001
#define SSE41_POS 0x00080000
#define SSE42_POS 0x00100000
#define AVX_POS   0x10000000
#define AVX2_POS  0x00000020
#define LVL_NUM   0x000000FF
#define LVL_TYPE  0x0000FF00
#define LVL_CORES 0x0000FFFF

namespace nertc {
  using pfnCallNtPowerInformation = NTSTATUS(WINAPI*)(POWER_INFORMATION_LEVEL InformationLevel,
    PVOID InputBuffer, ULONG InputBufferLength,
    PVOID OutputBuffer, ULONG OutputBufferLength);
  typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
  } PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

  /**
   * cpuid wrapper for Linux, MacOS and Windows
   * @param func_id
   * @param sub_func_id
   * @param regs
   */
  inline void cpuid(uint32_t func_id, uint32_t sub_func_id, uint32_t regs[4]) {
#ifdef _MSC_VER
    __cpuidex(reinterpret_cast<int*>(regs), static_cast<int>(func_id),
      static_cast<int>(sub_func_id));
#elif defined(__GNUC__) || defined(__clang__)
    __get_cpuid_count(func_id, sub_func_id, &regs[0], &regs[1], &regs[2],
      &regs[3]);
#elif __CYGWIN
    cpuid(&regs[0], &regs[1], &regs[2], &regs[3], func_id, sub_func_id);
#endif
  }

  DeviceWin::DeviceWin() {
    HRESULT hres;
    hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
      return ;
    }
    hres = CoInitializeSecurity(
      nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);
    if (FAILED(hres)) {
      CoUninitialize();
      return ;
    }
    IWbemLocator* pLoc = nullptr;
    hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
      IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hres)) {
      CoUninitialize();
      return ;
    }
    IWbemServices* pSvc = nullptr;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr,
      0, nullptr, nullptr, &pSvc);
    if (FAILED(hres)) {
      pLoc->Release();
      CoUninitialize();
      return ;
    }
    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
      RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
      nullptr, EOAC_NONE);
    if (FAILED(hres)) {
      pSvc->Release();
      pLoc->Release();
      CoUninitialize();
      return ;
    }

    std::vector<std::string> fields = {
      "NumberOfCores",
      "NumberOfLogicalProcessors",
      "MaxClockSpeed", // Maximum speed of the processor, in MHz
      "Name",
    };

    for (auto field : fields) {
      std::string query("SELECT " + field + " FROM Win32_Processor");

      IEnumWbemClassObject* pEnumerator = nullptr;
      hres = pSvc->ExecQuery(
        bstr_t(L"WQL"), bstr_t(std::wstring(query.begin(), query.end()).c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
        &pEnumerator);
      if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
      }
      IWbemClassObject* pclsObj = nullptr;
      ULONG uReturn = 0;
      while (pEnumerator) {
        pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (!uReturn) {
          break;
        }

        VARIANT vtProp;
        pclsObj->Get(std::wstring(field.begin(), field.end()).c_str(), 0, &vtProp,
          nullptr, nullptr);

        if (field == "Name") {
          std::wstring ws(vtProp.bstrVal, SysStringLen(vtProp.bstrVal));
          using convert_type = std::codecvt_utf8<wchar_t>;
          std::wstring_convert<convert_type, wchar_t> converter;

          //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
          wmi_cpu_info_.vendor = converter.to_bytes(ws);
        }
        else if (field == "NumberOfCores") {
          wmi_cpu_info_.cores = vtProp.uintVal;
        }
        else if (field == "NumberOfLogicalProcessors") {
          wmi_cpu_info_.logic_cores = vtProp.uintVal;
        }
        else if (field == "MaxClockSpeed") {
          wmi_cpu_info_.clock_speed = vtProp.uintVal;
        }

        VariantClear(&vtProp);
        pclsObj->Release();
      }
      if (pEnumerator)
        pEnumerator->Release();
    }

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
  }

  DeviceWin::~DeviceWin() {}

  int DeviceWin::GetCpuCores() {
    if (wmi_cpu_info_.cores == 0) {
      uint32_t regs[4]{};
      std::string vendorId = GetCpuVender();
      std::for_each(vendorId.begin(), vendorId.end(), [](char& in) { in = ::toupper(in); });
      cpuid(0, 0, regs);
      uint32_t HFS = regs[0];
      if (vendorId.find("INTEL") != std::string::npos) {
        if (HFS >= 11) {
          for (int lvl = 0; lvl < MAX_INTEL_TOP_LVL; ++lvl) {
            uint32_t regs_2[4]{};
            cpuid(0x0b, lvl, regs_2);
            uint32_t currLevel = (LVL_TYPE & regs_2[2]) >> 8;
            if (currLevel == 0x01) {
              int numCores = getNumLogicalCores() / static_cast<int>(LVL_CORES & regs_2[1]);
              if (numCores > 0) {
                wmi_cpu_info_.cores = numCores;
                break;
              }
            }
          }
        }
        else {
          if (HFS >= 4) {
            uint32_t regs_3[4]{};
            cpuid(4, 0, regs_3);
            int numCores = getNumLogicalCores() / static_cast<int>(1 + ((regs_3[0] >> 26) & 0x3f));
            if (numCores > 0) {
              wmi_cpu_info_.cores = numCores;
            }
          }
        }
      }
      else if (vendorId.find("AMD") != std::string::npos) {
        if (HFS > 0) {
          uint32_t regs_4[4]{};
          cpuid(0x80000000, 0, regs_4);
          if (regs_4[0] >= 8) {
            int numCores = 1 + (regs_4[2] & 0xff);
            wmi_cpu_info_.cores = numCores;
          }
        }
      }
      else {
        return 0;
      }
    }
    return wmi_cpu_info_.cores;
  }

  int DeviceWin::GetCpuClock() {
    if (wmi_cpu_info_.clock_speed == 0) {
      int max_freq = 0;
      HMODULE hPwr;
      do {
        uint32_t buf_len;
        NTSTATUS status;
        std::unique_ptr<char[]> buf;
        hPwr = ::LoadLibraryA("PowrProf.dll");
        if (hPwr == NULL)
          break;
        pfnCallNtPowerInformation pCallNtPowerInformation = (pfnCallNtPowerInformation)::GetProcAddress(hPwr, "CallNtPowerInformation");
        if (pCallNtPowerInformation == NULL)
          break;

        buf_len = sizeof(PROCESSOR_POWER_INFORMATION) * GetCpuCores();
        buf = std::make_unique<char[]>(buf_len);
        while (status = pCallNtPowerInformation(ProcessorInformation, NULL, 0, buf.get(), buf_len) == 0xC0000023 /*STATUS_BUFFER_TOO_SMALL*/) {
          if (buf_len >= 2 * 1024 * 1024)
            break;

          buf_len *= 2;
          buf = std::make_unique<char[]>(buf_len);
        }
        if (status != 0x00000000 /*STATUS_SUCCESS*/ || !buf)
          break;

        PPROCESSOR_POWER_INFORMATION power_info = (PPROCESSOR_POWER_INFORMATION)buf.get();
        max_freq = power_info->MaxMhz;
      } while (0);
      wmi_cpu_info_.clock_speed = max_freq;
    }
    return wmi_cpu_info_.clock_speed;
  }

  const char* DeviceWin::GetCpuVender() {
#if 0
    std::vector<const wchar_t*> vendor{};
    queryWMI("Win32_Processor", "Manufacturer", vendor);
    if (vendor.empty()) {
      static std::string v;
      v.clear();
      uint32_t regs[4]{ 0 };
      cpuid(0, 0, regs);
      v += std::string((const char*)&regs[1], 4);
      v += std::string((const char*)&regs[3], 4);
      v += std::string((const char*)&regs[2], 4);
      return v.c_str();
    }

    std::wstring tmp(vendor[0]);
    //setup converter
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    static std::string converted_str;
    converted_str = converter.to_bytes(tmp);
    return converted_str.c_str();
#endif

    if (wmi_cpu_info_.vendor.empty()) {
      std::vector<std::array<int, 4>> extdata_;
      std::array<int, 4> cpui;
      int nExIds_;

      // Calling __cpuid with 0x80000000 as the function_id argument
      // gets the number of the highest valid extended ID.
      __cpuid(cpui.data(), 0x80000000);
      nExIds_ = cpui[0];

      static char brand[0x40];
      memset(brand, 0, sizeof(brand));

      for (int i = 0x80000000; i <= nExIds_; ++i)
      {
        __cpuidex(cpui.data(), i, 0);
        extdata_.push_back(cpui);
      }

      // Interpret CPU brand string if reported
      if (nExIds_ >= 0x80000004)
      {
        memcpy(brand, extdata_[2].data(), sizeof(cpui));
        memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
        memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
      }

      wmi_cpu_info_.vendor = brand;
    }

    return wmi_cpu_info_.vendor.c_str();
  }

  int DeviceWin::GetRam() {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    // The amount of actual physical memory, in bytes.
    return static_cast<int>(status.ullTotalPhys >> 10);
  }

  int DeviceWin::getIntelCpuScore(std::string cpuType, std::string genNumStr, std::string flag) {
    int genNum = std::stoi(genNumStr);
    int score = 100;
    if (cpuType == "i3") {
      if (genNum < 4000) {
        // i3-3xxx
        score = 50;
      }
      else if (genNum < 7000 && (flag == "M" || flag == "U" || flag == "Y" || flag == "UM")) {
        // 4xxx--6xxx M/U/Y/UM
        score = 50;
      }
    }
    else if (cpuType == "i5") {
      if (genNum < 5000) {
        if (flag == "U" || flag == "Y") {
          score = 50;
        }
        else if (genNum < 2500 && flag == "M") {
          // 24XXM
          score = 50;
        }
      }
    }
    else if (cpuType == "i7") {
      if (genNum < 2700) {
        if (genNum > 1000) {
          if (flag == "M") {
            // 26xxM
            score = 50;
          }
        }
        else if (genNum <= 660 && flag == "LM") {
          // <=660LM
          score = 50;
        }
      }
    }
    return score;
  }

  int DeviceWin::getScoreByClockspeed(int frequency)
  {
    // Clockspeed (in MHz) found: 
    //[ 200, 700, 800, 900, 1000, 
    //  1100, 1200, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 
    //  2100, 2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900, 3000, 
    //  3100, 3200, 3300, 3400, 3500, 3600, 3700, 3800, 3900, 4000, 
    //  4100, 4200, 4300, 4400, 4500, 4600, 4700, 4800, 4900, 5000, 
    //  5100, 5200, 5300, 5400, 5500, 5600, 5700, 5800]
    // Nearly 0.7GHz to 5.8GHz step 0.1.

    int result = 100;

    if (frequency <= 0) {
      result = 100;
    }
    else {
      result = frequency / 50;
      result = min(result, 100);
    }
    return result;
  }

  int DeviceWin::getScoreByCoreNumber(int cores)
  {
    // Cores found:
    // [1, 2, 3, 4, 5, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 32, 36, 38, 40, 48, 60, 64, 80, 128]
    int result = 100;
    if (cores <= 0) {
      result = 100;
    }
    else if (cores <= 2) {
      result = 20;
    }
    else if (cores <= 4) {
      result = 40;
    }
    else if (cores <= 8) {
      result = 50;
    }
    else if (cores <= 16) {
      result = 55;
    }
    else if (cores <= 24) {
      result = 60;
    }
    else if (cores <= 36) {
      result = 70;
    }
    else if (cores <= 48) {
      result = 80;
    }
    else if (cores <= 64) {
      result = 90;
    }
    else {
      result = 100;
    }

    return result;
  }

  int DeviceWin::GetCpuScore() {
    //const char* cpuid = "Intel(R) Core(TM) i7-8700M CPU @ 3.20GHz";
    //const char* cpuid = GetCpuVender();
    //std::string pattern = "(Intel.*)(i\\d{1})-(\\d{3,5})([a-zA-Z]{0,1}).(.*).@.([0-9|.]{0,4})GHz";
    //std::regex express(pattern, std::regex_constants::syntax_option_type::ECMAScript);
    //std::cmatch c_results;
    //if (std::regex_search(cpuid, c_results, express)) {
    //  if (c_results.size() == 7) {
    //    // c_results[1] = 'Intel(R) Core(TM)'
    //    // c_results[2] = 'i7'
    //    // c_results[3] = '8700'
    //    // c_results[4] = 'M'
    //    // c_results[5] = 'CPU'
    //    // c_results[6] = '3.20'

    //    // Intel cpu judge. others is todo.
    //    if (c_results[0].str().find("Intel") != std::string::npos) {
    //      return getIntelCpuScore(c_results[2], c_results[3], c_results[4]);
    //    }
    //  }
    //}
    //"iPhone15,3";
    const char* cpuid = "Apple A16 Bionic @ 3.5 GHz";
    //const char* cpuid = GetCpuVender();
    std::string pattern = "(Apple.*).@.([0-9|.]{0,3}).GHz";
    std::regex express(pattern, std::regex_constants::syntax_option_type::ECMAScript);
    std::cmatch c_results;
    if (std::regex_search(cpuid, c_results, express)) {
      if (c_results.size() == 3) {
        // c_results[1] = 'Apple A16 Bionic'
        // c_results[2] = '3.5'
      }
    }
    return 0;

    return getScoreByClockspeed(GetCpuClock()) * 0.64 + getScoreByCoreNumber(GetCpuCores()) * 0.36;
  }

  DeviceInterface::DeviceLevel DeviceWin::GetDeviceLevel() {
    int score = GetCpuScore();
    DeviceInterface::DeviceLevel level = DeviceInterface::DEVICE_LEVEL_HIGH;
    if (score < 40) {
      level = DeviceInterface::DEVICE_LEVEL_LOW;
    }
    else if (score < 50) {
      level = DeviceInterface::DEVICE_LEVEL_MID;
    }
    else {
      level = DeviceInterface::DEVICE_LEVEL_HIGH;
    }
    return level;
  }

  int DeviceWin::getNumLogicalCores() {
    if (wmi_cpu_info_.logic_cores == 0) {
      std::string vendorId = GetCpuVender();
      std::for_each(vendorId.begin(), vendorId.end(), [](char& in) { in = ::toupper(in); });
      uint32_t regs[4]{};
      cpuid(0, 0, regs);
      uint32_t HFS = regs[0];
      if (vendorId.find("INTEL") != std::string::npos) {
        if (HFS >= 0xb) {
          for (int lvl = 0; lvl < MAX_INTEL_TOP_LVL; ++lvl) {
            uint32_t regs_2[4]{};
            cpuid(0x0b, lvl, regs_2);
            uint32_t currLevel = (LVL_TYPE & regs_2[2]) >> 8;
            if (currLevel == 0x02) {
              wmi_cpu_info_.logic_cores = static_cast<int>(LVL_CORES & regs_2[1]);
              break;
            }
          }
        }
      }
      else if (vendorId.find("AMD") != std::string::npos) {
        if (HFS > 0) {
          cpuid(1, 0, regs);
          wmi_cpu_info_.logic_cores = static_cast<int>(regs[1] >> 16) & 0xff;
        }
        else {
          wmi_cpu_info_.logic_cores = 1;
        }
      }
      else {
        wmi_cpu_info_.logic_cores = 0;
      }
    }
    return wmi_cpu_info_.logic_cores;
  }

}  // namespace nertc
