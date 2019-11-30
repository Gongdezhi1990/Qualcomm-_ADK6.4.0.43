/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_profile_handler.h
    
DESCRIPTION
    Header file for the HFP profile library.
*/

#ifndef HFP_PROFILE_HANDLER_H_
#define HFP_PROFILE_HANDLER_H_


/****************************************************************************
NAME    
    hfpProfileHandler

DESCRIPTION
    All messages for this profile lib are handled by this function

RETURNS
    void
*/
void hfpProfileHandler(Task task, MessageId id, Message message);


#endif /* HFP_PROFILE_HANDLER_H_ */
