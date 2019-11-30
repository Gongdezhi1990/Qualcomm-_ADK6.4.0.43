/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_trb.h

DESCRIPTION
    Implementation of an AHI transport using the Transaction Bridge.

*/
/*!
@file   ahi_host_trb.h
@brief  Implementation of an AHI transport using the using the Transaction Bridge.

        This is a VM application module that acts as the 'glue' between
        the host interface traps and the AHI transport API.
*/
#ifndef AHI_HOST_TRB_H_
#define AHI_HOST_TRB_H_

#ifdef ENABLE_AHI_TRB
void AhiTrbHostInit(void);
#else
#define AhiTrbHostInit() ((void)0)
#endif /* ENABLE_AHI_TRB */

#endif /* AHI_HOST_TRB_H_ */
