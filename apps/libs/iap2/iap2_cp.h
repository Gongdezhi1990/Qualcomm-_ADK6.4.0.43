/*****************************************************************************
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    iap2_cp.h

DESCRIPTION
    Header file for the iAP2 coprocessor APIs.
*/

#ifndef __IAP2_CP_H__
#define __IAP2_CP_H__

#include "iap2_private.h"


#define IAP2_CP_INIT_DELAY                          (10)

#define IAP2_CP_I2C_MAX_ATTEMPT                     (10)
#define IAP2_CP_I2C_MAX_TRANSFER_SIZE               (64)
#define IAP2_CP_I2C_RETRY_DELAY                     (1)
#define IAP2_CP_T_AUTH                              (400)

#ifndef IAP2_CP_I2C_ADDRESS
#define IAP2_CP_I2C_ADDRESS                         (0x20)
#endif

/* Coprocessor registers */
#define IAP2_CP_REG_DEVICE_VERSION                  (0x00)
#define IAP2_CP_REG_FIRMWARE_VERSION                (0x01)
#define IAP2_CP_REG_AUTH_PROTO_VERSION_MAJOR        (0x02)
#define IAP2_CP_REG_AUTH_PROTO_VERSION_MINOR        (0x03)
#define IAP2_CP_REG_DEVICE_ID                       (0x04)
#define IAP2_CP_REG_ERROR_CODE                      (0x05)
#define IAP2_CP_REG_AUTH_CONTROL_STATUS             (0x10)
#define IAP2_CP_REG_CHALLENGE_RSP_DATA_LEN          (0x11)
#define IAP2_CP_REG_CHALLENGE_RSP_DATA              (0x12)
#define IAP2_CP_REG_CHALLENGE_DATA_LEN              (0x20)
#define IAP2_CP_REG_CHALLENGE_DATA                  (0x21)
#define IAP2_CP_REG_AUTH_CERT_DATA_LEN              (0x30)
#define IAP2_CP_REG_AUTH_CERT_DATA                  (0x31)
#define IAP2_CP_REG_SELF_TEST_CONTROL_STATUS        (0x40)
#define IAP2_CP_REG_SYSTEM_EVENT_COUNTER            (0x4D)
#define IAP2_CP_REG_AUTH_CERT_SERIAL_NUMBER         (0x4E)

/* Coprocessor Authentication PROC_CONTROL values */
#define IAP2_CP_AUTH_CTRL_START_CHAL_RSP_GEN        (0x01)
#define IAP2_CP_AUTH_CTRL_START_CHAL_GEN            (0x02)
#define IAP2_CP_AUTH_CTRL_START_CHAL_REP_VERIFY     (0x03)
#define IAP2_CP_AUTH_CTRL_START_CERT_VERIFY         (0x04)

/* Coprocessor Authentication PROC_RESULTS values */
#define IAP2_CP_AUTH_RESULT_MASK                    (0x70)
#define IAP2_CP_AUTH_RESULT_INVALID                 (0x00)
#define IAP2_CP_AUTH_RESULT_CHAL_RSP_GEN_OK         (0x10)
#define IAP2_CP_AUTH_RESULT_CHAL_GEN_OK             (0x20)
#define IAP2_CP_AUTH_RESULT_CHAL_RSP_VERIFY_OK      (0x30)
#define IAP2_CP_AUTH_RESULT_CERT_VALID_OK           (0x40)

#define IAP2_CP_SELF_TEST_RESULT_MASK               (0xF0)
#define IAP2_CP_SELF_TEST_SUCCESS_MASK              (0xC0)

typedef enum
{
    IAP2_CP_SELF_TEST_REQ,
    IAP2_CP_SELF_TEST_CONFIG,
    IAP2_CP_SELF_TEST_STATUS,

    IAP2_CP_READ_AUTH_CERT_LEN_REQ,
    IAP2_CP_READ_AUTH_CERT_LEN_STATUS,

    IAP2_CP_READ_AUTH_CERT_REQ,
    IAP2_CP_READ_AUTH_CERT_STATUS,

    IAP2_CP_WRITE_CHALLENGE_LEN_REQ,
    IAP2_CP_WRITE_CHALLENGE_DATA_REQ,
    IAP2_CP_WRITE_AUTH_CTRL_REQ,
    IAP2_CP_READ_AUTH_RESULT_REQ,
    IAP2_CP_READ_AUTH_RESULT_STATUS,
    IAP2_CP_READ_CHALLENGE_RSP_LEN_REQ,
    IAP2_CP_READ_CHALLENGE_RSP_LEN_STATUS,

    IAP2_CP_READ_CHALLENGE_RSP_STATUS
} Iap2CpMessageId;

typedef struct
{
    iap2_link *link;
} IAP2_CP_READ_AUTH_CERT_LEN_REQ_T;

typedef struct
{
    iap2_link *link;
    uint16 auth_cert_len;
    uint8 checksum;
} IAP2_CP_READ_AUTH_CERT_REQ_T;

typedef struct
{
    uint16 pending_len;
} IAP2_CP_READ_AUTH_CERT_STATUS_T;

typedef struct
{
    iap2_link *link;
    uint16 challenge_data_len;
    uint8 challenge_data[1];
} IAP2_CP_WRITE_CHALLENGE_LEN_REQ_T;

typedef struct
{
    uint16 challenge_data_len;
    uint8 challenge_data[1];
} IAP2_CP_WRITE_CHALLENGE_DATA_REQ_T;

typedef struct
{
    uint16 pending_len;
} IAP2_CP_READ_CHALLENGE_RSP_STATUS_T;

void iap2CpInit(void);
void iap2CpReadAuthCertLen(iap2_link *link);
void iap2CpReadAuthCert(iap2_link *link, uint16 auth_cert_len, uint8 checksum);
void iap2CpWriteChallengeData(iap2_link *link, uint16 challenge_data_len, uint8 *challenge_data);
void iap2CpReadChallengeRsp(iap2_link *link, uint16 challenge_rsp_len, uint8 checksum);

#endif  /* __IAP2_CP_H__ */
