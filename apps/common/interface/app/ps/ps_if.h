/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        ps_if.h  -  PS Interface

CONTAINS
        Interface elements for PS subsystem

DESCRIPTION
        This file is seen by the stack, and VM applications, and
        contains elements that are common between them.
*/

#ifndef APP_PS_IF_H__
#define APP_PS_IF_H__

typedef enum
{
    ps_store_default        = 0x0,
    ps_store_implementation = 0x1,
    ps_store_factory        = 0x2,
    ps_store_rom            = 0x4,
    ps_store_transient      = 0x8,
    ps_store_application    = 0x10
} PsStores;

#define PSKEY_ID_MAX 0x29FF

#define PSKUSRB ((650))
#define PSKEY_USR0 (PSKUSRB+0)
#define PSKEY_USR49 (PSKUSRB+49)

#define PSKEXTENSION ((0x2000))
#define PSKDSPB ((PSKEXTENSION + 600))
#define PSKEY_DSP0 (PSKDSPB+0)
#define PSKEY_DSP49 (PSKDSPB+49)

#define PSKCLB ((PSKEXTENSION + 1550))
#define PSKEY_CONNLIB0 (PSKCLB+0)
#define PSKEY_CONNLIB49 (PSKCLB+49)

#define PSKUSRB1 (PSKEXTENSION + 1950)
#define PSKEY_USR50 (PSKUSRB1+0)
#define PSKEY_USR99 (PSKUSRB1+49)

#define PSKCUSTB (PSKEXTENSION + 2000)
#define PSKEY_CUSTOMER0 (PSKCUSTB+0)
#define PSKEY_CUSTOMER89 (PSKCUSTB+89)
/* Indices 90-99 are hijacked for READONLYx which are really MIBs */
#define PSKEY_CUSTOMER90 (PSKCUSTB+100)
#define PSKEY_CUSTOMER299 (PSKCUSTB+309)

/* Temperature and Charger MIB Keys */
#define PSKSYSB4 ((PSKEXTENSION + 1400))
#define PSKEY_CHARGER_MAXIMUM_VOLTAGE (PSKSYSB4+99)
#define PSKEY_THERMAL_SHUTDOWN_PERIOD (PSKSYSB4+112)
#define PSKEY_SHUTDOWN_TEMPERATURE (PSKSYSB4+113)
#define PSKEY_THERMAL_SHUTDOWN_LONG_PERIOD (PSKSYSB4+114)

/* start of pskey range for providing access to apps mib. 
 * Picked high number to prevent clash with Bluetooth pskeys. 
 */
#define PSKAPPSMIB_START (32000)
#define PSKEY_RAMPDOWN_TEMPERATURE (PSKAPPSMIB_START+0)
#define PSKEY_REENABLE_TEMPERATURE (PSKAPPSMIB_START+1)
#define PSKEY_CHARGER_MAXIMUM_CURRENT_TRICKLE (PSKAPPSMIB_START+2)
#define PSKEY_CHARGER_MAXIMUM_CURRENT_PRE (PSKAPPSMIB_START+3)
#define PSKEY_CHARGER_MAXIMUM_CURRENT_FAST (PSKAPPSMIB_START+4)

#define PSKAPPSMIB_END PSKEY_CHARGER_MAXIMUM_CURRENT_FAST

#define PSKEY_MAX_APPKEY_INDEX 509
/* 290-299 are repurposed as PSKEY_READONLY0-9 */
#define IS_APPKEY_READONLY(appkey) (appkey >= 290 && appkey <= 299)

/* The following keys are those that have traditionally been read
   by VMapplications.

   Comment in the published ps_if.h says that these are normally
   "a selection of values from psbc_keys.h published to the VM" */

#define PSKEY_HOST_INTERFACE (((500))+5)
#define PSKEY_MKT_TASK_ACTIVE (((500))+6)
#define PSKEY_IPV4_STATIC_ADDR (((850))+3)
#define PSKEY_IPV4_STATIC_SUBNET_MASK (((850))+6)
#define PSKEY_MDNS_NAME (((850))+8)
#define PSKEY_FIXED_PIN (((850))+9)
#define PSKEY_PIO_WAKEUP_STATE (((900))+27)

/* USB MIB Keys */
#define MIB_USB_BCD_DEVICE (125)
#define PSKHOSTIOUSBB (700)
#define MIB_USB_VERSION (PSKHOSTIOUSBB+0)
#define MIB_USB_DEVICE_CLASS_CODES (PSKHOSTIOUSBB+1)
#define MIB_USB_VENDOR_ID (PSKHOSTIOUSBB+2)
#define MIB_USB_PRODUCT_ID (PSKHOSTIOUSBB+3)
#define MIB_USB_MANUF_STRING (PSKHOSTIOUSBB+5)
#define MIB_USB_PRODUCT_STRING (PSKHOSTIOUSBB+6)
#define MIB_USB_SERIAL_NUMBER_STRING (PSKHOSTIOUSBB+7)
#define MIB_USB_CONFIG_STRING (PSKHOSTIOUSBB+8)
#define MIB_USB_MAX_POWER (PSKHOSTIOUSBB+9)
#define MIB_USB_LANGID (PSKHOSTIOUSBB+13)

#define PSKAPPSUSBMIB_START (PSKHOSTIOUSBB)
#define PSKAPPSUSBMIB_END (MIB_USB_LANGID)

/* Block of READONLY Keys reserved for the customer's use */ 
#define PSKEY_READONLY0 (PSKCUSTB+90)
#define PSKEY_READONLY1 (PSKCUSTB+91)
#define PSKEY_READONLY2 (PSKCUSTB+92)
#define PSKEY_READONLY3 (PSKCUSTB+93)
#define PSKEY_READONLY4 (PSKCUSTB+94)
#define PSKEY_READONLY5 (PSKCUSTB+95)
#define PSKEY_READONLY6 (PSKCUSTB+96)
#define PSKEY_READONLY7 (PSKCUSTB+97)
#define PSKEY_READONLY8 (PSKCUSTB+98)
#define PSKEY_READONLY9 (PSKCUSTB+99)

#define PSKREADONLYMIB_START (PSKEY_READONLY0)
#define PSKREADONLYMIB_END (PSKEY_READONLY9)

#endif  /* APP_PS_IF_H__ */
