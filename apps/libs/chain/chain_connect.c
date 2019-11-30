/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    chain_connect.c

DESCRIPTION
    Functions for connecting chains based around inputs/outputs/connections
*/

#include <panic.h>
#include <print.h>
#include <stream.h>

#include "chain_connect.h"

/******************************************************************************/
static void connectOperators(kymera_chain_t *chain, const operator_connection_t *connection)
{
    unsigned i;
    for(i = 0; i < connection->number_of_terminals; ++i)
    {
        chainConnectOperatorTerminals(chain, 
                                      connection->source_role, 
                                      connection->first_source_terminal + i,
                                      connection->sink_role,
                                      connection->first_sink_terminal + i);
    }
}

/******************************************************************************/
void chainConnectOperatorTerminals(kymera_chain_t *chain, unsigned source_role, unsigned source_terminal, unsigned sink_role, unsigned sink_terminal)
{
    Source source;
    Sink sink;
    Operator op;
    op = ChainGetOperatorByRole(chain, source_role);
    source = StreamSourceFromOperatorTerminal(op, (uint16)(source_terminal));
    op = ChainGetOperatorByRole(chain, sink_role);
    sink = StreamSinkFromOperatorTerminal(op, (uint16)(sink_terminal));

    PanicNull(source);
    PanicNull(sink);
    PanicNull(StreamConnect(source, sink));
}

/******************************************************************************/
void chainConnectAllOperators(kymera_chain_t *chain)
{
    const operator_connection_t *connection;
    const chain_config_t *config = chain->config;
    
    for (connection = config->connections;
         connection < (config->connections + config->number_of_connections);
         connection++)
    {
        connectOperators(chain, connection);
    }
}

/******************************************************************************/
Sink chainConnectGetInput(kymera_chain_t *chain, unsigned input_role)
{
    const operator_endpoint_t* input;
    const chain_config_t *config = chain->config;
    
    for(input = config->inputs;
        input < (config->inputs + config->number_of_inputs);
        input++)
    {
        PRINT(("Role %d\n", input->endpoint_role));
        if (input->endpoint_role == input_role)
        {
            Operator op = ChainGetOperatorByRole(chain, input->operator_role);
            return StreamSinkFromOperatorTerminal(op, (uint16)input->terminal);
        }
    }
    return NULL;
}

/******************************************************************************/
Source chainConnectGetOutput(kymera_chain_t *chain, unsigned output_role)
{
    const operator_endpoint_t* output;
    const chain_config_t *config = chain->config;
    
    for(output = config->outputs;
        output < (config->outputs + config->number_of_outputs);
        output++)
    {
        PRINT(("Role %d\n", output->endpoint_role));
        if(output->endpoint_role == output_role)
        {
            Operator op = ChainGetOperatorByRole(chain, output->operator_role);
            return StreamSourceFromOperatorTerminal(op, (uint16)output->terminal);
        }
    }
    return NULL;
}

/******************************************************************************/
unsigned chainConnectGetNumberOfOutputs(kymera_chain_t *chain)
{
    const chain_config_t *config = chain->config;

    return (config->number_of_outputs);
}

/******************************************************************************/
unsigned chainConnectGetRoleByOutputEndpointNumber(kymera_chain_t *chain, unsigned endpoint_number)
{
    const operator_endpoint_t* output;
    const chain_config_t *config = chain->config;

    output = (&config->outputs[endpoint_number]);

    return (output->endpoint_role);
}
