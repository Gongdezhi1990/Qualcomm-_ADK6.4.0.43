/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_call_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_CALL_HANDLER_H_
#define HFP_CALL_HANDLER_H_


/****************************************************************************
NAME    
    hfpHandleCallRingIndication

DESCRIPTION
    Handle any call state changes required when RING is received on link

RETURNS
    void
*/
void hfpHandleCallRingIndication(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleCallDialComplete

DESCRIPTION
    Handle any call state changes required when dial functions complete

RETURNS
    void
*/
void hfpHandleCallDialComplete(hfp_link_data* link, hfp_lib_status status);


/****************************************************************************
NAME    
    hfpHandleCallVoiceRecDisabled

DESCRIPTION
    Handle any call state changes required when voice recognition disabled

RETURNS
    void
*/
void hfpHandleCallVoiceRecDisabled(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleCallAudio

DESCRIPTION
    Handle any call state changes required when audio is connected on link

RETURNS
    void
*/
void hfpHandleCallAudio(hfp_link_data* link, bool audio);


/****************************************************************************
NAME    
    hfpHandleCallIncomingTimeout

DESCRIPTION
    Handle HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0/1_IND

RETURNS
    void
*/
void hfpHandleCallIncomingTimeout(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleCallIndication

DESCRIPTION
    Handle a call notification on link

RETURNS
    void
*/
void hfpHandleCallIndication(hfp_link_data* link, hfp_call call);


/****************************************************************************
NAME    
    hfpHandleCallSetupIndication

DESCRIPTION
    Handle a call setup notification on link

RETURNS
    void
*/
void hfpHandleCallSetupIndication(hfp_link_data* link, hfp_call_setup call_setup);


/****************************************************************************
NAME    
    hfpHandleCallHeldIndication

DESCRIPTION
    Handle a held call notification on link

RETURNS
    void
*/
void hfpHandleCallHeldIndication(hfp_link_data* link, hfp_call_hold call_hold);


/****************************************************************************
NAME    
    hfpHandleCurrentCallsInd

DESCRIPTION
    Handle a current calls notification on link

RETURNS
    void
*/
void hfpHandleCallCurrentIndication(hfp_link_data* link, hfp_call_multiparty multiparty);


/****************************************************************************
NAME    
    hfpHandleCallResponseHoldIndication

DESCRIPTION
    Handle a response and hold notification on link (+BTRH)

RETURNS
    void
*/
void hfpHandleCallResponseHoldIndication(hfp_link_data* link, hfp_response_hold_state state);


/****************************************************************************
NAME    
    hfpHandleCallMultiparty

DESCRIPTION
    If the link doesn't support CLCC then we can only guess at multiparty
    state based on the headset sending AT+CHLD=3. 

RETURNS
    void
*/
void hfpHandleCallMultiparty(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleAnswerCall

DESCRIPTION
    Answer an incoming call.

RETURNS
    void
*/
bool hfpHandleAnswerCall(const HFP_INTERNAL_AT_ANSWER_REQ_T* req);

/****************************************************************************
NAME    
    hfpHandleAnswerHspCall

DESCRIPTION
    Answer an incoming call from HSP AG.

RETURNS
    void
*/
bool hfpHandleAnswerHspCall(const HFP_INTERNAL_AT_ANSWER_REQ_T* req);


/****************************************************************************
NAME    
    hfpHandleTerminateCall

DESCRIPTION
    Terminate an ongoing call process.

RETURNS
    void
*/
bool hfpHandleTerminateCall(hfp_link_data* link, hfp_at_cmd pending_cmd);


/****************************************************************************
NAME    
    hfpHandleTerminateHspCall

DESCRIPTION
    Terminate an ongoing call process.

RETURNS
    void
*/
bool hfpHandleTerminateHspCall(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleChupAtAck

DESCRIPTION
    Work out the confirmation message to send on receiving ACK from AT+CHUP

RETURNS
    void
*/
MessageId hfpHandleChupAtAck(hfp_link_data* link);


#endif /* HFP_CALL_HANDLER_H_ */
