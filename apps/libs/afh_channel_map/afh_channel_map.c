/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    afh_channel_map.c

DESCRIPTION
      A utility library to handle hardware specific dependency of AFH channel map update.
*/

#include <panic.h>
#include <kalimba.h>
#include <broadcast_msg_interface.h>
#include <erasure_code_common.h>
#include <broadcast_context.h>
#include <sink.h>
#include "afh_channel_map.h"

/** Macro for making a coding info byte */
#define EC_CI_MAKE(ID, SEQ, PADDING, AFH_CHANNEL_MAP_CHANGE_PENDING) \
    (uint8)(((ID) & EC_CI_CODING_ID_MASK) | \
            (((SEQ) & EC_CI_SEQUENCE_NUMBER_MASK) << EC_CI_SEQUENCE_NUMBER_SHIFT) | \
            (((PADDING) & EC_CI_PADDING_OCTETS_MASK) << EC_CI_PADDING_OCTETS_SHIFT) | \
            (((AFH_CHANNEL_MAP_CHANGE_PENDING) & EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_MASK) << EC_CI_AFH_CHANNEL_MAP_CHANGE_PENDING_SHIFT))

/* Enumerate the states for determining/settting AFH channel map update pending state */
typedef enum __afh_channel_map_update_pending_state
{
    afh_channel_map_change_pending_state_0 = 0,
    afh_channel_map_change_pending_state_1 = 1,
    afh_channel_map_change_pending_state_unknown
} afh_channel_map_change_pending_state_t;

static bool afh_state = afh_channel_map_change_pending_state_0;

static void sendAfhUpdateFromVm(void)
{
    uint16 toflush = 0;
    uint8* dest = NULL;
    uint16 offset = 0;

    /* We are interested in only sending the header information */
    toflush = (uint16)(EC_HEADER_SIZE_OCTETS);

    offset = SinkClaim(BroadcastContextGetSink(), toflush);

    if(offset != 0xFFFF)
    {
        dest = SinkMap(BroadcastContextGetSink());
        if(dest)
            dest = dest + offset;
    }

    if(dest)
    {
        dest[0] = BroadcastContextGetStreamId();
        dest[1] = EC_CI_MAKE(EC_NOT_ENCODED, 0, 0, afh_state);
        dest[2] = 0;
        PanicFalse(SinkFlush(BroadcastContextGetSink(), toflush));
    }
}

/*!
    @brief Updates the Bluecore Plugin that AFH channel map is pending
*/
/******************************************************************************/
void afhChannelMapChangeIsPending(void)
{
    afh_state = !afh_state;

    if(!KalimbaSendMessage(KALIMBA_MSG_AFH_CHANNEL_MAP_CHANGE_PENDING, 0, 0, 0, 0))
    {
        /* In case DSP is not running, we should still send AFH update to receivers from VM.
        Failing to do this may lead to audio drops on receivers */
        sendAfhUpdateFromVm();
    }
}

/******************************************************************************/
void afhChannelMapInit(void)
{
    afh_state = afh_channel_map_change_pending_state_0;
}

