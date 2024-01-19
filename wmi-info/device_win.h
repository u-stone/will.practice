/*
 * Created by LiuGuoyuan in 2022-12
 * Copyright (c) 2022 NETEASE RTC. All rights reserved.
 */
#ifndef RTC_BASE_PLATFORM_DEVICE_DEVICE_WIN_H_
#define RTC_BASE_PLATFORM_DEVICE_DEVICE_WIN_H_

#include "device_interface.h"
#include <string>
namespace nertc {

class DeviceWin : public DeviceInterface {
 public:
  DeviceWin();
  ~DeviceWin();

  virtual int GetCpuCores() override;

  virtual int GetCpuClock() override;

  virtual const char* GetCpuVender() override;

  // The amount of actual physical memory, in KB.
  virtual int GetRam() override;

  virtual int GetCpuScore() override;

  virtual DeviceLevel GetDeviceLevel() override;

private:
  int getNumLogicalCores();
  int getIntelCpuScore(std::string cpuType, std::string genNumStr, std::string flag);
  int getScoreByClockspeed(int frequency /* MHz*/);
  int getScoreByCoreNumber(int cores);

  struct info {
    uint32_t cores = 0;
    uint32_t logic_cores = 0;
    uint32_t clock_speed = 0;
    std::string vendor;
  } wmi_cpu_info_;
};

}  // namespace nertc

#endif  // BASE_PLATFORM_DEVICE_DEVICE_IOS_H_