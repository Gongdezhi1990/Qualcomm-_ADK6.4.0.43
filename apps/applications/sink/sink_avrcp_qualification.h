/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   Interface to the PTS qualification for AVRCP qualification, which includes:
            - AVRCP TG CAT1 mandatory functionality and AVRCP CT CAT2 passthrough command.
*/

#ifndef _SINK_AVRCP_QUALIFICATION_H_
#define _SINK_AVRCP_QUALIFICATION_H_

#include <message.h>
#include <app/message/system_message.h>
#include <avrcp.h>


#ifdef ENABLE_AVRCP


/* Utility function to handle all the TG CAT-1 mandatory request indications */
bool handleAvrcpQualificationTestCaseInd (Task task, MessageId id, Message message);

/* Utility function to handle all the TG CAT-1 mandatory register of notifications */
bool handleAvrcpQualificationRegisterNotificationInd(const AVRCP_REGISTER_NOTIFICATION_IND_T  *msg);

/* Utility function to check the "volume up" passthrough command capabilities of CAT-2 CT*/
void handleAvrcpQualificationVolumeUp (void);

/* Utility function to check the "volume down" passthrough command capabilities of CAT-2 CT*/
void handleAvrcpQualificationVolumeDown (void);

/* Utility function to set the status that a track is selected and playing. This is used as an intermediate step while testing 
 CAT-1 TG for EVENT_TRACK_CHANGED notification with CT */
void handleAvrcpQualificationPlayTrack (void);

/* Utility function to send SetAbsoluteVolume for CAT-2 CT*/
void handleAvrcpQualificationSetAbsoluteVolume (void);

/* Utility function to handle VolumeChange Indication from TG CAT-2*/
void handleAvrcpVolumeChangedInd(const AVRCP_EVENT_VOLUME_CHANGED_IND_T *msg);

/* Utility function to handle SetAbsoluteVolumeCfm from TG CAT-2*/
void handleAvrcpSetAbsoluteVolumeCfm(const AVRCP_SET_ABSOLUTE_VOLUME_CFM_T *msg);

/* Utility function to configure AVRCP frame data size*/
void handleAvrcpQualificationConfigureDataSize(void);

/* Utility function to send second avctp connection request.*/
void sinkSendAvctpSecondConnect(void);

#else   /* ENABLE_AVRCP - not enabled, use stubs */

#define sinkAvrcpQualificationInit() ((void)0)
#define handleAvrcpQualificationTestCaseInd(task, id, message) (FALSE)
#define handleAvrcpQualificationRegisterNotificationInd(msg) (FALSE)
#define handleAvrcpQualificationVolumeUp() ((void)0)
#define handleAvrcpQualificationVolumeDown() ((void)0)
#define handleAvrcpQualificationPlayTrack() ((void)0)
#define handleAvrcpQualificationSetAbsoluteVolume() ((void)0)
#define handleAvrcpVolumeChangedInd(msg) ((void)0)
#define handleAvrcpSetAbsoluteVolumeCfm(msg) ((void)0)
#define handleAvrcpQualificationConfigureDataSize() ((void)0)
#define sinkSendAvctpSecondConnect() ((void)0)
#endif  /*ENABLE_AVRCP */

#endif /* _SINK_AVRCP_QUALIFICATION_H_ */

