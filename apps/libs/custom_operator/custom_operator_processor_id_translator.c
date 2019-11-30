/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    custom_operator_processor_id_translator.c

DESCRIPTION
    API used to override the default processor ID for a given capability ID.
*/

#include "custom_operator_processor_id_translator.h"
#include "custom_operator.h"
#include <stdlib.h>
#include <panic.h>

typedef struct
{
    capability_id_t capability_id;
    operator_processor_id_t processor_override;
} capability_processor_ids_t;

static capability_processor_ids_t *array_map = NULL;
static unsigned int entry_count = 0;

static bool scanForExistingProcessorIdMapping(capability_id_t search_cap_id, unsigned int * output_index);

void CustomOperatorReplaceProcessorId(capability_id_t cap_id, operator_processor_id_t override_proc_id )
{
    unsigned int index = 0;

    if(scanForExistingProcessorIdMapping(cap_id, &index) == FALSE)
    {
        index = entry_count++;
        array_map = PanicNull(realloc(array_map,(entry_count * sizeof(capability_processor_ids_t))));
    }
    array_map[index].capability_id = cap_id;
    array_map[index].processor_override = override_proc_id;
}

operator_processor_id_t customOperatorGetProcessorId(capability_id_t cap_id, operator_processor_id_t default_processor)
{
    unsigned int index = 0;
    if(scanForExistingProcessorIdMapping(cap_id, &index))
        return array_map[index].processor_override;
    else
        return default_processor;
}

static bool scanForExistingProcessorIdMapping(capability_id_t search_cap_id, unsigned int * output_index)
{
    bool result = FALSE;
    unsigned int index;
    for(index = 0; index < entry_count; ++index)
    {
        if(array_map[index].capability_id == search_cap_id)
        {
            result = TRUE;
            *output_index = index;
            break;
        }
    }
    return result;
}   

#ifdef HOSTED_TEST_ENVIRONMENT
void CustomOperatorProcessorIdTranslatorTestReset(void)
{
    if(array_map != NULL)
    {
        free(array_map);
        array_map = NULL;
        entry_count = 0;
    }    
}
#endif
