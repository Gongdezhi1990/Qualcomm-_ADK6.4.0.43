#!/usr/bin/env python
# Copyright (c) 2017 Qualcomm Technologies International, Ltd.
#   Part of 6.3
"""
    Maintain subsystem name to subsystem number mapping
"""

class SubsystemNumbers(object):
    subsys_ids = {
        "curator" : 0,
        "btss" : 2,
        "audio" : 3,
        "apps0" : 4,
        "apps1" : 4,
        "curator_fs" : 4,
        "p0_rw_config" : 4,
        "p0_ro_cfg_fs" : 4,
        "p0_ro_fs" : 4,
        "p0_device_ro_fs" : 4 
    }
    
    @classmethod
    def get_subsystem_number_from_name(cls, name):
        retval = -1
        if name in cls.subsys_ids:
            retval = cls.subsys_ids[name]
        
        return retval
        
    

