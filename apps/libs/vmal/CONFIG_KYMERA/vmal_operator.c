/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal_operator.c

DESCRIPTION
    Operator shim
*/

#include <vmal.h>
#include <operator.h>
#include <panic.h>
#include <stdlib.h>

#ifndef UNUSED
#define UNUSED(x) ((void)x)
#endif

#define MIN_RECEIVE_LEN 10



Operator VmalOperatorCreate(uint16 cap_id)
{
    return OperatorCreate(cap_id, 0, NULL);
}

Operator VmalOperatorCreateWithKeys(uint16 capability_id, vmal_operator_keys_t* keys, uint16 num_keys)
{
    return OperatorCreate(capability_id, num_keys, (OperatorCreateKeys *)keys);
}

bool VmalOperatorFrameworkEnableMainProcessor(bool enable)
{
    return OperatorFrameworkEnable(enable ? MAIN_PROCESSOR_ON : MAIN_PROCESSOR_OFF);
}

bool VmalOperatorFrameworkEnableSecondProcessor(bool enable)
{
    return OperatorFrameworkEnable(enable ? SECOND_PROCESSOR_ON : SECOND_PROCESSOR_OFF);
}

/*
 * send_len and rec_len are the send and receive message lengths measured in uint16's
 * The Macro SIZEOF_MESSAGE() should be used to calculate these values to ensure
 * cross platform compatibility
 * */

bool VmalOperatorMessage(Operator opid, const void * send_msg, uint16 send_len, 
                                        void * recv_msg, uint16 recv_len)
{
    void* receive_message = recv_msg;
    uint16 receive_length = recv_len;
    bool result;
    
    if(0 == recv_len)
    {
        /* We always provide a response pointer for the time being */
        receive_message = PanicUnlessMalloc(MIN_RECEIVE_LEN * sizeof(uint16));
#ifdef OPERATOR_MESSAGE_MUST_RECEIVE
        /* CSRA68100 currently requires the response length be set */
        receive_length = MIN_RECEIVE_LEN;
#endif
    }
    
    result = OperatorMessage(opid, (const uint16 *)send_msg, send_len, (uint16 *)receive_message, receive_length);
    
    if(0 == recv_len)
    {
        free(receive_message);
    }
    return result;
}
