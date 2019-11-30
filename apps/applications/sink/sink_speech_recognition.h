/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    control the speech recogntion plugin from the device.

    If SR is enabled, the  plugin can be used to answer or reject an 
    incoming call. 
    
*/

#ifndef _SINK_SR_H_
#define _SINK_SR_H_

#include <csrtypes.h>

#ifdef ENABLE_SPEECH_RECOGNITION

#define CSR_SR_APP_TIMEOUT 0x0001
#define CSR_SR_APP_RESTART 0x0002

/****************************************************************************
DESCRIPTION
  	This function is called to enable speech recognition mode
*/
void speechRecognitionStart(void);

/****************************************************************************
DESCRIPTION
  	This function is called to reenable speech recognition mode
*/
void speechRecognitionReStart(void);

/****************************************************************************
DESCRIPTION
  	This function is called to enable speech recognition mode
*/
void speechRecognitionStop(void);


/****************************************************************************
DESCRIPTION
  	This function is called to determine if speech rec is enabled
RETURNS
    True if Speech Rec is enabled
*/
bool speechRecognitionIsEnabled(void) ;

/****************************************************************************
NAME
    speechRecognitionFeatureEnabled

DESCRIPTION
    If SR feature is enabled or not 

PARAMS
    void 

RETURNS
    Bool  TRUE for enabled otherwise FALSE
*/
bool speechRecognitionFeatureEnabled(void);

/****************************************************************************
NAME
    speechRecSetIsActive

DESCRIPTION
    Set if SR is active

PARAMS
    Bool 

RETURNS
    void
*/
void speechRecSetIsActive(bool value);

 /****************************************************************************
NAME
    speechRecognitionTuningIsActive

DESCRIPTION
    This function is called to determine if speech rec tuning is currently active

PARAMS
    void

RETURNS
    True if Speech Rec tuning is active
*/
bool speechRecognitionTuningIsActive(void);

/****************************************************************************
NAME
    speechRecognitionSetTuningActive

DESCRIPTION
    This function sets the speech rec tuning

PARAMS
    Bool
    
RETURNS
    void
*/
void speechRecognitionSetTuningActive(bool value);

/****************************************************************************
NAME
    speechRecognitionIsSSREnabled

DESCRIPTION
    Get if Simple Speech Recognition is enabled or not

PARAMS
    void 

RETURNS
    Bool TRUE for enabled otherwise FALSE
*/
bool speechRecognitionIsSSREnabled(void);


/****************************************************************************
NAME
    speechRecognitionSetSSREnabled

DESCRIPTION
    Set Simple Speech Recognition enabled or not

PARAMS
    Bool 

RETURNS
    void
*/
void speechRecognitionSetSSREnabled(bool value);

/****************************************************************************
NAME
    speechRecognitionGetSessionSSREnabled

DESCRIPTION
    Get Session data for SSR Enabled 
    
PARAMS
    void
    
    bool 
*/
 bool speechRecognitionGetSessionSSREnabled(void);

/****************************************************************************
NAME
    speechRecognitionSetSessionSSREnabled

DESCRIPTION
    Set session data from global data
    
PARAMS
    void

RETURNS
    void 
*/
void speechRecognitionSetSessionSSREnabled(void);

#endif /*ENABLE_SPEECH_RECOGNITION*/

/****************************************************************************
NAME
    speechRecognitionIsActive

DESCRIPTION
    This function is called to determine if speech rec is currently active
    
PARAMS
    void
    
RETURNS
    Bool    True if Speech Rec is active
*/
#ifdef ENABLE_SPEECH_RECOGNITION
bool speechRecognitionIsActive(void) ;
#else
#define speechRecognitionIsActive() (FALSE)
#endif


/****************************************************************************
NAME
    speechRecognitionGetRepeatTime

DESCRIPTION
     This function used to retrive repeat timer

PARAMS
    void 

RETURNS
    uint16
*/
#ifdef ENABLE_SPEECH_RECOGNITION
uint16 speechRecognitionGetRepeatTime(void);
#else
#define speechRecognitionGetRepeatTime() ((uint16)0)
#endif

/****************************************************************************
NAME
    speechRecognitionSetRepeatTime

DESCRIPTION
     This function used to set repeat timer

PARAMS
    uint16

RETURNS
    bool
*/
#ifdef ENABLE_SPEECH_RECOGNITION
bool speechRecognitionSetRepeatTime(uint16 time);
#else
#define speechRecognitionSetRepeatTime(time) (FALSE)
#endif

#endif /* _SINK_SR_H_ */
