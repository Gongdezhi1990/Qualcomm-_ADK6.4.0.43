/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include <message.h>
#include <stream.h>
#include <usb.h>

#include "usb_device_class.h"
#include <print.h>

#include "usb_device_battery_charging.h"

#ifndef USB_DEVICE_CLASS_REMOVE_CHARGING

bool usbEnumerateBatteryCharging(Task app_task)
{
#ifndef DEBUG_PRINT_ENABLED
    UNUSED(app_task);
#endif
    PRINT(("USB: setup batt charge %p\n", (void *) app_task));
    StreamConfigure((vm_stream_config_key)VM_STREAM_USB_ATTACH_MSG_ENABLED, 1);
    return TRUE;
}

#endif /* USB_DEVICE_CLASS_REMOVE_CHARGING */
