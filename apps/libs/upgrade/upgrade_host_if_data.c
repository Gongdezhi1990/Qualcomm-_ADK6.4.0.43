/****************************************************************************
Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_host_if_data.c

DESCRIPTION
    Implementation of the module which handles protocol message communications
    between host and device.

NOTES

*/

#include <stdlib.h>
#include <string.h>

#include <panic.h>
#include <byte_utils.h>
#include <print.h>

#include "upgrade_host_if.h"
#include "upgrade_host_if_data.h"

#define SHORT_MSG_SIZE (sizeof(uint8) + sizeof(uint16))

void UpgradeHostIFDataSendShortMsg(UpgradeMsgHost message)
{
    uint16 byteIndex;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE);
    if(!payload)
    {
        return;
    }

    byteIndex = 0;
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, message - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, 0);

    UpgradeHostIFClientSendData(payload, byteIndex);
}

void UpgradeHostIFDataSendSyncCfm(uint16 status, uint32 id)
{
    uint16 byteIndex;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_SYNC_CFM_T));
    if(!payload)
    {
        return;
    }

    PRINT(("UpgradeHostIFDataSendSyncCfm status %d, id 0x%lx, version %d\n", status, id, PROTOCOL_CURRENT_VERSION));

    byteIndex = 0;
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_SYNC_CFM - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_SYNC_CFM_BYTE_SIZE);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, (uint8)status);
    byteIndex += ByteUtilsSet4Bytes(payload, byteIndex, id);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, PROTOCOL_CURRENT_VERSION);

    UpgradeHostIFClientSendData(payload, byteIndex);
}

void UpgradeHostIFDataSendStartCfm(uint16 status, uint16 batteryLevel)
{
    uint16 byteIndex;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_START_CFM_T));
    if(!payload)
    {
        return;
    }

    PRINT(("UpgradeHostIFDataSendStartCfm status %d, batLevel 0x%x\n", status, batteryLevel));

    byteIndex = 0;
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_START_CFM - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_START_CFM_BYTE_SIZE);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, (uint8)status);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, batteryLevel);

    UpgradeHostIFClientSendData(payload, byteIndex);
}

void UpgradeHostIFDataSendBytesReq(uint32 numBytes, uint32 startOffset)
{
    uint16 byteIndex;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_DATA_BYTES_REQ_T));
    if(!payload)
    {
        return;
    }

    PRINT(("UpgradeHostIFDataSendBytesReq numBytes %ld, startOffset 0x%lx\n", numBytes, startOffset));

    byteIndex = 0;
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_DATA_BYTES_REQ - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_DATA_BYTES_REQ_BYTE_SIZE);
    byteIndex += ByteUtilsSet4Bytes(payload, byteIndex, numBytes);
    byteIndex += ByteUtilsSet4Bytes(payload, byteIndex, startOffset);

    {
        uint8 i;
        for(i = 0; i < (SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_DATA_BYTES_REQ_T)); ++i)
        {
            PRINT(("payload[%d] 0x%x\n", i, payload[i]));
        }
    }

    UpgradeHostIFClientSendData(payload, byteIndex);
}

void UpgradeHostIFDataSendErrorInd(uint16 errorCode)
{
    uint16 byteIndex;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_ERRORWARN_IND_T));
    if(!payload)
    {
        return;
    }

    PRINT(("UpgradeHostIFDataSendErrorInd errorCode 0x%x\n", errorCode));

    byteIndex = 0;
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_ERRORWARN_IND - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_ERRORWARN_IND_BYTE_SIZE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, errorCode);

    UpgradeHostIFClientSendData(payload, byteIndex);
}

void UpgradeHostIFDataSendIsCsrValidDoneCfm(uint16 backOffTime)
{
    uint16 byteIndex;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_IS_CSR_VALID_DONE_CFM_T));
    if(!payload)
    {
        return;
    }

    PRINT(("UpgradeHostIFDataSendIsCsrValidDoneCfm backOffTime 0x%x\n", backOffTime));

    byteIndex = 0;
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_IS_CSR_VALID_DONE_CFM - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_IS_CSR_VALID_DONE_CFM_BYTE_SIZE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, backOffTime);

    UpgradeHostIFClientSendData(payload, byteIndex);
}

void UpgradeHostIFDataBuildIncomingMsg(Task clientTask, uint8 *data, uint16 dataSize)
{
    uint16 msgId;
    uint16 length;

    PRINT(("UpgradeHostIFDataBuildIncomingMsg size %d, id 0x%x\n", dataSize, dataSize ? data[0] + UPGRADE_HOST_MSG_BASE : 0));

    if(dataSize < SHORT_MSG_SIZE || data == NULL)
    {
        return;
    }

    PRINT(("UpgradeHostIFDataBuildIncomingMsg size %d, id 0x%x\n", dataSize, data[0] + UPGRADE_HOST_MSG_BASE));

    msgId = ByteUtilsGet1ByteFromStream(data);
    PRINT(("msgId 0x%x\n", msgId));
    msgId += UPGRADE_HOST_MSG_BASE;

    switch(msgId)
    {
    case UPGRADE_HOST_SYNC_REQ:
        {
            MESSAGE_MAKE(msg, UPGRADE_HOST_SYNC_REQ_T);
            length = ByteUtilsGet2BytesFromStream(&data[1]);
            msg->inProgressId = ByteUtilsGet4BytesFromStream(&data[3]);
            PRINT(("UPGRADE_HOST_SYNC_REQ len %d inProgressId 0x%lx\n", length, msg->inProgressId));
            MessageSend(clientTask, msgId, msg);
        }
        break;

    case UPGRADE_HOST_START_REQ:
    case UPGRADE_HOST_START_DATA_REQ:
    case UPGRADE_HOST_ABORT_REQ:
    case UPGRADE_HOST_PROGRESS_REQ:
    case UPGRADE_HOST_IS_CSR_VALID_DONE_REQ:
    case UPGRADE_HOST_SYNC_AFTER_REBOOT_REQ:
    case UPGRADE_HOST_VERSION_REQ:
    case UPGRADE_HOST_VARIANT_REQ:
    case UPGRADE_HOST_ERASE_SQIF_CFM:
        PRINT(("No data message\n"));
        MessageSend(clientTask, msgId, 0);
        break;

    case UPGRADE_HOST_DATA:
        {
            UPGRADE_HOST_DATA_T *msg;
            length = ByteUtilsGet2BytesFromStream(&data[1]);
            if(length < 2)
            {
                PRINT(("UPGRADE_HOST_DATA len %d error\n", length));
                break;
            }
            msg = (UPGRADE_HOST_DATA_T *)PanicUnlessMalloc(sizeof(UPGRADE_HOST_DATA_T) + length-2);
            msg->length = length-1;
            msg->moreData = ByteUtilsGet1ByteFromStream(&data[3]);
            /*ByteUtilsMemCpyFromStream((uint8 *)msg->data, &data[4], length-1);*/
            memcpy((uint8 *)&msg->data[0], &data[4], length-1);

            MessageSend(clientTask, msgId, msg);
        }
        break;

    case UPGRADE_HOST_TRANSFER_COMPLETE_RES:
    case UPGRADE_HOST_IN_PROGRESS_RES:
    case UPGRADE_HOST_COMMIT_CFM:
        {
            MESSAGE_MAKE(msg, UPGRADE_HOST_GENERIC_ACTION_T);
            PRINT(("UPGRADE_HOST_GENERIC_ACTION\n"));
            msg->action = ByteUtilsGet1ByteFromStream(&data[3]);
            MessageSend(clientTask, msgId, msg);
        }
        break;

    case UPGRADE_HOST_ERRORWARN_RES:
        {
            MESSAGE_MAKE(msg, UPGRADE_HOST_ERRORWARN_RES_T);
            msg->errorCode = ByteUtilsGet2BytesFromStream(&data[3]);
            PRINT(("UPGRADE_HOST_ERRORWARN_RES 0x%x\n", msg->errorCode));
            MessageSend(clientTask, msgId, msg);
        }
        break;

    default:
        PRINT(("unhandled msg\n"));
    }
}

/*
NAME
    UpgradeHostIFDataSendVersionCfm - Send an UPGRADE_VERSION_CFM message to the host.

DESCRIPTION
    Build an UPGRADE_VERSION_CFM protocol message, containing the current major and 
    minor upgrade version numbers, and the current PS config version, and send it to
    the host.
 */
void UpgradeHostIFDataSendVersionCfm(const uint16 major_ver, const uint16 minor_ver, const uint16 ps_config_ver)
{
    uint16 byteIndex = 0;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_VERSION_CFM_T));

    PRINT(("UpgradeHostIFDataSendVersionCfm Major %d, Minor %d PS %d\n", major_ver, minor_ver, ps_config_ver));
    
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_VERSION_CFM - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_VERSION_CFM_BYTE_SIZE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, major_ver);    
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, minor_ver);    
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, ps_config_ver);    

    UpgradeHostIFClientSendData(payload, byteIndex);
}

/*
NAME
    UpgradeHostIFDataSendVariantCfm - Sned an UPGRADE_VARIANT_CFM message to the host

DESCRIPTION
    Build an UPGRADE_VARIANT_CFM protocol message, containing the 8 byte device variant
    identifier, and send it to the host.
 */
void UpgradeHostIFDataSendVariantCfm(const char *variant)
{
    uint16 byteIndex = 0;
    uint8 *payload = PanicUnlessMalloc(SHORT_MSG_SIZE + sizeof(UPGRADE_HOST_VARIANT_CFM_T));

    PRINT(("UpgradeHostIFDataSendVariantCfm variant %s\n", variant));
    
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, UPGRADE_HOST_VARIANT_CFM - UPGRADE_HOST_MSG_BASE);
    byteIndex += ByteUtilsSet2Bytes(payload, byteIndex, UPGRADE_HOST_VARIANT_CFM_BYTE_SIZE);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[0]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[1]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[2]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[3]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[4]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[5]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[6]);
    byteIndex += ByteUtilsSet1Byte(payload, byteIndex, variant[7]);

    UpgradeHostIFClientSendData(payload, byteIndex);
}
