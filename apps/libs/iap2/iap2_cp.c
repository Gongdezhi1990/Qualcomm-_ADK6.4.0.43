/****************************************************************************
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_cp.c
DESCRIPTION
   This file contains routines to manage the authentication IC. The IC is documented
   in the MFI specification. The interface to the IC allows for reading and
   writing of eight and sixteen bit registers over I2C and for transferring data to and
   from the device. Typically a challenge is transmitted to the device. The resulting
   signature or the devices X.509 certificate can be read for transmission to phone. A
   large part of this code involves dealing with init,configuration of authentication IC as
   well as timing over the I2C bus.

*/
#include <i2c.h>
#include <sink.h>
#include "iap2_private.h"
#include "iap2_cp.h"
#include "iap2_link.h"

static iap2_cp_data *iap2_cp = NULL;

static bool iap2CpSetReadRegister(uint8 addr)
{
    return (I2cTransfer(IAP2_CP_I2C_ADDRESS, &addr, 1, NULL, 0) == 2);
}

static bool iap2CpReadData(uint16 size, uint8 *buffer)
{
    return (I2cTransfer(IAP2_CP_I2C_ADDRESS, NULL, 0, buffer, size) >= size);
}

static bool iap2CpReadUint8(uint8 *buffer)
{
    return iap2CpReadData(1, buffer);
}

static bool iap2CpReadUint16(uint16 *buffer)
{
    uint8 rx_buffer[2];

    if (iap2CpReadData(2, rx_buffer))
    {
        *buffer = ((uint16)rx_buffer[0] << 8) | (uint16)rx_buffer[1];
        return TRUE;
    }
    else
        return FALSE;
}

static bool iap2CpWriteData(uint16 size, uint8 *data)
{
    return (I2cTransfer(IAP2_CP_I2C_ADDRESS, data, size, NULL, 0) == (size + 1));
}

static bool iap2CpWriteRegister(uint8 reg, uint16 size, uint8 *data)
{
    uint8 *tx_buffer = PanicUnlessMalloc(size + 1);
    bool success = FALSE;

    tx_buffer[0] = reg;

    if (size)
        memmove(&tx_buffer[1], data, size);

    success = iap2CpWriteData(size + 1, tx_buffer);

    free(tx_buffer);
    return success;
}

static bool iap2CpWriteUint8(uint8 reg, uint8 data)
{
    return iap2CpWriteRegister(reg, 1, &data);
}

static bool iap2CpWriteUint16(uint8 reg, uint16 data)
{
    uint8 tx_buffer[2];
    tx_buffer[0] = (data >> 8) & 0xff;
    tx_buffer[1] = data & 0xff;

    return iap2CpWriteRegister(reg, 2, tx_buffer);
}

static void iap2CpInitFail(Task task)
{
    MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_INIT_CFM, iap2_status_coprocessor_fail);
    MessageSend(task, IAP2_INTERNAL_CP_INIT_CFM, msg);
    iap2_cp->client_task = NULL;
    iap2_cp->retry = 0;
}

static void iap2CpReadAuthCertLenFail(Task task)
{
    MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM, iap2_status_coprocessor_fail);
    msg->auth_cert_len = 0;
    MessageSend(task, IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM, msg);
    iap2_cp->client_task = NULL;
    iap2_cp->retry = 0;
}

static void iap2CpReadAuthCertFail(Task task)
{
    MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, iap2_status_coprocessor_fail);
    msg->checksum = 0;
    MessageSend(task, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, msg);
    iap2_cp->client_task = NULL;
    iap2_cp->retry = 0;
}

static void iap2CpWriteChallengeDataFail(Task task)
{
    MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM, iap2_status_coprocessor_fail);
    msg->challenge_rsp_len = 0;
    MessageSend(task, IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM, msg);
    iap2_cp->client_task = NULL;
    iap2_cp->retry = 0;
}

static void iap2CpReadChallengeRspFail(Task task)
{
    MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM, iap2_status_coprocessor_fail);
    msg->checksum = 0;
    MessageSend(task, IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM, msg);
    iap2_cp->client_task = NULL;
    iap2_cp->retry = 0;
}

static void iap2CpHandleSelfTestReq(void)
{
    DEBUG_PRINT_CP(("iAP2 coprocessor self test req retry=%d\n", iap2_cp->retry));

    if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
    {
        /* We retried too many times */
        iap2CpInitFail(iap2_cp->client_task);
    }
    else if (iap2CpWriteUint8(IAP2_CP_REG_SELF_TEST_CONTROL_STATUS, 1))
    {
        /* Write succeeded, move to next step */
        MessageSend(&iap2_cp->task, IAP2_CP_SELF_TEST_CONFIG, NULL);
        iap2_cp->retry = 0;
    }
    else
    {
        /* Write failed, retry */
        MessageSendLater(&iap2_cp->task, IAP2_CP_SELF_TEST_REQ, NULL, IAP2_CP_I2C_RETRY_DELAY);
        iap2_cp->retry++;
    }
}

static void iap2CpHandleSelfTestConfig(void)
{
    DEBUG_PRINT_CP(("iAP2 coprocessor self test config retry=%d\n", iap2_cp->retry));

    if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
    {
        /* We retried too many times */
        iap2CpInitFail(iap2_cp->client_task);
    }
    else if (iap2CpSetReadRegister(IAP2_CP_REG_SELF_TEST_CONTROL_STATUS))
    {
        /* Write succeeded, move to next step */
        MessageSend(&iap2_cp->task, IAP2_CP_SELF_TEST_STATUS, NULL);
        iap2_cp->retry = 0;
    }
    else
    {
        /* Write failed, retry */
        MessageSendLater(&iap2_cp->task, IAP2_CP_SELF_TEST_CONFIG, NULL, IAP2_CP_I2C_RETRY_DELAY);
        iap2_cp->retry++;
    }
}

static void iap2CpHandleSelfTestStatus(void)
{
    uint8 result = 0;
    DEBUG_PRINT_CP(("iAP2 coprocessor self test status retry=%d\n", iap2_cp->retry));

    if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
    {
        /* We retried too many times */
        iap2CpInitFail(iap2_cp->client_task);
    }
    else if (iap2CpReadUint8(&result))
    {
        /* Read succeeded, check result */
        DEBUG_PRINT_CP(("iAP2 coprocessor self test result=%x\n", result));

        if ((result & IAP2_CP_SELF_TEST_SUCCESS_MASK) == IAP2_CP_SELF_TEST_SUCCESS_MASK)
        {
            /* Self test succeeded, init completed */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_INIT_CFM, iap2_status_success);
            MessageSend(iap2_cp->client_task, IAP2_INTERNAL_CP_INIT_CFM, msg);
            iap2_cp->client_task = NULL;
            iap2_cp->retry = 0;
        }
        else
        {
            /* Self test failed */
            iap2CpInitFail(iap2_cp->client_task);
        }
    }
    else
    {
        /* Read failed, retry */
        iap2_cp->retry++;
        MessageSendLater(&iap2_cp->task, IAP2_CP_SELF_TEST_STATUS, NULL, IAP2_CP_I2C_RETRY_DELAY);
    }
}

static void iap2CpHandleReadAuthCertLenReq(IAP2_CP_READ_AUTH_CERT_LEN_REQ_T *req)
{
    if (iap2LinkValidate(req->link))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert len req link=0x%p retry=%d\n", (void*)req->link, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpReadAuthCertLenFail(&req->link->task);
        }
        else if (iap2CpSetReadRegister(IAP2_CP_REG_AUTH_CERT_DATA_LEN))
        {
            /* Write succeeded, move to next step */
            MessageSend(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_LEN_STATUS, NULL);
            iap2_cp->client_task = &req->link->task;
            iap2_cp->retry = 0;
        }
        else
        {
            /* Write failed, retry */
            MAKE_MESSAGE(msg, IAP2_CP_READ_AUTH_CERT_LEN_REQ);
            msg->link = req->link;
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_LEN_REQ, msg, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
}

static void iap2CpHandleReadAuthCertLenStatus(void)
{
    if (iap2LinkValidate((iap2_link *)iap2_cp->client_task))
    {
        uint16 auth_cert_len = 0;
        DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert len status link=0x%p  retry=%d\n", (void*)iap2_cp->client_task, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpReadAuthCertLenFail(iap2_cp->client_task);
        }
        else if (iap2CpReadUint16(&auth_cert_len))
        {
            /* Read succeeded, notify client task */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM, iap2_status_success);
            msg->auth_cert_len = auth_cert_len;;
            MessageSend(iap2_cp->client_task, IAP2_INTERNAL_CP_READ_AUTH_CERT_LEN_CFM, msg);
            iap2_cp->client_task = NULL;
            iap2_cp->retry = 0;

            DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert completed len=%d\n", auth_cert_len));
        }
        else
        {
            /* Read failed, retry */
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_LEN_STATUS, NULL,IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleReadAuthCertReq(IAP2_CP_READ_AUTH_CERT_REQ_T *req)
{
    if (iap2LinkValidate(req->link))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert req link=0x%p len=%d retry=%d\n", (void*)req->link, req->auth_cert_len, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpReadAuthCertFail(&req->link->task);
        }
        else if (iap2CpSetReadRegister(IAP2_CP_REG_AUTH_CERT_DATA))
        {
            /* Write succeeded, move to next step */
            MAKE_MESSAGE(msg, IAP2_CP_READ_AUTH_CERT_STATUS);
            msg->pending_len = req->auth_cert_len;
            MessageSend(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_STATUS, msg);
            iap2_cp->client_task = &req->link->task;
            iap2_cp->retry = 0;
            iap2_cp->checksum = req->checksum;
        }
        else
        {
            /* Write failed, retry */
            MAKE_MESSAGE(msg, IAP2_CP_READ_AUTH_CERT_REQ);
            msg->link = req->link;
            msg->auth_cert_len = req->auth_cert_len;
            msg->checksum = req->checksum;
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_REQ, msg, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
}

static void iap2CpHandleReadAuthCertStatus(const IAP2_CP_READ_AUTH_CERT_STATUS_T *stat)
{
    iap2_link *link = (iap2_link *)iap2_cp->client_task;

    if (iap2LinkValidate(link))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert status link=0x%p retry=%d pending=%d\n", (void*)iap2_cp->client_task, iap2_cp->retry, stat->pending_len));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpReadAuthCertFail(iap2_cp->client_task);
        }
        else
        {
            uint16 pending_len = stat->pending_len;
            uint8 *buffer = MALLOC(MIN(pending_len, IAP2_CP_I2C_MAX_TRANSFER_SIZE));
            uint16 read_len;

            while (buffer && pending_len)
            {
                read_len = MIN(pending_len, IAP2_CP_I2C_MAX_TRANSFER_SIZE);

                if (SinkSlack(link->sink) >= read_len && iap2CpReadData(read_len, buffer))
                {
                    /* Read succeeded, calculate checksum */
                    iap2_cp->checksum += iap2Checksum(buffer, read_len);

                    /* Write data to Sink */
                    memmove(SinkMap(link->sink) + SinkClaim(link->sink, read_len), buffer, read_len);
                    pending_len -= read_len;
                }
                else
                    break;
            }

            /* Check if we read any data this time */
            if (stat->pending_len != pending_len)
            {
                SinkFlush(link->sink, stat->pending_len - pending_len);
                iap2_cp->retry = 0;
            }
            else
                iap2_cp->retry++;

            if (pending_len)
            {
                /* We have more data to read, schedule the next message */
                MAKE_MESSAGE(msg, IAP2_CP_READ_AUTH_CERT_STATUS);
                msg->pending_len = pending_len;
                MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_STATUS, msg, IAP2_CP_I2C_RETRY_DELAY);
            }
            else
            {
                /* We have no more data to read, notify the client task */
                MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, iap2_status_success);
                msg->checksum = iap2_cp->checksum & 0xff;
                DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert completed checksum=%x\n", msg->checksum));
                MessageSend(iap2_cp->client_task, IAP2_INTERNAL_CP_READ_AUTH_CERT_CFM, msg);
                iap2_cp->client_task = NULL;
                iap2_cp->retry = 0;

            }

            free(buffer);
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleWriteChallengeLenReq(IAP2_CP_WRITE_CHALLENGE_LEN_REQ_T *req)
{
    if (iap2LinkValidate(req->link))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor write challenge data len req link=0x%p len=%d retry=%d\n",
                     (void*)req->link, req->challenge_data_len, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(&req->link->task);
        }
        else if (iap2CpWriteUint16(IAP2_CP_REG_CHALLENGE_DATA_LEN, req->challenge_data_len))
        {
            /* Write succeeded, move to next step */
            MAKE_MESSAGE_WITH_LEN(msg, IAP2_CP_WRITE_CHALLENGE_DATA_REQ, req->challenge_data_len);
            msg->challenge_data_len = req->challenge_data_len;
            memmove(&msg->challenge_data[0], &req->challenge_data[0], req->challenge_data_len);
            MessageSend(&iap2_cp->task, IAP2_CP_WRITE_CHALLENGE_DATA_REQ, msg);
            iap2_cp->client_task = &req->link->task;
            iap2_cp->retry = 0;
        }
        else
        {
            /* Write failed, retry */
            MAKE_MESSAGE_WITH_LEN(msg, IAP2_CP_WRITE_CHALLENGE_LEN_REQ, req->challenge_data_len);
            msg->link = req->link;
            msg->challenge_data_len = req->challenge_data_len;
            memmove(&msg->challenge_data[0], &req->challenge_data[0], req->challenge_data_len);
            MessageSendLater(&iap2_cp->task, IAP2_CP_WRITE_CHALLENGE_LEN_REQ, msg, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
}

static void iap2CpHandleWriteChallengeDataReq(IAP2_CP_WRITE_CHALLENGE_DATA_REQ_T *req)
{
    iap2_link *link = (iap2_link *)iap2_cp->client_task;

    if (iap2LinkValidate(link))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor write challenge data req link=0x%p len=%d retry=%d\n",
                     (void*)link, req->challenge_data_len, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(iap2_cp->client_task);
        }
        else if (iap2CpWriteRegister(IAP2_CP_REG_CHALLENGE_DATA, req->challenge_data_len, req->challenge_data))
        {
            /* Write succeeded, move to next step */
            MessageSend(&iap2_cp->task, IAP2_CP_WRITE_AUTH_CTRL_REQ, NULL);
            iap2_cp->retry = 0;
        }
        else
        {
            /* Write failed, retry */
            MAKE_MESSAGE_WITH_LEN(msg, IAP2_CP_WRITE_CHALLENGE_DATA_REQ, req->challenge_data_len);
            msg->challenge_data_len = req->challenge_data_len;
            memmove(msg->challenge_data, req->challenge_data, req->challenge_data_len);
            MessageSendLater(&iap2_cp->task, IAP2_CP_WRITE_CHALLENGE_DATA_REQ, msg, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleWriteAuthCtrlReq(void)
{
    if (iap2LinkValidate((iap2_link *)iap2_cp->client_task))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor write auth ctrl req retry=%d\n", iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(iap2_cp->client_task);
        }
        else if (iap2CpWriteUint8(IAP2_CP_REG_AUTH_CONTROL_STATUS, IAP2_CP_AUTH_CTRL_START_CHAL_RSP_GEN))
        {
            /* Write succeeded, allow time for authentication and move to next step */
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_RESULT_REQ, NULL, IAP2_CP_T_AUTH);
            iap2_cp->retry = 0;
        }
        else
        {
            /* Write failed, retry */
            MessageSendLater(&iap2_cp->task, IAP2_CP_WRITE_AUTH_CTRL_REQ, NULL, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleReadAuthResultReq(void)
{
    if (iap2LinkValidate((iap2_link *)iap2_cp->client_task))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor read auth result req link=0x%p retry=%d\n", (void*)iap2_cp->client_task, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(iap2_cp->client_task);
        }
        else if (iap2CpSetReadRegister(IAP2_CP_REG_AUTH_CONTROL_STATUS))
        {
            /* Write succeeded, move to next step */
            MessageSend(&iap2_cp->task, IAP2_CP_READ_AUTH_RESULT_STATUS, NULL);
            iap2_cp->retry = 0;
        }
        else
        {
            /* Write failed, retry in 20ms*/
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_RESULT_REQ, NULL, 20);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleReadAuthResultStatus(void)
{
    if (iap2LinkValidate((iap2_link *)iap2_cp->client_task))
    {
        uint8 result = 0;
        DEBUG_PRINT_CP(("iAP2 coprocessor read auth result status link=0x%p retry=%d\n", (void*)iap2_cp->client_task, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(iap2_cp->client_task);
        }
        else if (iap2CpReadUint8(&result))
        {
            /* Read succeeded, check result */
            DEBUG_PRINT_CP(("iAP2 coprocessor auth result=%x\n", result));

            if ((result & IAP2_CP_AUTH_RESULT_MASK) == IAP2_CP_AUTH_RESULT_CHAL_RSP_GEN_OK)
            {
                /* Challenge response generated, move to next step */
                MessageSend(&iap2_cp->task, IAP2_CP_READ_CHALLENGE_RSP_LEN_REQ, NULL);
                iap2_cp->retry = 0;
            }
            else
            {
                /* Challenge response generated failed */
                iap2CpWriteChallengeDataFail(iap2_cp->client_task);
            }
        }
        else
        {
            /* Read failed, retry */
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_AUTH_RESULT_STATUS, NULL, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}


static void iap2CpHandleReadChallengeRspLenReq(void)
{
    if (iap2LinkValidate((iap2_link *)iap2_cp->client_task))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor read challenge rsp len req link=0x%p retry=%d\n", (void*)iap2_cp->client_task, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(iap2_cp->client_task);
        }
        else if (iap2CpSetReadRegister(IAP2_CP_REG_CHALLENGE_RSP_DATA_LEN))
        {
            /* Write succeeded, move to next step */
            MessageSend(&iap2_cp->task, IAP2_CP_READ_CHALLENGE_RSP_LEN_STATUS, NULL);
            iap2_cp->retry = 0;
        }
        else
        {
            /* Write failed, retry */
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_CHALLENGE_RSP_LEN_REQ, NULL, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleReadChallengeRspLenStatus(void)
{
    if (iap2LinkValidate((iap2_link *)iap2_cp->client_task))
    {
        uint16 challenge_rsp_len = 0;
        DEBUG_PRINT_CP(("iAP2 coprocessor read challenge rsp len status link=0x%p retry=%d\n", (void*)iap2_cp->client_task, iap2_cp->retry));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpWriteChallengeDataFail(iap2_cp->client_task);
        }
        else if (iap2CpReadUint16(&challenge_rsp_len))
        {
            /* Read succeeded, notify client task */
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM, iap2_status_success);
            msg->challenge_rsp_len = challenge_rsp_len;;
            MessageSend(iap2_cp->client_task, IAP2_INTERNAL_CP_WRITE_CHALLENGE_DATA_CFM, msg);
            iap2_cp->retry = 0;

            DEBUG_PRINT_CP(("iAP2 coprocessor read challenge rsp len completed len=%d\n", challenge_rsp_len));
        }
        else
        {
            /* Read failed, retry */
            MessageSendLater(&iap2_cp->task, IAP2_CP_READ_CHALLENGE_RSP_LEN_STATUS, NULL, IAP2_CP_I2C_RETRY_DELAY);
            iap2_cp->retry++;
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpHandleReadChallengeRspStatus(const IAP2_CP_READ_CHALLENGE_RSP_STATUS_T *stat)
{
    iap2_link *link = (iap2_link *)iap2_cp->client_task;

    if (iap2LinkValidate(link))
    {
        DEBUG_PRINT_CP(("iAP2 coprocessor read challenge rsp status link=0x%p retry=%d pending=%d\n", (void*)link, iap2_cp->retry, stat->pending_len));

        if (iap2_cp->retry > IAP2_CP_I2C_MAX_ATTEMPT)
        {
            /* We retried too many times */
            iap2CpReadChallengeRspFail(iap2_cp->client_task);
        }
        else
        {
            uint16 pending_len = stat->pending_len;
            uint8 *buffer = MALLOC(MIN(pending_len, IAP2_CP_I2C_MAX_TRANSFER_SIZE));
            uint16 read_len;

            while (buffer && pending_len)
            {
                read_len = MIN(pending_len, IAP2_CP_I2C_MAX_TRANSFER_SIZE);

                if (SinkSlack(link->sink) >= read_len && iap2CpReadData(read_len, buffer))
                {
                    /* Read succeeded, calculate checksum */
                    iap2_cp->checksum += iap2Checksum(buffer, read_len);

                    /* Write data to Sink */
                    memmove(SinkMap(link->sink) + SinkClaim(link->sink, read_len), buffer, read_len);
                    pending_len -= read_len;
                }
                else
                    break;
            }

            /* Check if we read any data this time */
            if (stat->pending_len != pending_len)
            {
                SinkFlush(link->sink, stat->pending_len - pending_len);
                iap2_cp->retry = 0;
            }
            else
                iap2_cp->retry++;

            if (pending_len)
            {
                /* We have more data to read, schedule the next message */
                MAKE_MESSAGE(msg, IAP2_CP_READ_CHALLENGE_RSP_STATUS);
                msg->pending_len = pending_len;
                MessageSendLater(&iap2_cp->task, IAP2_CP_READ_CHALLENGE_RSP_STATUS, msg, IAP2_CP_I2C_RETRY_DELAY);
            }
            else
            {
                /* We have no more data to read, notify the lib task */
                MAKE_MESSAGE_WITH_STATUS(msg, IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM, iap2_status_success);
                msg->checksum = iap2_cp->checksum & 0xff;
                DEBUG_PRINT_CP(("iAP2 coprocessor read challenge rsp completed checksum=%x\n", msg->checksum));
                MessageSend(&link->task, IAP2_INTERNAL_CP_READ_CHALLENGE_RSP_CFM, msg);


                iap2_cp->client_task = NULL;
                iap2_cp->retry = 0;
            }

            free(buffer);
        }
    }
    else
    {
        iap2_cp->client_task = NULL;
        iap2_cp->retry = 0;
    }
}

static void iap2CpTaskHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
        case IAP2_CP_SELF_TEST_REQ:
        iap2CpHandleSelfTestReq();
        break;

        case IAP2_CP_SELF_TEST_CONFIG:
        iap2CpHandleSelfTestConfig();
        break;

        case IAP2_CP_SELF_TEST_STATUS:
        iap2CpHandleSelfTestStatus();
        break;

        case IAP2_CP_READ_AUTH_CERT_LEN_REQ:
        iap2CpHandleReadAuthCertLenReq((IAP2_CP_READ_AUTH_CERT_LEN_REQ_T *)message);
        break;

        case IAP2_CP_READ_AUTH_CERT_LEN_STATUS:
        iap2CpHandleReadAuthCertLenStatus();
        break;

        case IAP2_CP_READ_AUTH_CERT_REQ:
        iap2CpHandleReadAuthCertReq((IAP2_CP_READ_AUTH_CERT_REQ_T *)message);
        break;

        case IAP2_CP_READ_AUTH_CERT_STATUS:
        iap2CpHandleReadAuthCertStatus((const IAP2_CP_READ_AUTH_CERT_STATUS_T *)message);
        break;

        case IAP2_CP_WRITE_CHALLENGE_LEN_REQ:
        iap2CpHandleWriteChallengeLenReq((IAP2_CP_WRITE_CHALLENGE_LEN_REQ_T *)message);
        break;

        case IAP2_CP_WRITE_CHALLENGE_DATA_REQ:
        iap2CpHandleWriteChallengeDataReq((IAP2_CP_WRITE_CHALLENGE_DATA_REQ_T *)message);
        break;

        case IAP2_CP_WRITE_AUTH_CTRL_REQ:
        iap2CpHandleWriteAuthCtrlReq();
        break;

        case IAP2_CP_READ_AUTH_RESULT_REQ:
        iap2CpHandleReadAuthResultReq();
        break;

        case IAP2_CP_READ_AUTH_RESULT_STATUS:
        iap2CpHandleReadAuthResultStatus();
        break;

        case IAP2_CP_READ_CHALLENGE_RSP_LEN_REQ:
        iap2CpHandleReadChallengeRspLenReq();
        break;

        case IAP2_CP_READ_CHALLENGE_RSP_LEN_STATUS:
        iap2CpHandleReadChallengeRspLenStatus();
        break;

        case IAP2_CP_READ_CHALLENGE_RSP_STATUS:
        iap2CpHandleReadChallengeRspStatus((const IAP2_CP_READ_CHALLENGE_RSP_STATUS_T *)message);
        break;

        default:
        DEBUG_PRINT_CP(("iap2CpTaskHandler unknown msg=%04x\n", id));
        break;
    }
}

void iap2CpInit(void)
{
    if (!iap2_cp)
    {

        DEBUG_PRINT_CP(("iAP2 coprocessor init\n"));

        iap2_cp = &iap2_lib->cp;
        iap2_cp->task.handler = iap2CpTaskHandler;
        iap2_cp->client_task = &iap2_lib->task;
        iap2_cp->retry = 0;
        
        /* Wait for coprocessor to initialise */
        MessageSendLater(&iap2_cp->task, IAP2_CP_SELF_TEST_REQ, NULL, IAP2_CP_INIT_DELAY);
    }
}

void iap2CpReadAuthCertLen(iap2_link *link)
{
    MAKE_MESSAGE(msg, IAP2_CP_READ_AUTH_CERT_LEN_REQ);

    DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert len link=0x%p\n", (void*)link));

    msg->link = link;
    MessageSendConditionallyOnTask(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_LEN_REQ, msg, &iap2_cp->client_task);
}

void iap2CpReadAuthCert(iap2_link *link, uint16 auth_cert_len, uint8 checksum)
{
    MAKE_MESSAGE(msg, IAP2_CP_READ_AUTH_CERT_REQ);

    DEBUG_PRINT_CP(("iAP2 coprocessor read auth cert link=0x%p len=%d checksum=%x\n", (void*)link, auth_cert_len, checksum));

    msg->link = link;
    msg->auth_cert_len = auth_cert_len;
    msg->checksum = checksum;
    MessageSendConditionallyOnTask(&iap2_cp->task, IAP2_CP_READ_AUTH_CERT_REQ, msg, &iap2_cp->client_task);
}

void iap2CpWriteChallengeData(iap2_link *link, uint16 challenge_data_len, uint8 *challenge_data)
{
    /* First write challenge data length */
    MAKE_MESSAGE_WITH_LEN(msg, IAP2_CP_WRITE_CHALLENGE_LEN_REQ, challenge_data_len);

    DEBUG_PRINT_CP(("iAP2 coprocessor write challenge data len=%d link=0x%p\n", challenge_data_len, (void*)link));

    msg->link = link;
    msg->challenge_data_len = challenge_data_len;
    memmove(&msg->challenge_data[0], challenge_data, challenge_data_len);
    MessageSendConditionallyOnTask(&iap2_cp->task, IAP2_CP_WRITE_CHALLENGE_LEN_REQ, msg, &iap2_cp->client_task);
}

void iap2CpReadChallengeRsp(iap2_link *link, uint16 challenge_rsp_len, uint8 checksum)
{
    /* This function call will alwasys followed by iap2CpWriteChallengeData */
    if (iap2_cp->client_task == &link->task)
    {
        MAKE_MESSAGE(msg, IAP2_CP_READ_CHALLENGE_RSP_STATUS);

        DEBUG_PRINT_CP(("iAP2 coprocessor read challenge response data len=%d link=0x%p\n", challenge_rsp_len, (void*)link));

        msg->pending_len = challenge_rsp_len;
        iap2_cp->checksum = checksum;
        MessageSend(&iap2_cp->task, IAP2_CP_READ_CHALLENGE_RSP_STATUS, msg);
    }
}

