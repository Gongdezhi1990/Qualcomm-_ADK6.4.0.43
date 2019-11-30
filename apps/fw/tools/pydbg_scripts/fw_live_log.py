############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2018 Qualcomm Technologies International, Ltd.
#   
#
############################################################################
"""
Run the apps1 fw_live_log() command
"""
import sys
import csr

device = csr.dev.attached_device
apps1 = device.chip.apps_subsystem.p1
apps1.fw.env.load=True # Force the firmware environment to load first 
apps1.live_log()
