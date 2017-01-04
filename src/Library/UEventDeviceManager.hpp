//
// Copyright (C) 2016 Red Hat, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors: Daniel Kopecek <dkopecek@redhat.com>
//
#pragma once
#include <build-config.h>

#if defined(HAVE_UEVENT)

#include "Typedefs.hpp"
#include "Common/Thread.hpp"

#include "DeviceManager.hpp"
#include "Device.hpp"
#include "Rule.hpp"
#include "SysFSDevice.hpp"

#include <istream>
#include <sys/stat.h>
#include <dirent.h>

namespace usbguard {
  class UEventDeviceManager;

  class UEventDevice : public Device
  {
  public:
    UEventDevice(UEventDeviceManager& device_manager, SysFSDevice& sysfs_device);

    SysFSDevice& sysfsDevice();
    const String& getSysPath() const;
    bool isController() const;

  protected:
    void readDescriptors(std::istream& stream);
    void readConfiguration(int c_num, std::istream& stream);
    void readInterfaceDescriptor(int c_num, int i_num, std::istream& stream);
    void readEndpointDescriptor(int c_num, int i_num, int e_num, std::istream& stream);

  private:
    SysFSDevice _sysfs_device;
  };

#if !defined(USBGUARD_SYSFS_ROOT)
#define USBGUARD_SYSFS_ROOT "/sys"
#endif

  class UEventDeviceManager : public DeviceManager
  {
  public:
    UEventDeviceManager(DeviceManagerHooks& hooks, const String& sysfs_root = USBGUARD_SYSFS_ROOT, bool dummy_mode = false);
    ~UEventDeviceManager();

    void setDefaultBlockedState(bool state);

    void start();
    void stop();
    void scan();

    Pointer<Device> applyDevicePolicy(uint32_t id, Rule::Target target);
    void insertDevice(Pointer<UEventDevice> device);
    Pointer<Device> removeDevice(const String& syspath);

    uint32_t getIDFromSysPath(const String& syspath) const;

  protected:
    int ueventOpen();
    int ueventDummyOpen();
    void sysfsApplyTarget(SysFSDevice& sysfs_device, Rule::Target target);

    bool knownSysPath(const String& syspath, uint32_t * id = nullptr) const;
    void learnSysPath(const String& syspath, uint32_t id = 0);
    void forgetSysPath(const String& syspath);

    void thread();
    void ueventProcessRead();
    void ueventProcessUEvent(const UEvent& uevent);
    void ueventEnumerateDevices();
    void ueventEnumerateDummyDevices();

    static String ueventEnumerateFilterDevice(const String& filepath, const struct dirent* direntry);
    void ueventEnumerateTriggerDevice(const String& filepath);

    void processDevicePresence(SysFSDevice& sysfs_device);

    void processDeviceInsertion(SysFSDevice& sysfs_device, bool signal_present);
    void processDevicePresence(uint32_t id);
    void processDeviceRemoval(const String& sysfs_devpath);

  private:
    Thread<UEventDeviceManager> _thread;
    int _uevent_fd;
    int _wakeup_fd;
    StringKeyMap<uint32_t> _syspath_map;
    String _sysfs_root;
    std::atomic<bool> _enumeration_complete;
    bool _default_blocked_state;
    bool _dummy_mode;
  };
} /* namespace usbguard */
#endif /* HAVE_UEVENT */