/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef AGHFP_MISC_HANDLER_H_
#define AGHFP_MISC_HANDLER_H_


/****************************************************************************
 HS has sent us a keypad command. Profile handler has asked
 us to act on this request.
*/
void aghfpHandleKeypadCommand(AGHFP *aghfp, uint16 keycode);

/****************************************************************************
 HF wants to enable/disable NR and EC on the AG. Profile handler has asked
 us to act on this request.
*/
void aghfpHandleNrecSetupReq(AGHFP *aghfp, bool enable);


/****************************************************************************
 HF wants to enable/disable voice recognition in the AG. Profile handler has
 asked us to act on this request.
*/
void aghfpHandleVoiceRecognitionSetupReq(AGHFP *aghfp, bool enable);


/****************************************************************************
 AG wants to tell the HF to enable/disable its voice activation. Profile
 handler has asked us to act on this request.
*/
void aghfpHandleVoiceRecognitionEnable(AGHFP *aghfp, bool enable);


/****************************************************************************
 HF wants to know what phone number corresponds to the voice print it has
 just recorded. This message causes the AG to go into a "phone number
 selection" mode, so that the user can select the number that corresponds to
 the voice print he/she has just recorded.

 Profile handler has asked us to act on this request.
*/
void aghfpHandlePhoneNumberForVoiceTagReq(AGHFP *aghfp);


/****************************************************************************
 AG wants to send a phone number to the HF in response to an AT+BINP request.
 Profile handler has asked us to handle this.
*/
void aghfpHandleSendPhoneNumberForVoiceTag(AGHFP *aghfp, uint16 size_number, uint8 *number);


/****************************************************************************
 AG wants to send speaker volume information to the HF. Profile handler has
 asked us to act on this request.
*/
void aghfpHandleSetRemoteSpeakerVolume(AGHFP *aghfp, uint8 volume);


/****************************************************************************
 AG wants to send microphone gain information to the HF. Profile handler has
 asked us to act on this request.
*/
void aghfpHandleSetRemoteMicrophoneGain(AGHFP *aghfp, uint8 gain);


/****************************************************************************
 HF has sent us its microphone gain value. We need to acknowledge the
 message with OK and send the gain value to the app.
*/
void aghfpHandleSyncMicrophoneGain(AGHFP *aghfp, uint8 gain);


/****************************************************************************
 HF has sent us its speaker volume value. We need to acknowledge the
 message with OK and send the volume value to the app.
*/
void aghfpHandleSyncSpeakerVolume(AGHFP *aghfp, uint8 volume);


/****************************************************************************
 AG wants to send microphone gain information to the HF. Profile handler has
 asked us to act on this request.
*/
void aghfpHandleTransmitDtmfCode(AGHFP *aghfp, uint8 code);

void aghfpHandleResponseHoldStatusRequest(AGHFP *aghfp);
void aghfpHandleSetResponseHoldStatus(AGHFP *aghfp, uint8 cmd);
void aghfpHandleSendResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state);
void aghfpHandleConfirmResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state);

void aghfpHandleSubscriberNumbersRequest(AGHFP *aghfp);
void aghfpHandleSendSubscriberNumber(AGHFP *aghfp, AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER_T *res);
void aghfpHandleSendSubscriberNumbersComplete(AGHFP *aghfp);

void aghfpHandleCurrentCallsRequest(AGHFP *aghfp);
void aghfpHandleSendCurrentCall(AGHFP *aghfp, AGHFP_INTERNAL_SEND_CURRENT_CALL_T *res);
void aghfpHandleSendCurrentCallsComplete(AGHFP *aghfp);

void aghfpHandleNetworkOperatorFormatRequest(AGHFP *aghfp, uint8 mode, uint8 format);
void aghfpHandleNetworkOperatorRequest(AGHFP *aghfp);
void aghfpHandleSendNetworkOperator(AGHFP *aghfp, uint8 mode, uint16 size_operator, uint8* operator);

#endif /* AGHFP_MISC_HANDLER_H_ */
