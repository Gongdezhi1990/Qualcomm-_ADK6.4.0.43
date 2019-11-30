/*
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#ifndef _SINK_CONFIG_H_
#define _SINK_CONFIG_H_

#include <csrtypes.h>

/****************************************************************************
NAME 
    ConfigRetrieve

DESCRIPTION
    This function is called to read a configuration key.  If the key exists
    in persistent store it is read from there.  If it does not exist then
    the default is read from constant space

    The data parameter should have uint16 alignment. malloc blocks will
    meet this requirement.

    The len parameter should match the value given by sizeof(). On the XAP 
    processor this will be in the unit of words.
 
RETURNS
    0 if no data was read otherwise the length of data in words. If comparing
    against a sizeof() then the sizeof() value should be converted to words
    using the PS_SIZE_ADJ() macro
*/
uint16 ConfigRetrieve(uint16 config_id, void* data, uint16 len);

/****************************************************************************
NAME 
    ConfigStore

DESCRIPTION
    This function is called to store a configuration key.  This will always
    be in the PS Store

    The len parameter should match the value given by sizeof(). On the XAP 
    processor this will be in the unit of words. If comparing against a sizeof() 
    then the sizeof() value should be converted to words using the PS_SIZE_ADJ() 
    macro

     The data pointer should be aligned to uint16.

RETURNS
    0 if no data was stored otherwise the length of data in words
*/
uint16 ConfigStore(uint16 config_id, const void* data, uint16 len);

#endif /* _SINK_CONFIG_H_ */
