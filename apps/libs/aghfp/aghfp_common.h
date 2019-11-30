/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#ifndef AGHFP_COMMON_H_
#define AGHFP_COMMON_H_

#include "aghfp.h"
#include "aghfp_private.h"

/****************************************************************************
 Update the aghfp state.
*/
void aghfpSetState(AGHFP *aghfp, aghfp_state state);


/*****************************************************************************
 Create a common cfm message (many AGHFP defined messages sent to the app
 have the form of the message below and a common function can be used to
 allocate them). Send the message not forgetting to set the correct 
 message id.
*/
void aghfpSendCommonCfmMessageToApp(uint16 message_id, AGHFP *aghfp, aghfp_lib_status status);


bool supportedProfileIsHfp(aghfp_profile profile);
bool supportedProfileHasHfp15Features(aghfp_profile profile);
bool supportedProfileHasHfp17Features(aghfp_profile profile);
bool supportedProfileIsHsp(aghfp_profile profile);

codec_mode_bit_type aghfpConvertCodecModeIdToBit(codec_mode_id_type mode_id);
codec_mode_id_type  aghfpConvertCodecModeBitToId(codec_mode_bit_type mode_bit);



#endif /* AGHFP_COMMON_H_ */
