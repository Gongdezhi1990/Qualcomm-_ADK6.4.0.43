"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd

Provides a helper function to allow flash memory to be programmed using pydbg/pylib
"""
# Python 2 and 3
from __future__ import print_function

import glob
import os
import pprint
import sys
import time

import maker.subsystem_numbers as subsys_numbers


class Pyflash(object):

    def __init__(self, devkit_path, transport_uri):
        self.transport_uri = self.get_pylib_target(transport_uri)
        self.devkit_path  = devkit_path
        self.register_pylib_path(devkit_path)

    def register_pylib_path(self, devkit_path):
        """
        Get the parent path to the pylib in the devkit and add it to sys.path if
        it's not already there
        """
        path = os.path.join(devkit_path, os.path.join("apps", "fw", "tools"))
        if not path in sys.path:
            sys.path += [path, os.path.join(path, "pylib")]
        path = os.path.join(path,"make")
        if not path in sys.path:
            sys.path.append(path)


    def get_pylib_target(self, devkit_target):
        """
        Munge IDE syntax for dongles into pydbg's
        """
        # drop uri scheme, convert to list and drop device name from end
        target_list = devkit_target.split('://')[1].split('/')[:-1]

        # join list back together with ':' inbetween
        return ':'.join(target_list)

    def get_firmware_builds(self, image_type, image_path):
        # Load Curator ELF if possible to support disabling deep sleep by poking the
        # kip table
        curator_elf = glob.glob(os.path.join(self.devkit_path, "images",
                                         "curator_firmware", "*", "proc.elf"))
        # This list will contain zero or one entries depending on whether an ELF
        # is present or not
        firmware_builds = ["curator:%s" % os.path.dirname(cur_elf) for cur_elf in curator_elf]

        # Load the Apps P1 ELF in order to flash it
        image_dir = os.path.dirname(image_path)
        if image_type == "apps1":
            firmware_builds += ["%s:%s" % (image_type,image_dir)]

        firmware_builds = ",".join(firmware_builds)
        return firmware_builds

    def burn(self, image_type, image_path, reset_device=True):

        from csr.front_end.pydbg_front_end import PydbgFrontEnd

        success = False
        sys.stdout.flush()

        subsys_id = subsys_numbers.SubsystemNumbers.get_subsystem_number_from_name(image_type)

        if subsys_id != -1:
            firmware_builds = self.get_firmware_builds(image_type, image_path)

            device, _ = PydbgFrontEnd.attach({"firmware_builds" : firmware_builds,
                                        "device_url" : self.transport_uri,
                                        "preload" : True})
            siflash = device.chip.curator_subsystem.siflash
            apps_fw =  device.chip.apps_subsystem.p0.fw
            apps1_fw = device.chip.apps_subsystem.p1.fw

            if image_type != "curator":
                # Disable Curator USB interrupts to avoid flashing problems with USB attached
                disable_curator_usb_interrupts(device)
                # Try to disable deep sleep, unless it's the Curator itself that's being set up.
                disable_deep_sleep_if_possible(device)

            if image_type == "curator":
                print("Flash curator")
                image_dir = os.path.dirname(image_path)
                siflash.register_program_curator(dir_xuv=image_dir)
            elif image_type == "btss":
                # Special method which deals with the continuous read mode gubbins
                siflash.reprogram_bt(image_path, show_progress=True)
            elif image_type == "curator_fs":
                siflash.identify(subsys_id,0)
                apps_fw.load_curator_fs(image_path)
            elif image_type == "p0_rw_config":
                siflash.identify(subsys_id,0)
                apps_fw.load_rw_config(image_path)
            elif image_type == "p0_ro_cfg_fs":
                siflash.identify(subsys_id,0)
                apps_fw.load_config_fs(image_path)
            elif image_type == "p0_ro_fs":
                siflash.identify(subsys_id,0)
                apps_fw.load_fs(image_path)
            elif image_type == "p0_device_ro_fs":
                siflash.identify(subsys_id,0)
                apps_fw.load_device_config_fs(image_path)
            elif image_type == "apps0":
                siflash.identify(subsys_id,0)
                apps_fw.loader.load_custom("apps_p0", image_path)
            elif image_type == "apps1":
                siflash.identify(subsys_id,0)
                apps1_fw.load()
            else:
                siflash.reprogram(subsys_id,
                        0, image_path)

            if reset_device == True:
                device.reset()

            # Give the hardware a couple of seconds prior to being run
            time.sleep(2)

            success = True

        return success

