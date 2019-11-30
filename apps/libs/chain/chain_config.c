/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    chain_config.c

DESCRIPTION
    Functions for accessing chain configuration data
*/

#include <panic.h>
#include <print.h>
#include "chain_config.h"

/****************************************************************************
DESCRIPTION
    This function will return filtered version of the operator_config_t if role matches.
*/
static const operator_config_t* getOperatorConfigIfRoleInFilter(unsigned role, const operator_filters_internal_t* filter)
{
    unsigned i;

    for(i = 0; i < filter->num_operator_filters; i++)
    {
        if(filter->operator_filters[i].role == role)
        {
            return &filter->operator_filters[i];
        }
    }
    return NULL;
}

/****************************************************************************
DESCRIPTION
    This function will return either filtered version of operator_config_t or stream's operator_config_t, unless the capability_id is set to 
    capability_id_none in which case it will return NULL.
*/
static const operator_config_t* getFilteredOperatorConfig(const kymera_chain_t *chain, const operator_config_t* op_config)
{
    const operator_config_t* result = getOperatorConfigIfRoleInFilter(op_config->role, &chain->filters);

    if(!result)
        result = op_config;
 
    if(result->capability_id == capability_id_none)
        return NULL;

    return result;
}

/******************************************************************************/
void chainConfigStore(kymera_chain_t *chain, const chain_config_t *config, const operator_filters_t* filter)
{
    chain->config = config;
    if(filter)
    {
        if(!chainConfigIsStreamBased(chain))
        {
            PRINT(("Cannot filter chains unless they use stream based config\n"));
            Panic();
        }
        
        chain->filters.num_operator_filters = filter->num_operator_filters;
        memcpy(chain->filters.operator_filters, filter->operator_filters, filter->num_operator_filters * sizeof(operator_config_t));
    }
}

/******************************************************************************/
bool chainConfigIsStreamBased(const kymera_chain_t *chain)
{
    return chain->config->paths != NULL;
}

/******************************************************************************/
const operator_config_t* chainConfigGetOperatorConfig(const kymera_chain_t *chain, unsigned index)
{
    const operator_config_t* op_config;
    
    if(chain && chain->config)
    {
        op_config = &chain->config->operator_config[index];
        
        return getFilteredOperatorConfig(chain, op_config);
    }
    
    return NULL;
}

/******************************************************************************/
bool chainConfigIsWholeChainFiltered(const kymera_chain_t *chain)
{
    unsigned i;
    const chain_config_t *config = chain->config;
    
    for (i = 0; i < config->number_of_operators; i++)
    {
        if(chainConfigGetOperatorConfig(chain, i))
            return FALSE;
    }
    return TRUE;
}

/******************************************************************************/
bool chainConfigUsesSecondProcessor(const kymera_chain_t *chain)
{
    unsigned i;
    const chain_config_t *config = chain->config;
    
    for (i = 0; i < config->number_of_operators; i++)
    {
        const operator_config_t *op_config = chainConfigGetOperatorConfig(chain, i);
        
        if(op_config && (op_config->processor_id == OPERATOR_PROCESSOR_ID_1))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************/
const chain_config_t* ChainGetConfig(kymera_chain_handle_t handle)
{
    if(handle)
        return handle->config;
    
    return NULL;
}
