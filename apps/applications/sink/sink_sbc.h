/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
\file
\ingroup sink_app
\brief   Set the SBC encoder configuration parameters. 
          This is currently used only for PTS conformance.
*/

#ifndef _SINK_SBC_H_
#define _SINK_SBC_H_


/*******************************************************************************
NAME
    SinkSbcGetAudioConnectFormat 
    
DESCRIPTION
    Get the SBC encoder format required for the header based on the user configuration
    
PARAMETERS

RETURNS
    SBC Encoder Header Format
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetAudioConnectFormat (void);
#else
#define SinkSbcGetAudioConnectFormat() ((void)0)
#endif


/*******************************************************************************
NAME
    SinkSbcGetSamplingFreq
    
DESCRIPTION
    Get the SBC Sampling frequency configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetSamplingFreq(void);
#else
#define SinkSbcGetSamplingFreq() ((void)0)
#endif


/*******************************************************************************
NAME
    SinkSbcGetChannelMode
    
DESCRIPTION
    Get the SBC Channel Mode configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetChannelMode(void);
#else
#define SinkSbcGetChannelMode() ((void)0)
#endif


/*******************************************************************************
NAME
    SinkSbcGetBlockLength
    
DESCRIPTION
    Get the SBC Block Length configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetBlockLength(void);
#else
#define SinkSbcGetBlockLength() ((void)0)
#endif


/*******************************************************************************
NAME
    SinkSbcGetSubbands
    
DESCRIPTION
    Get the SBC Subbands configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetSubbands(void);
#else
#define SinkSbcGetSubbands() ((void)0)
#endif


/*******************************************************************************
NAME
    SinkSbcGetAllocationMethod
    
DESCRIPTION
    Get the SBC Allocation Method configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetAllocationMethod(void);
#else
#define SinkSbcGetAllocationMethod() ((void)0)
#endif

/*******************************************************************************
NAME
    SinkSbcGetMinBitpool
    
DESCRIPTION
    Get the SBC Min Bitpool configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetMinBitpool(void);
#else
#define SinkSbcGetMinBitpool() ((void)0)
#endif

/*******************************************************************************
NAME
    SinkSbcGetMaxBitpool
    
DESCRIPTION
    Get the SBC Max Bitpool configured by user
    
PARAMETERS
    
RETURNS
    None
*/
#ifdef SBC_ENCODER_CONFORMANCE
uint8 SinkSbcGetMaxBitpool(void);
#else
#define SinkSbcGetMaxBitpool() ((void)0)
#endif

#endif /* _SINK_SBC_H_ */

