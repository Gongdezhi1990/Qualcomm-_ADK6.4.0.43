/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    custom_operator.c

DESCRIPTION
    The purpose of this library is to manage the creation and destruction of operators.
    It sits on top of the operators library and manages any DSP capability bundle file load/unload required.
    Additionally, it acts as a medium to override capability IDs at a low level, by specifying that you want
    some capability ID to be replaced with a different one before creating an operator for it.
*/

#include "custom_operator.h"
#include "custom_operator_bundle_files.h"
#include "custom_operator_capability_id_translator.h"
#include "custom_operator_processor_id_translator.h"
#include <audio_config.h>


static const uint16 downloadable_cap_mask = 0x4000;

static bool isDownloadableCapability(capability_id_t cap_id)
{
    if (((uint16) cap_id) & downloadable_cap_mask)
        return TRUE;

    return FALSE;
}

Operator CustomOperatorCreate(capability_id_t cap_id, operator_processor_id_t processor_id, operator_priority_t priority, const operator_setup_t* setup)
{
    Operator op;
    FILE_INDEX bundle_file_index = FILE_NONE;

    processor_id = customOperatorGetProcessorId(cap_id,processor_id);

    cap_id = customOperatorGetCapabilityId(cap_id);

    if (isDownloadableCapability(cap_id))
        bundle_file_index = customOperatorLoadBundle(cap_id);

    op = OperatorsCreateWithSetup(cap_id, processor_id, priority, setup);
    if (bundle_file_index != FILE_NONE)
        customOperatorAddOperatorToBundleFile(op, bundle_file_index);

    return op;
}

void CustomOperatorDestroy(Operator *operators, unsigned number_of_operators)
{
    unsigned i;

    OperatorsDestroy(operators, number_of_operators);

    for(i = 0; i < number_of_operators; i++)
        customOperatorUnloadBundle(operators[i]);
}
