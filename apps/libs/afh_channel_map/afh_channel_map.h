/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    afh_channel_map.h

DESCRIPTION
    A utility library to update plugin that AFH channel map is pending.
*/


/*!
@file    afh_channel_map.h
@brief  A utility library to update plugin that AFH channel map is pending.
*/


#ifndef AFH_CHANNEL_MAP_
#define AFH_CHANNEL_MAP_

/*!
@brief Interface to update the Bluecore Plugin that AFH channel map is pending

@param None.

@return None
*/
void afhChannelMapChangeIsPending(void);

/*!
@brief Interface to reset the afh update state flag

@param None.

@return None
*/
void afhChannelMapInit(void);

#endif /* BROADCAST_CMD_ */
