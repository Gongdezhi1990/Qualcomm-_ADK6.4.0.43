/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_host_if_data.h
    
DESCRIPTION
    Interface to the module which handles protocol message communications
    with the host.
    
    A set of functions for building and sending protocol message from the
    device to the host.
    
    A generic function for handling incoming protocol messages from the host,
    building an internal message, and passing it to the host interface client.
    Currently that client is the upgrade state machine.
*/
#ifndef UPGRADE_HOST_IF_DATA_H_
#define UPGRADE_HOST_IF_DATA_H_

#include <message.h>
#include "upgrade_msg_host.h"

/*!
    Definition of version number.

    Sent by the device using SYNC_CFM command.
 */
typedef enum
{
    PROTOCOL_VERSION_1 = 1,
    PROTOCOL_VERSION_2,
    PROTOCOL_VERSION_3
    
} ProtocolVersion;


/*!
    Definition the current protocol version in use.
 */
#define PROTOCOL_CURRENT_VERSION    PROTOCOL_VERSION_3

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataSendShortMsg(UpgradeMsgHost message);

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataSendSyncCfm(uint16 status, uint32 id);

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataSendStartCfm(uint16 status, uint16 batteryLevel);

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataSendBytesReq(uint32 numBytes, uint32 startOffset);

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataSendErrorInd(uint16 errorCode);

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataSendIsCsrValidDoneCfm(uint16 backOffTime);

/*!
    @brief Build and send an UPGRADE_VERSION_CFM message to the host.

    @param major_ver Current Upgrade Major version number.
    @param minor_ver Current Upgrade Minor version number.
    @param ps_config_ver Current PS Configuration version number.

    @return void
 */
void UpgradeHostIFDataSendVersionCfm(const uint16 major_ver,
                                     const uint16 minor_ver,
                                     const uint16 ps_config_ver);

/*!
    @brief Build and send an UPGRADE_VARIANT_CFM message to the host.

    @param variant 8 character string with device variant details. 

    @return void
 */
void UpgradeHostIFDataSendVariantCfm(const char *variant);

/*!
    @brief TODO
    @return void
 */
void UpgradeHostIFDataBuildIncomingMsg(Task clientTask, uint8 *data, uint16 dataSize);

#endif /* UPGRADE_HOST_IF_DATA_H_ */
