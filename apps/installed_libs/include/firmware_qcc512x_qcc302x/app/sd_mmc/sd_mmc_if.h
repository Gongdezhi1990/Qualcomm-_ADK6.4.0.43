/* Copyright (c) 2017 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
    sd_mmc_if.h

CONTAINS
    Definitions for the SD-MMC subsystem.

DESCRIPTION
    This file is seen by the stack/CDA devices, and customer applications, and
    contains things that are common between them.
*/

/*!
 @file sd_mmc_if.h
*/

#ifndef __SD_MMC_IF_H__
#define __SD_MMC_IF_H__

/*! @brief Returned values from SdMmcDeviceInfo() */
typedef struct
{
    /*! An 8-bit binary number that identifies the card manufacturer. The MID
     * number is controlled, defined, and allocated to a SD Memory Card
     * manufacturer by the SD-3C, LLC. This procedure is established to ensure
     * uniqueness of the CID register.
     */
    uint8  manuf_id;

    /*! A 2-character ASCII string that identifies the card OEM and/or the card
     * contents (when used as a distribution media either on ROM or FLASH cards)
     * The OID number is controlled, defined, and allocated to a SD Memory Card
     * manufacturer by the SD-3C, LLC. This procedure is established to ensure
     * uniqueness of the CID register.
     */
    uint8  oem_id[2];

    /*! The product name is a 5-character ASCII string. */
    uint8  product_name[6];

    /*! The product revision is composed of two Binary Coded Decimal (BCD)
     * digits, four bits each, representing an "n.m" revision number. The "n" is
     * the most significant nibble and "m" is the least significant nibble.
     * As an example, the PRV binary value field for product revision "6.2" will
     * be: 0110 0010b
     */
    uint8  product_revision;

    /*! The Serial Number is 32 bits of binary number. */
    uint8  product_serial[4];

    /*! The manufacturing date is composed of two hexadecimal digits, one is
     * 8 bits representing the year(y) and the other is 4 bits representing the
     * month (m).
     * The "m" field [11:8] is the month code. 1 = January.
     * The "y" field [19:12] is the year code. 0 = 2000.
     * As an example, the binary value of the Date field for production date
     * April 2001 will be: 00000001 0100.
     */
    uint16 manuf_date;

    /*! Device size as number of 512-byte blocks */
    uint32 device_size;
} sd_mmc_dev_info;
#endif /* __SD_MMC_IF_H__ */

