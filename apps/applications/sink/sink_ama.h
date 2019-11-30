/*******************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd.

*/

/*!
@file    sink_ama.h
@brief   Interface File to manage code for AVS
*/

#ifndef SINK_AMA_H
#define SINK_AMA_H

#include <csrtypes.h>

/*!
    @brief Initialise the Voice Assistant Application module

    @param None

    @return None
*/
#ifdef ENABLE_AMA
void SinkAmaInit(void);
#else
#define SinkAmaInit() ((void)(0))
#endif

/*!
    @brief Triggers the event to start the button tap VA session

    @param None

    @return None
*/
#ifdef ENABLE_AMA
void SinkAmaTriggerTaptoTalk(void);
#else
#define SinkAmaTriggerTaptoTalk() ((void)(0))
#endif

/*!
    @brief Triggers the event to start push to talk VA session

    @param None

    @return None
*/
#ifdef ENABLE_AMA
void SinkAmaTriggerPushtoTalk(void);
#else
#define SinkAmaTriggerPushtoTalk() ((void)(0))
#endif

/*!
    @brief Triggers the event to send release push to talk VA session

    @param None

    @return None
*/
#ifdef ENABLE_AMA
void SinkAmaTriggerPushtoTalkRelease(void);
#else
#define SinkAmaTriggerPushtoTalkRelease() ((void)(0))
#endif

/*!
    @brief Checks if Ama is being advertised.

    @return True if enabled
*/
#ifdef ENABLE_AMA
bool SinkAmaIsAdvertiseAmaEnabled(void);
#else
#define SinkAmaIsAdvertiseAmaEnabled() (FALSE)
#endif

/*!
    @brief Function to notify AMA of an HFP Call Indication
*/
#ifdef ENABLE_AMA
void SinkAmaHFPCallInd(void);
#else
#define SinkAmaHFPCallInd() ((void)(0))
#endif

/*!
    @brief Function to Resume AMA after an HFP Call End
*/
#ifdef ENABLE_AMA
void SinkAmaResumeSession(void);
#else
#define SinkAmaResumeSession() ((void)(0))
#endif

/*!
    @brief Function to STOP AMA Session 
*/
#ifdef ENABLE_AMA
void SinkAmaVoiceStop(void);
#else
#define SinkAmaVoiceStop() ((void)(0))
#endif

/*!
    @brief Function to reset AMA RFCOMM link on disconnect
*/
#ifdef ENABLE_AMA
void SinkAmaResetLink(const bdaddr* bd_addr);
#else
#define SinkAmaResetLink(x) ((void)(0))
#endif

/*!
    @brief Function to reset AMA Transport Parse Status after Link Loss
*/
#ifdef ENABLE_AMA
void SinkAmaResetTransportAfterLinkLoss(void);
#else
#define SinkAmaResetTransportAfterLinkLoss(x) ((void)(0))
#endif

#endif /* SINK_AMA_H */
