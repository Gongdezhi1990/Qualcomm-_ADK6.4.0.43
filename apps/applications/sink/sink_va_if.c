/*******************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd.
Part of 6.2

FILE NAME
    sink_va_if.c

DESCRIPTION
    Interface to the selected voice assistant

NOTES

*/

#ifdef ENABLE_BISTO
#include "sink2bisto.h"
#endif
#include "sink_va_if.h"
#include "sink_ama.h"
#include <vmtypes.h>

#ifdef ENABLE_VOICE_ASSISTANT

/*
    Where more than one assistant's handler function is called the appropriate
    assistant is currently selected by not defining the function of the unused
    voice assistant in the header. Mutual exclusivity is enforced by the project
    settings.
*/


/*****************************************************************************/
void SinkVaTriggerTaptoTalk(void)
{
    SinkAmaTriggerTaptoTalk();
}

/*****************************************************************************/
void SinkVaUsrCancelSession(void)
{
    SinkAmaVoiceStop();

}

/*****************************************************************************/
void SinkVaTriggerPushToTalkStart(void)
{
    SinkAmaTriggerPushtoTalk();
}

/*****************************************************************************/
void SinkVaTriggerPushToTalkStop(void)
{
    SinkAmaTriggerPushtoTalkRelease();
}

/*****************************************************************************/
void SinkVaCancelSession(void)
{
    SinkAmaHFPCallInd();
}

/*****************************************************************************/
void SinkVaResumeSession(void)
{
    SinkAmaResumeSession();
}

/*****************************************************************************/
void SinkVaInit(void)
{
    SinkAmaInit();
}

/*****************************************************************************/
void SinkVaHandleLinkLoss(void)
{
    SinkAmaResetTransportAfterLinkLoss();
}

#endif /* ENABLE_VOICE_ASSISTANT*/

#ifdef ENABLE_BISTO
sinkVaEventHandler_t stored_callback = NULL;

void sinkVaRegisterEventHandler(sinkVaEventHandler_t callback)
{
    stored_callback = callback;
}

bool sinkVaEventHandler(sinkEvents_t sink_event)
{
    bool handled = FALSE;
    
    if (stored_callback != NULL)
    {
        handled = stored_callback(sink_event);
    }

    return handled;
}

#endif  /* !ENABLE_BISTO */
