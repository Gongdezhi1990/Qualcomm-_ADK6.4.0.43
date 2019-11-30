/*
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#include <csrtypes.h>
#include <usb.h>

#include "sink_usb_debug.h"

/****************************************************************************
NAME
    UsbDebugEnableAllow

DESCRIPTION
    Enables USB debug operation.
*/
#ifdef ENABLE_ALLOW_USB_DEBUG

void UsbDebugEnableAllow(void)
{
    /*    Enable the USB Debug Interface by invoking a P0 trap API.
    
          The result of this change will not be persistant after a power cycle or any
          other curator reset eg. panic, tbridge reset etc. This is because only the
          cached value is updated. The default initial value is stored in the curator
          file system, which is read only in the Apps SQIF.   */
    
    UsbDebugAllow(TRUE);
}

#endif /* ENABLE_ALLOW_USB_DEBUG */
