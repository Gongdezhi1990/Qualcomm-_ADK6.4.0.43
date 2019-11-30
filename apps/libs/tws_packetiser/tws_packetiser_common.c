/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packetiser_common.c
*/

#include <sink.h>

#include <tws_packetiser_private.h>

uint8 *tpSinkMapAndClaim(Sink sink, uint32 len)
{
    uint8 *dest = SinkMap(sink);
    if (dest)
    {
        int32 extra = len - SinkClaim(sink, 0);
        if (extra > 0)
        {
            if (SinkClaim(sink, extra) == 0xFFFF)
            {
                return NULL;
            }
        }
    }
    return dest;
}
