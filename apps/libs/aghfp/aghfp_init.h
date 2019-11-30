/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#ifndef AGHFP_INIT_H_
#define AGHFP_INIT_H_


#include "aghfp_private.h"


/****************************************************************************
	Send internal init req messages until we have completed the profile
	lib initialisation.
*/
void aghfpHandleInternalInitReq(AGHFP *aghfp);


/****************************************************************************
	Send an internal init cfm message.
*/
void aghfpSendInternalInitCfm(Task task, aghfp_init_status s, uint8 c);


/****************************************************************************
	This message is sent once various parts of the library initialisation 
	process have completed.
*/
void aghfpHandleInternalInitCfm(AGHFP *aghfp, const AGHFP_INTERNAL_INIT_CFM_T *cfm);

/****************************************************************************
	Handles confirmation registering the AGHFP to receive Synchronous connection
	notifications from the Connection library.
*/
void aghfpHandleSyncRegisterCfm(AGHFP *aghfp);


/****************************************************************************
	Reset all the connection related state in this function.
*/
void aghfpResetConnectionRelatedState(AGHFP *aghfp);


#endif /* AGHFP_INIT_H_ */
