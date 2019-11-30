/*
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.

*/

/*!
@file
@ingroup sink_app
@brief
    Contains utility functions used by the sink application
*/

#ifndef _SINK_UTILS_H_
#define _SINK_UTILS_H_

#include <csrtypes.h>


/*******************************************************************************
NAME
    bitCounter16

DESCRIPTION
    Function to count the number of set bits in a uint16 bitmask
*/
uint16 bitCounter16(uint16 to_count);


/*******************************************************************************
NAME    
    bitCounter32
    
DESCRIPTION
    Function to count the number of set bits in a 32bit mask
*/
uint16 bitCounter32(uint32 to_count);

/***************************************************************************
NAME
    memcpy_unpack
 
DESCRIPTION
    Utility function to copy uint16 packed bytes into uint8 bytes
 
PARAMS
    destination where uint8 bytes to be copied
    source of uint16 bytes
    number of bytes to be copied
 
RETURNS
    void
*/
void memcpy_unpack(uint8* dest, const uint16* src, uint16 length);

/***************************************************************************
NAME
    memcpy_pack
 
DESCRIPTION
    Utility function to copy uint8 packed bytes into uint16 bytes
 
PARAMS
    destination where uint16 bytes to be copied
    source of uint8 bytes
    number of bytes to be copied
 
RETURNS
    void
*/
void memcpy_pack(uint16* dest, uint8* source, uint16 length);

#endif /* _SINK_UTILS_H_ */
