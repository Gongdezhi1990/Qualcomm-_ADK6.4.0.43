/*
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

/*!
@file    sink_hid_device_service_record.h
@brief   HID SDP service record
*/

#ifndef HID_DEVICE_SERVICE_RECORD_H
#define HID_DEVICE_SERVICE_RECORD_H

/* length of HID descriptor */
#define HID_HEADSET_REMOTE_DESCRIPTOR_LEN     33
#define HID_HEADSET_REMOTE_REPORT_LEN     1

#define HID_HEADSET_REMOTE_REPORT_DESCRIPTOR     \
    0x05, 0x0C,  /* USAGE PAGE (Consumer Device) */ \
    0x09, 0x01,  /* USAGE (Consumer Control) */ \
    0xA1, 0x01, /* COLLECTION (Application) */\
        0x15, 0x00, /* LOGICAL MINIMUM (0) */\
        0x25, 0x01, /* LOGICAL MAXIMUM (1) */\
        0x75, 0x01, /* REPORT SIZE (1) */\
        0x95, 0x02, /* REPORT COUNT (2) */\
        0x09, 0xE9, /* USAGE (Volume Up) */\
        0x09, 0xEA, /* USAGE (Volume Down) */\
        0x81, 0x02, /* INPUT (data, Var, Abs) */\
        0x95, 0x01, /* REPORT COUNT (1) */\
        0x09, 0x21, /* USAGE (Center) */\
        0x81, 0x02, /* INPUT (data, Var, Abs) */\
        0x75, 0x05, /* REPORT SIZE (5) */\
        0x95, 0x01, /* REPORT COUNT (1) */\
        0x81, 0x03, /* INPUT (Cnst, Var, Abs) */\
    0xc0        /* END COLLECTION */

#define HID_DEVICE_SERVICE_RECORD_HEADER                      \
  0x09, /* ServiceClassIDList(0x0001) */                      \
    0x00,                                                                      \
    0x01,                                                                      \
  0x35, /* DataElSeq 3 bytes */                                    \
  0x03,                                                                        \
    0x19, /* uuid HID(0x1124) */                                   \
    0x11,                                                                      \
    0x24,                                                                      \
  0x09, /* ProtocolDescriptorList(0x0004) */                  \
    0x00,                                                                      \
    0x04,                                                                      \
  0x35, /* DataElSeq 13 bytes */                                  \
  0x0d,                                                                        \
    0x35, /* DataElSeq 6 bytes */                                  \
    0x06,                                                                      \
      0x19, /* uuid L2CAP(0x0100) */                             \
      0x01,                                                                    \
      0x00,                                                                    \
      0x09, /* uint16 PSM HID Control(0x0011) */           \
        0x00,                                                                  \
        0x11,                                                                  \
    0x35, /* DataElSeq 3 bytes */                                  \
    0x03,                                                                      \
      0x19, /* uuid HIDP(0x0011) */                               \
      0x00,                                                                    \
      0x11,                                                                    \
  0x09, /* LanguageAttributeIDList(0x0006) */               \
    0x00,                                                                      \
    0x06,                                                                      \
  0x35, /* DataElSeq 9 bytes */                                    \
  0x09,                                                                        \
    0x09, /* uint16 0x656e  */                                        \
      0x65,                                                                    \
      0x6e,                                                                    \
    0x09, /* uint16 0x006a */                                        \
      0x00,                                                                    \
      0x6a,                                                                    \
    0x09, /* uint16 0x0100 */                                        \
      0x01,                                                                    \
      0x00,                                                                    \
  0x09, /* AdditionalProtocolDescriptorList(0x000d) */    \
    0x00,                                                                      \
    0x0d,                                                                      \
  0x35, /* DataElSeq 15 bytes */                                  \
  0x0f,                                                                        \
    0x35, /* DataElSeq 12 bytes */                                \
    0x0d,                                                                      \
      0x35, /* DataElSeq 6 bytes */                                \
      0x06,                                                                    \
        0x19, /* uuid L2CAP(0x0100) */                           \
        0x01,                                                                  \
        0x00,                                                                  \
        0x09, /* uint16 PSM HID Interrupt(0x0013) */       \
          0x00,                                                                \
          0x13,                                                                \
      0x35, /* DataElSeq 3 bytes */                                \
      0x03,                                                                    \
        0x19, /* uuid HIDP(0x0011) */                             \
        0x00,                                                                  \
        0x11,                                                                 \
      0x09, /* ServiceName(0x0100) = "Headset" */      \
    0x01,                                                                    \
    0x00,                                                                    \
  0x25, /* String length 11 */                                     \
  0x07,                                                                      \
    'H','e','a','d','s','e','t'

#define HID_DEVICE_SERVICE_RECORD_FOOTER       \
  0x09, /* BluetoothProfileDescriptorList(0x0009) */       \
    0x00,                                                                      \
    0x09,                                                                      \
  0x35, /* DataElSeq 8 bytes */                                    \
  0x08,                                                                        \
    0x35, /* DataElSeq 6 bytes */                                  \
    0x06,                                                                      \
      0x19, /* uuid HID(0x1124) */                                 \
      0x11,                                                                    \
      0x24,                                                                    \
      0x09, /* uint16 version number (v1.1)*/                 \
        0x01,                                                                  \
        0x01,                                                                  \
  0x09, /* HIDParserVersion(0x0201) = "0x0111" */      \
    0x02,                                                                      \
    0x01,                                                                      \
  0x09, /* uint16 0x0111 */                                          \
    0x01,                                                                      \
    0x11,                                                                      \
  0x09, /* HIDDeviceSubclass(0x0202) = Headset "0x04" */   \
    0x02,                                                                      \
    0x02,                                                                      \
  0x08, /* uint8 0x04 */                                               \
    0x04,                                                                      \
  0x09, /* HIDCountryCode(0x0203) = "0x21" */                                  \
    0x02,                                                                      \
    0x03,                                                                      \
  0x08, /* uint8 USA = 0x21 */                                                 \
    0x21,                                                                      \
  0x09, /* HIDVirtualCable(0x0204) = "false" */                                 \
    0x02,                                                                      \
    0x04,                                                                      \
  0x28, /* bool false */                                                        \
    0x00,                                                                      \
  0x09, /* HIDReconnectInitiate(0x0205) = "false" */                            \
    0x02,                                                                      \
    0x05,                                                                      \
  0x28, /* bool false */                                                        \
    0x00,                                                                      \
  0x09, /* HID Descriptor List (0x0206) */                                     \
    0x02,                                                                      \
    0x06,                                                                      \
  0x35, /* DataElSeq HID_HEADSET_REPORT_DESCRIPTOR_LEN+6 Bytes */                  \
  (HID_HEADSET_REMOTE_DESCRIPTOR_LEN + 6),                                         \
    0x35, /* DataElSeq HID_HEADSET_REPORT_DESCRIPTOR_LEN+4 Bytes */                \
    (HID_HEADSET_REMOTE_DESCRIPTOR_LEN + 4),                                       \
      0x08, /* Report Descriptor(0x0822) */                                    \
        0x22,                                                                  \
      0x25, /* String length HID_HEADSET_REPORT_DESCRIPTOR_LEN */                  \
      HID_HEADSET_REMOTE_DESCRIPTOR_LEN,                                           \
        HID_HEADSET_REMOTE_REPORT_DESCRIPTOR,                                             \
  0x09, /* HIDLANGIDBaseList(0x0207) */                                        \
    0x02,                                                                      \
    0x07,                                                                      \
  0x35, /* DataElSeq 8 bytes */                                                \
  0x08,                                                                        \
    0x35, /* DataElSeq 6 bytes */                                              \
    0x06,                                                                      \
      0x09, /* uint16 0x0409 */                                                \
        0x04,                                                                  \
        0x09,                                                                  \
      0x09, /* uint16 0x0100 */                                                \
        0x01,                                                                  \
        0x00,                                                                  \
  0x09, /* HIDSDPDisable(0x0208) = "false" */             \
    0x02,                                                                     \
    0x08,                                                                     \
  0x28, /* bool false */                                               \
    0x00,                                                                     \
  0x09, /* HIDBatteryPower(0x0209) = "true" */            \
    0x02,                                                                      \
    0x09,                                                                      \
  0x28, /* bool true */                                                        \
    0x01,                                                                      \
  0x09, /* HIDRemoteWake(0x020a) = "true" */            \
    0x02,                                                                      \
    0x0a,                                                                      \
  0x28, /* bool true */                                                       \
    0x01,                                                                      \
  0x09, /* HIDSupervisionTimeout(0x020c) = "0x0c80" */                         \
    0x02,                                                                      \
    0x0c,                                                                      \
  0x09, /* uint16 0x0c80 */                                                    \
    0x0c,                                                                      \
    0x80,                                                                      \
  0x09, /* HIDNormallyConnectable(0x020d) = "false" */                         \
    0x02,                                                                      \
    0x0d,                                                                      \
  0x28, /* bool false */                                                       \
    0x00,                                                                      \
  0x09, /* HIDBootDevice(0x020e) = "false" */                                  \
    0x02,                                                                      \
    0x0e,                                                                      \
  0x28, /* bool false */                                                       \
    0x00,                                                                      \
  0x09, /* HIDSSRHostMaxLatency(0x020f) = "0x02d0" */                          \
    0x02,                                                                      \
    0x0f,                                                                      \
  0x09, /* uint16 0x02d0 */                                                    \
    0x02,                                                                      \
    0xd0,                                                                      \
  0x09, /* HIDSSRHostMinTimeout(0x0210) = "0x0048" */                          \
    0x02,                                                                      \
    0x10,                                                                      \
  0x09, /* uint16 0x0048 */                                                    \
    0x00,                                                                      \
    0x48

#endif
