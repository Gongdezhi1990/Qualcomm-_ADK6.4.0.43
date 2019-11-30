/****************************************************************************
Copyright (c) 2011 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    gatt_caching.h       

DESCRIPTION
    This file contains the type definitions and function prototypes for 
    GATT Caching that need to be shared between the GATT and Connection
    libraries.
*/
#ifndef GATT_CACHING_H_
#define GATT_CACHING_H_

/* GATT-related attribute data. */
typedef struct _gatt_content_t
{
    unsigned    addr_type:2;
    unsigned    trusted:1;
    unsigned    change_aware:1;
    unsigned    robust_caching:1;
    unsigned    indications_enabled:1;
    unsigned    unused:10;
} gatt_content_t;

/* GATT related attribute data - This deliberately mirrors the td_data_t struct
 * above; since the only difference is the content field, it can simply be 
 * typecast from it.
 */
typedef struct _gatt_td_data_t
{
    uint16              bdaddr[3];
    gatt_content_t      content;
    uint16              data[1];
} gatt_td_data_t;


#endif  /* GATT_CACHING_H_ */
