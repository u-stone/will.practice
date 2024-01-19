#include "D3D9Device.h"
#include "demo_define.h"
//#include <windows.h>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>

D3D9Device::D3D9Device(HWND hWnd)
  : hWnd_(hWnd)
{
  d3d9_ = Direct3DCreate9(D3D_SDK_VERSION);
}

D3D9Device::~D3D9Device()
{
  SAFE_RELEASE(d3d9_) //LPDIRECT3D9接口对象的使命完成，我们将其释放掉
}

void D3D9Device::init()
{
  D3DCAPS9 caps;
  int vp = 0;
  if (FAILED(d3d9_->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps))) {
    return;
  }
  if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    vp = D3DCREATE_HARDWARE_VERTEXPROCESSING; //支持硬件顶点运算，我们就采用硬件顶点运算
  else
    vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING; //不支持硬件顶点运算，采用软件顶点运算

  D3DPRESENT_PARAMETERS d3dpp;
  ZeroMemory(&d3dpp, sizeof(d3dpp));
  d3dpp.BackBufferWidth = WINDOW_WIDTH;
  d3dpp.BackBufferHeight = WINDOW_HEIGHT;
  d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
  d3dpp.BackBufferCount = 1;
  d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
  d3dpp.MultiSampleQuality = 0;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.hDeviceWindow = hWnd_;
  d3dpp.Windowed = true;
  d3dpp.EnableAutoDepthStencil = true;
  d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
  d3dpp.Flags = 0;
  d3dpp.FullScreen_RefreshRateInHz = 0;
  d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

  if (FAILED(d3d9_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
    hWnd_, vp, &d3dpp, &device9_))) {
    return;
  }

  currentAdapterMode();
  checkAllFormat();
  availableAdatperModes();
}

void D3D9Device::currentAdapterMode()
{
  if (!d3d9_)
    return;
  int count = d3d9_->GetAdapterCount();
  D3DADAPTER_IDENTIFIER9* adapters = (D3DADAPTER_IDENTIFIER9*)malloc(sizeof(D3DADAPTER_IDENTIFIER9) * count);
  D3DDISPLAYMODE* display_modes = (D3DDISPLAYMODE*)malloc(sizeof(D3DDISPLAYMODE) * count);

  for (int index = 0; index < count; ++index) {
    d3d9_->GetAdapterIdentifier(index, 0, &(adapters[index]));

    d3d9_->GetAdapterDisplayMode(index, &display_modes[index]);
  }

  free(adapters);
  adapters = nullptr;

  free(display_modes);
  display_modes = nullptr;
}

void D3D9Device::checkAllFormat()
{
  // all format list in D3DFORMAT
  auto formats_test = {
    D3DFMT_R8G8B8   ,
    D3DFMT_A8R8G8B8 ,
    D3DFMT_X8R8G8B8 ,
    D3DFMT_R5G6B5   ,
    D3DFMT_X1R5G5B5 ,
    D3DFMT_A1R5G5B5 ,
    D3DFMT_A4R4G4B4 ,
    D3DFMT_R3G3B2   ,
    D3DFMT_A8       ,
    D3DFMT_A8R3G3B2 ,
    D3DFMT_X4R4G4B4 ,
    D3DFMT_A2B10G10R10 ,
    D3DFMT_A8B8G8R8 ,
    D3DFMT_X8B8G8R8 ,
    D3DFMT_G16R16 ,
    D3DFMT_A2R10G10B10 ,
    D3DFMT_A16B16G16R16,
    D3DFMT_A8P8,
    D3DFMT_P8,
    D3DFMT_L8,
    D3DFMT_L16,
    D3DFMT_A8L8,
    D3DFMT_A4L4,

    D3DFMT_V8U8,
    D3DFMT_Q8W8V8U8,
    D3DFMT_V16U16,
    D3DFMT_Q16W16V16U16,
    D3DFMT_CxV8U8,

    D3DFMT_A1,
    D3DFMT_A2B10G10R10_XR_BIAS,
    D3DFMT_BINARYBUFFER
  };

  std::vector<D3DFORMAT> formats_supported;

  for (auto format : formats_test) {
    UINT adapter_mode_count = d3d9_->GetAdapterModeCount(0, format);
    if (adapter_mode_count > 0) {
      auto iter = std::find(std::begin(formats_supported), std::end(formats_supported), format);
      if (iter == std::end(formats_supported)) {
        formats_supported.push_back(format);
      }
    }
  }

  if (formats_supported.empty()) {
    return;
  }
}

void D3D9Device::availableAdatperModes()
{
  int adapter_count = d3d9_->GetAdapterCount();
  std::wostringstream oss;
  struct AdapterInfo {
    std::vector<D3DFORMAT> formats;
    std::vector<D3DDISPLAYMODE> display_modes;
    std::vector<DWORD> behaviors;
  };
  std::map<int, AdapterInfo> adapter_map;

  D3DFORMAT format = D3DFMT_A8R8G8B8;

  for (int index_adapter = 0; index_adapter < adapter_count; ++index_adapter) {
    // get all available display mode
    UINT adapter_mode_count = d3d9_->GetAdapterModeCount(index_adapter, format);

    D3DDISPLAYMODE mode;
    std::vector<D3DDISPLAYMODE> display_modes;
    std::vector<D3DFORMAT> formats_available;

    for (UINT adapter_mode_index = 0; adapter_mode_index < adapter_mode_count; ++adapter_mode_index) {

      auto hr = d3d9_->EnumAdapterModes(index_adapter, format, adapter_mode_index, &mode);
      if (hr != D3D_OK) {
        continue;
      }

      auto iter = std::find_if(display_modes.begin(), display_modes.end(),
        [&](D3DDISPLAYMODE iter) {
        return 
          mode.Width == iter.Width 
          && mode.Height == iter.Width 
          && mode.Format == iter.Format 
          && mode.RefreshRate == iter.RefreshRate;
      });
      if (iter == display_modes.end()) {
        display_modes.push_back(mode);
      }

      auto iter_format = std::find(formats_available.begin(), formats_available.end(), mode.Format);
      if (iter_format == formats_available.end()) {
        formats_available.push_back(mode.Format);
      }
    }
    if (formats_available.empty() || display_modes.empty()) {
      continue;
    }
    adapter_map[index_adapter] = { formats_available, display_modes };
  }

  D3DDEVTYPE device_types[] = {
    D3DDEVTYPE_HAL,
    D3DDEVTYPE_REF,
    D3DDEVTYPE_SW,
    D3DDEVTYPE_NULLREF
  };

  bool HALIsWindowedCompatible = false;
  D3DCAPS9 caps;

  for (int index_adapter = 0; index_adapter < adapter_count; ++index_adapter) {
    // one adapter with multiple device type.
    for (int index_device_type = 0; index_device_type < sizeof(device_types) / sizeof(device_types[0]);
      ++index_device_type) {
      if (FAILED(d3d9_->GetDeviceCaps(index_adapter, device_types[index_device_type], &caps))) {
        continue;
      }

      auto info = adapter_map[index_adapter];
      auto &formats = info.formats;

      std::vector<DWORD> behaviors;
      // check all formats for every device type.
      for (int index_format = 0; index_format < formats.size(); ++index_format) {
        D3DFORMAT format = formats[index_format];
        if (FAILED(d3d9_->CheckDeviceType(index_adapter, device_types[index_device_type],
          format, format, FALSE))) {
          continue;
        }

        if (device_types[index_device_type] == D3DDEVTYPE_HAL) {
          if (caps.Caps2) {
            HALIsWindowedCompatible = true;
          }
        }

        DWORD behavior = 0;
        if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
          behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
          if (caps.DevCaps & D3DCREATE_PUREDEVICE) {
            behavior |= D3DCREATE_PUREDEVICE;
          }
          else {
            behavior |= D3DCREATE_MIXED_VERTEXPROCESSING;
          }
        }
        else {
          behavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        behaviors.push_back(behavior);

        // depth / stencil buffer format for this device/format.
        std::vector<D3DFORMAT> depth_stencil_formats;

        auto find_depth_stencil_format = [&](D3DFORMAT check_format) {
          auto hr = d3d9_->CheckDeviceFormat(index_adapter,
            device_types[index_device_type],
            format,
            D3DUSAGE_DEPTHSTENCIL,
            D3DRTYPE_SURFACE, 
            check_format);

          if (SUCCEEDED(hr)) {
            hr = d3d9_->CheckDepthStencilMatch(index_adapter,
              device_types[index_device_type],
              format,
              format,
              check_format);

            if (SUCCEEDED(hr)) {
              depth_stencil_formats.push_back(check_format);
            }
          }
        };

        // all Depth, stencil, vertex, and index buffers formats list in D3DFORMAT
        auto formats_to_check = {
          D3DFMT_D16_LOCKABLE ,
          D3DFMT_D32          ,
          D3DFMT_D15S1        ,
          D3DFMT_D24S8        ,
          D3DFMT_D24X8        ,
          D3DFMT_D24X4S4      ,
          D3DFMT_D16          ,

          D3DFMT_D32F_LOCKABLE,
          D3DFMT_D24FS8       ,

      #if !defined(D3D_DISABLE_9EX)
          D3DFMT_D32_LOCKABLE ,
          D3DFMT_S8_LOCKABLE  ,
      #endif // !D3D_DISABLE_9EX

          D3DFMT_L16          ,

          D3DFMT_VERTEXDATA,
          D3DFMT_INDEX16 ,
          D3DFMT_INDEX32
        };

        std::for_each(std::begin(formats_to_check), 
          std::end(formats_to_check), 
          find_depth_stencil_format);
      } // index_format

      info.behaviors = behaviors;
    } // index_device_type

  } // adapter_index
}
