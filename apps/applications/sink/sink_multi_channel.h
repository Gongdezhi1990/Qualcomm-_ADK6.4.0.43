/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    Module responsible for reading multi-channel configuration and other 
    functionality related to multi-channel audio output.
*/
#ifndef SINK_MULTI_CHANNEL_H
#define SINK_MULTI_CHANNEL_H

/****************************************************************************
NAME
    SinkMultiChannelInit

DESCRIPTION
    Reads the PSKEY containing multi-channel output configuration and 
    initialises the multi-channel helper plug-in.

RETURNS
    void
*/
void SinkMultiChannelInit(void);

#endif /*SINK_MULTI_CHANNEL_H*/
