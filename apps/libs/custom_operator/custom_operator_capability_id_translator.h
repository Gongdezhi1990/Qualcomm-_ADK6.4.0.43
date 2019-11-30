/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    custom_operator_capability_id_translator.h

DESCRIPTION
    API used to override the original/hard-coded capability IDs.
*/

#ifndef LIBS_CUSTOM_OPERATOR_CAPABILITY_ID_TRANSLATOR_H_
#define LIBS_CUSTOM_OPERATOR_CAPABILITY_ID_TRANSLATOR_H_

#include "operators.h"

/*
 * Get the capability ID for this capability using the original/hard-coded capability ID as input.
 * If there is a new capability ID for this capability it returns it, otherwise it returns the capability ID used as input.
 */
capability_id_t customOperatorGetCapabilityId(capability_id_t original_capability_id);

#endif /* LIBS_CUSTOM_OPERATOR_CAPABILITY_ID_TRANSLATOR_H_ */
