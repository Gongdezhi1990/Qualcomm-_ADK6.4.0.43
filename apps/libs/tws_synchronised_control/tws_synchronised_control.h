/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_synchronised_control.h

DESCRIPTION
    Creates and uses a wallclock via the rtime library in order
    to provide timestamps for use in synchronising AVRCP commands 
    between TWS devices.

*/

/*!
@file   tws_synchronised_control.h
@brief  Creates and uses a wallclock via the rtime library in order
        to provide timestamps for use in synchronising AVRCP commands
        between TWS devices.

*/

#ifndef TWS_SYNCHRONISED_CONTROL_H
#define TWS_SYNCHRONISED_CONTROL_H

#include <sink.h>

/*!
    @brief  tws_timestamp_t type used for synchronising tws control commands
            across an AVRCP link.
*/
typedef uint32 tws_timestamp_t;

/*!
@brief  Enables the library functionality by creating a wallclock
        instance to allow time stamped AVRCP control between peer devices

@param avrcp_sink The sink of the AVRCP link
*/
void twsSynchronisedControlEnable(Sink avrcp_sink);

/*!
@brief  Destroys the AVRCP wallclock instance
*/
void twsSynchronisedControlDisable(void);

/*!
@brief  Checks to see whether the library is available and enabled

@return TRUE if enabled, otherwise FALSE
*/
bool twsSynchronisedControlIsEnabled(void);

/*!
@brief  Provides a future timestamp value to tag outgoing AVRCP control messages

@param  milliseconds_in_future requested time in milliseconds for which timestamp
        is to be provided

@return A timestamp value corresponding to the requested future time period
*/
tws_timestamp_t twsSynchronisedControlGetFutureTimestamp(uint16 milliseconds_in_future);

/*!
@brief  Converts a timestamp value into an actual time period from now

@param  timestamp The timestamp to be converted into a time period

@return The time period from now in milliseconds, negative numbers represent the past
*/
int32 twsSynchronisedControlConvertTimeStampToMilliseconds(tws_timestamp_t timestamp);

#endif
