/****************************************************************************
Copyright (c) 2016-2017 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_host_spi.h

DESCRIPTION
    Implementation of an AHI transport using the BCCMDs that work over SPI.

*/
/*!
@file   ahi_host_spi.h
@brief  Implementation of an AHI transport using the BCCMDs that work over SPI.

        This is a VM application module that acts as the 'glue' between
        The new traps for BCCMDs that carry data to the VM and the AHI transport API.
*/
#ifndef AHI_SPI_HOST_H_
#define AHI_SPI_HOST_H_

#if defined ENABLE_AHI_SPI
void AhiSpiHostInit(void);
#else
#define AhiSpiHostInit() ((void)0)
#endif /* ENABLE_AHI_SPI */

#endif /* AHI_SPI_HOST_H_ */
