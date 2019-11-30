/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_device_manager.c

DESCRIPTION
    Contains functions to manage remote devices used by the SWAT library
*/

#include "swat.h"
#include "swat_private.h"
#include "swat_device_manager.h"

/* External lib includes */
#include <bdaddr.h>

/* Firmware includes */
#include <stdlib.h>
#include <sink.h>

/****************************************************************************/
int swatAddDevice(const bdaddr *addr)
{
    uint8 i;
    
    if (!BdaddrIsZero(addr))
    {
        for (i=0; i < swat->max_remote_devs; i++)
        {
            /* Is the device already known by the SWAT library? */
            if ( BdaddrIsSame((const bdaddr *)&swat->remote_devs[i].bd_addr, addr) )
            {
                SWAT_DEBUG(("[SWAT] Known Device ID[%x] BDADDR[%04X %02X %06lX]\n",i,swat->remote_devs[i].bd_addr.nap,swat->remote_devs[i].bd_addr.uap,swat->remote_devs[i].bd_addr.lap));
                return i;
            }
            /* Is the current device entry empty? If so, add device, if not, move to next entry */
            else if (BdaddrIsZero(&swat->remote_devs[i].bd_addr))
            {   
                memset(&swat->remote_devs[i], 0, sizeof(remoteDevice));
                swat->remote_devs[i].bd_addr = *addr;                           /* Set Bluetooth address for device */
                swat->remote_devs[i].id = i;                                    /* Assign ID for device */
                swat->remote_devs[i].signalling_state = swat_signalling_idle;   /* Init signalling state */
                
                swat->remote_devs[i].media_state = swat_media_closed;           /* Init media state */
                swat->remote_devs[i].media_ll_state = swat_media_closed;        /* Init low latency media state */
                swat->remote_devs[i].transport_state = swat_transport_idle;     /* Init transport state for media channel */
                swat->remote_devs[i].transport_ll_state = swat_transport_idle;  /* Init transport state for low latency media channel */
                swat->remote_devs[i].volume = 10;                               /* TODO : Default volume level for the device? For now, just use 10 */
                swat->remote_devs[i].sub_trim = 9;                              /* TODO : Default sub_trim level for the device? For now, just use 9 (midpoint) */
                swat->remote_devs[i].signalling_block = 0;                      /* Ensure signalling block will not block the first command */
                
                /* Initalise the SWAT control flags for the device */
                swat->remote_devs[i].disconnecting_signalling = 0;
                swat->remote_devs[i].initiated_esco_disconnect = 0;
                
                SWAT_DEBUG(("[SWAT] Added Device [%04X %02X %06lX] assigned Device ID[%u]\n",addr->nap,addr->uap,addr->lap,i));
                return i;
            }
        }
    }
    SWAT_DEBUG(("[SWAT] Could not add Device [%04X %02X %06lX]\n",addr->nap,addr->uap,addr->lap));
    return ERROR_MAX_DEVICES;
}


/****************************************************************************/
bool swatRemoveDevice(uint16 device_id)
{
    if (BdaddrIsZero(&swat->remote_devs[device_id].bd_addr))
    {
        SWAT_DEBUG(("[SWAT] : ERROR: Can't remove unknown device ID[%x]\n", device_id));
        return FALSE;
    }
    else
    {
        SWAT_DEBUG(("[SWAT] : Removed DEVICE[%x] from known devices list\n", device_id));
        memset(&swat->remote_devs[device_id], 0, sizeof(remoteDevice));
        return TRUE;
    }
}


/****************************************************************************/
remoteDevice* swatFindDeviceFromBdaddr(const bdaddr *addr)
{
    uint8 i;
    
    if (!BdaddrIsZero(addr))
    {
        for (i=0; i<swat->max_remote_devs; i++)
        {
            if (BdaddrIsSame(&swat->remote_devs[i].bd_addr, addr))
            {
                return &swat->remote_devs[i];
            }
        }
    }

    SWAT_DEBUG(("[SWAT] Device with bdaddr [%04X %02X %06lX] is not recognised\n",addr->nap,addr->uap,addr->lap));
    return NULL;
}


/****************************************************************************/
remoteDevice* swatFindDeviceFromSink(Sink sink)
{
    tp_bdaddr tpaddr;
    
    if (SinkIsValid(sink) && SinkGetBdAddr(sink, &tpaddr))
    {
        return swatFindDeviceFromBdaddr(&tpaddr.taddr.addr);
    }
    else
    {
        return NULL;
    }
}


/****************************************************************************/
remoteDevice* swatFindLocalDeviceFromSink(Sink sink)
{
    uint8 i;
    
    for (i=0; i<swat->max_remote_devs; i++)
    {
        if ( (swat->remote_devs[i].signalling_sink == sink) ||
             (swat->remote_devs[i].media_sink == sink) ||
             (swat->remote_devs[i].media_ll_sink == sink) )
        {
            return &swat->remote_devs[i];
        }
    }
    return NULL;
}
