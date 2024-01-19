/*
 * Created by Qijiyue in 2022-12
 * Copyright (c) 2022 NETEASE RTC. All rights reserved.
 */

#ifndef BASE_PLATFORM_DEVICE_DEVICE_INTERFACE_H_
#define BASE_PLATFORM_DEVICE_DEVICE_INTERFACE_H_

namespace nertc {
class DeviceInterface {
public:
  typedef enum {
    DEVICE_LEVEL_LOW = 0,
    DEVICE_LEVEL_MID = 1,
    DEVICE_LEVEL_HIGH = 2
  } DeviceLevel;

 public:
  DeviceInterface() {}
  virtual ~DeviceInterface() {}

  virtual int GetCpuCores() = 0;
  virtual int GetCpuClock() = 0;
  virtual const char* GetCpuVender() = 0;
  virtual int GetRam() = 0;
  virtual int GetCpuScore() = 0;
  virtual DeviceLevel GetDeviceLevel() = 0;
};

}  // namespace nertc

#endif  // BASE_PLATFORM_DEVICE_DEVICE_INTERFACE_H_

