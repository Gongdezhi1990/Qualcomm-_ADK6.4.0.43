/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    custom_operator_processor_id_translator.h

DESCRIPTION
    API used to override the default processor ID for a given capability ID.
*/

#ifndef LIBS_CUSTOM_OPERATOR_PROCESSOR_ID_TRANSLATOR_H_
#define LIBS_CUSTOM_OPERATOR_PROCESSOR_ID_TRANSLATOR_H_

#include "operators.h"

/*
 * Get the processor ID for this capability using the original/hard-coded capability ID as input.
 * If there is an override/replacement for the given capability it returns mapped processor ID, 
        otherwise it returns the default processor ID provided as 2nd parameter.
 * Note a Thread Offloading MIB key must be enabled, or audio subsystem will panic */
operator_processor_id_t customOperatorGetProcessorId(capability_id_t capability_id, operator_processor_id_t default_processor);

#endif /* LIBS_CUSTOM_OPERATOR_PROCESSOR_ID_TRANSLATOR_H_ */
