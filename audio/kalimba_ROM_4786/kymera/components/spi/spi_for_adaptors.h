/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup spi SPI low-level communication
 * \file spi.h
 * \ingroup spi
 *
 * SPI module public header file.
 */

#ifndef _SPI_FOR_ADAPTORS_H_
#define _SPI_FOR_ADAPTORS_H_

/* an old default size in case messages come in without a block alloc command first */
#define SPI_MSG_PAYLOAD_SIZE            64

/* routing code for SPI message - when multiple transports exist for certain 
 * adaptors, then this will be more abstrac, i.e. as commented in OBPM adaptor, too,
 * transport and adaptor-related routing/identity will be divorced from each other. 
 */
/* For Kymera dual-core this needs to keep bits 7,6,5 unused to fit in connection id. */
#define RESPOND_TO_SPI                  0x1F

#endif /* _SPI_FOR_ADAPTORS_H_ */
