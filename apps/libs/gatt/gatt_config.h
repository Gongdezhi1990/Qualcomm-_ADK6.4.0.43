/*************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_config.h

DESCRIPTION
    Configuration of the GATT library.

NOTES
*/

#ifndef _GATT_CONFIG_H
#define _GATT_CONFIG_H

/*
  GATT feature selection

  There are 11 features defined in the GATT Profile:
  
  1. Server Configuration
      1.1 Exchange MTU
  2. Primary Service Discovery
      2.1 Discover All Primary Services
      2.2 Discover Primary Services by Service UUID
  3. Relationship Discovery
      3.1 Find Included Services
  4. Characteristic Discovery
      4.1 Discover All Characteristics of a Service
      4.2 Discover Characteristic by UUID
  5. Characteristic Descriptor Discovery
      5.1 Discover All Characteristic Descriptors
  6. Reading a Characteristic Value
      6.1 Read Characteristic Value
      6.2 Read Using Characteristic UUID
      6.3 Read Long Characteristic Value
      6.4 Read Multiple Characteristic Values
  7. Writing a Characteristic Value
      7.1 Write Without Response
      7.2 Signed Write Without Response
      7.3 Write Characteristic Value
      7.4 Write Long Characteristic Value
      7.5 Characteristic Valueable Reliable Write
  8. Notification of a Characteristic Value
      8.1 Notification
  9. Indication of a Characteristic Value
      9.1 Indication
  10. Reading a Characteristic Descriptor
      10.1 Read Characteristic Descriptor
      10.2 Read Long Characteristic Descriptor
  11. Writing a Characteristic Descriptor
      11.1 Write Characteristic Descriptor
      11.2 Write Long Characteristic Descriptor

  Each of the features is mapped to procedures and sub-procedures.

  Each of the sub-procedures can be independently compiled into the GATT
  library using GATT_FEATURE bitmask.
  
 */
/* gatt_server_config.c */
#define GATT_EXCHANGE_MTU              0x00000001
#define GATT_SERVER_CONFIG             (GATT_EXCHANGE_MTU)

/* gatt_primary_discovery.c */
#define GATT_DISC_ALL_PRIMARY_SERVICES 0x00000002
#define GATT_DISC_PRIMARY_SERVICE      0x00000004
#define GATT_PRIMARY_DISCOVERY         (GATT_DISC_ALL_PRIMARY_SERVICES | \
                                        GATT_DISC_PRIMARY_SERVICE)

/* gatt_relationship_discovery.c */
#define GATT_FIND_INCLUDED_SERVICES     0x00000008
#define GATT_RELATIONSHIP_DISCOVERY     (GATT_FIND_INCLUDED_SERVICES)

/* gatt_characteristic_discovery.c */
#define GATT_DISC_ALL_CHARACTERISTICS   0x00000010
#define GATT_DISC_CHARACTERISTIC        0x00000020
#define GATT_CHARACTERISTIC_DISCOVERY   (GATT_DISC_ALL_CHARACTERISTICS | \
                                         GATT_DISC_CHARACTERISTIC)

/* gatt_descriptor_discovery.c */
#define GATT_DISC_ALL_DESCRIPTORS       0x00000040
#define GATT_DESCRIPTOR_DISCOVERY       (GATT_DISC_ALL_DESCRIPTORS)

/* gatt_characteristic_read.c */
#define GATT_READ                       0x00000080
#define GATT_READ_BY_UUID               0x00000100
#define GATT_READ_LONG                  0x00000200
#define GATT_READ_MULTIPLE              0x00000400
#define GATT_CHARACTERISTIC_READ        (GATT_READ |         \
                                         GATT_READ_BY_UUID | \
                                         GATT_READ_LONG |    \
                                         GATT_READ_MULTIPLE)

/* gatt_characteristic_write.c */
#define GATT_WRITE_COMMAND              0x00000800
#define GATT_WRITE_SIGNED               0x00001000
#define GATT_WRITE                      0x00002000
#define GATT_WRITE_LONG                 0x00004000
#define GATT_WRITE_RELIABLE             0x00008000
#define GATT_CHARACTERISTIC_WRITE       (GATT_WRITE_COMMAND |   \
                                         GATT_WRITE_SIGNED |    \
                                         GATT_WRITE |           \
                                         GATT_WRITE_LONG |      \
                                         GATT_WRITE_RELIABLE)

/* gatt_notification_indication.c */
#define GATT_NOTIFICATION               0x00010000
#define GATT_INDICATION                 0x00020000 /* Mandatory */

/* gatt_descriptor_read.c */
#define GATT_READ_DESCRIPTOR            0x00040000
#define GATT_READ_LONG_DESCRIPTOR       0x00080000
#define GATT_DESCRIPTOR_READ            (GATT_READ_DESCRIPTOR |         \
                                         GATT_READ_LONGDESCRIPTOR)

/* gatt_descriptor_write.c */
#define GATT_WRITE_DESCRIPTOR           0x00100000
#define GATT_WRITE_LONG_DESCRIPTOR      0x00200000
#define GATT_DESCRIPTOR_WRITE           (GATT_WRITE_DESCRIPTOR |        \
                                         GATT_WRITE_LONG_DESCRIPTOR)

/* GATT features to compile in */
#define GATT_FEATURES                   (GATT_SERVER_CONFIG |           \
                                         GATT_PRIMARY_DISCOVERY |       \
                                         GATT_RELATIONSHIP_DISCOVERY |  \
                                         GATT_CHARACTERISTIC_DISCOVERY | \
                                         GATT_DESCRIPTOR_DISCOVERY |    \
                                         GATT_CHARACTERISTIC_READ |     \
                                         GATT_CHARACTERISTIC_WRITE |    \
                                         GATT_NOTIFICATION |            \
                                         GATT_INDICATION |              \
                                         GATT_DESCRIPTOR_READ |         \
                                         GATT_DESCRIPTOR_WRITE)

#endif /* _GATT_CONFIG_H */
