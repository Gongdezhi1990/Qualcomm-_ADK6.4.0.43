/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
*/

#ifndef CHAIN_CONNECT_H_
#define CHAIN_CONNECT_H_

#include "chain_list.h"

/****************************************************************************
DESCRIPTION
    Connect source terminal to sink terminal
*/
void chainConnectOperatorTerminals(kymera_chain_t *chain, unsigned source_role, unsigned source_terminal, unsigned sink_role, unsigned sink_terminal);

/****************************************************************************
DESCRIPTION
    Connect all operators listed in the config connections
*/
void chainConnectAllOperators(kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Get the input terminal from config inputs which matches input_role
*/
Sink chainConnectGetInput(kymera_chain_t *chain, unsigned input_role);

/****************************************************************************
DESCRIPTION
    Get the input terminal from config outputs which matches output_role
*/
Source chainConnectGetOutput(kymera_chain_t *chain, unsigned output_role);

/****************************************************************************
DESCRIPTION
    Get the number of output endpoint roles in the chain
*/
unsigned chainConnectGetNumberOfOutputs(kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Get a role by output endpoint number
*/
unsigned chainConnectGetRoleByOutputEndpointNumber(kymera_chain_t *chain, unsigned endpoint_number);
#endif /* CHAIN_CONNECT_H_ */
