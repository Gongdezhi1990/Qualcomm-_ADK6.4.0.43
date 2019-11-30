/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    anc_licence_check.c

DESCRIPTION

*/

#include <csrtypes.h>
#include <feature.h>
#include "anc_licence_check.h"

bool ancLicenceCheckIsAncLicenced(void)
{
    return (FeatureVerifyLicense(ANC_FEED_FORWARD)
                || FeatureVerifyLicense(ANC_FEED_BACK)
                || FeatureVerifyLicense(ANC_HYBRID)
                || FeatureVerifyLicense(ANC_FEED_FORWARD_EARBUD)
                || FeatureVerifyLicense(ANC_FEED_BACK_EARBUD)
                || FeatureVerifyLicense(ANC_HYBRID_EARBUD));
}
