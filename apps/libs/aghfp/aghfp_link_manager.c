/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_link_manager.c
    
DESCRIPTION
    Handles the link mode information for Secure connection.
	
*/

#include "aghfp.h"
#include "aghfp_private.h"


/*! @brief The link whether SC or non-SC is decided as a result of link encrypted after pairing.
                 This function will set the link mode accordingly in AGHFP library.

    @param aghfp A pointer to the profile instance.
    @param secure_link Whether the link is secure or not.
*/
void AghfpLinkSetLinkMode(AGHFP *aghfp, bool secure_link)
{
    /* Set the link secure connection */
    if(aghfp)
    {
       aghfp->link_mode_secure = secure_link;
    }
}

