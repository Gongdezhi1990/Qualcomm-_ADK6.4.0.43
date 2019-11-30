/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_app.c
    
DESCRIPTION
    Handles application launch requests from the client.
*/

#include <panic.h>
#include <stdlib.h>
#include "iap2.h"

#define APP_LENGTH_MSB_OFFSET (0)
#define APP_LENGTH_LSB_OFFSET (1)
#define APP_BUNDLE_ID_MSB_OFFSET (2)
#define APP_BUNDLE_ID_LSB_OFFSET (3)
#define APP_NAME_OFFSET (4)

void Iap2AppLaunchReq(iap2_link *link, char *app_name)
{

    size_t length = strlen(app_name);

    if (length)
    {
        char *buffer;
        uint16 packet_length;

        ++length; /* include terminator */

        packet_length = length + APP_NAME_OFFSET;
        buffer = PanicUnlessMalloc(packet_length);

        memcpy(buffer + APP_NAME_OFFSET, app_name, length);

        buffer[APP_LENGTH_MSB_OFFSET] = packet_length >> 8;
        buffer[APP_LENGTH_LSB_OFFSET] = packet_length & 0xFF;
        buffer[APP_BUNDLE_ID_MSB_OFFSET] = 0;
        buffer[APP_BUNDLE_ID_LSB_OFFSET] = 0;

        if(Iap2ControlMessage(link, RequestAppLaunch,
               packet_length, (uint8*)buffer, FALSE) != iap2_status_success)
        {
            free(buffer);
        }
    }
}
