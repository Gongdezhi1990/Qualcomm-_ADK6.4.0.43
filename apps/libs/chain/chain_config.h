/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

#ifndef CHAIN_CONFIG_H_
#define CHAIN_CONFIG_H_

#include "chain_list.h"

/****************************************************************************
DESCRIPTION
    Store config and filters in the chain structure
*/
void chainConfigStore(kymera_chain_t *chain, const chain_config_t *config, const operator_filters_t* filter);

/****************************************************************************
DESCRIPTION
    Check whether the chain was set up to use streams (or the inputs/outputs/
    connections configuration)
*/
bool chainConfigIsStreamBased(const kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Get the operators config for index. Note that this may be filtered in which
    case this function will return NULL.
*/
const operator_config_t* chainConfigGetOperatorConfig(const kymera_chain_t *chain, unsigned index);

/****************************************************************************
DESCRIPTION
    Check whether every operator for the chain has been removed by filters
*/
bool chainConfigIsWholeChainFiltered(const kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Check whether the chain has been configured with operators to be created
    in the second audio processor
*/
bool chainConfigUsesSecondProcessor(const kymera_chain_t *chain);

#endif /* CHAIN_CONFIG_H_ */
