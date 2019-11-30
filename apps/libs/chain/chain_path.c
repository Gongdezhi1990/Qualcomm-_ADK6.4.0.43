/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    chain_path.c

DESCRIPTION
    Functions to parse streams
*/

#include <stream.h>
#include <panic.h>

#include "chain_path.h"
#include "chain_connect.h"

#define for_all_paths(config, path) for (path = config->paths; path < (config->paths + config->number_of_paths); path++)

#define isNodeValid(path, node)  (node < (path->nodes + path->number_of_nodes))
#define isNodeCreated(chain, node) (node && (ChainGetOperatorByRole(chain, node->operator_role) != INVALID_OPERATOR))

/******************************************************************************/
static const operator_path_node_t* getNextCreatedNode(kymera_chain_t* chain, const operator_path_t* path, const operator_path_node_t* node)
{
    const operator_path_node_t* next_node;

    if(isNodeValid(path, node))
    {
        for(next_node = node + 1; isNodeValid(path, next_node); next_node++)
        {
            if(isNodeCreated(chain, next_node))
                return next_node;
        }
    }
    return NULL;
}

/******************************************************************************/
static const operator_path_node_t* getFirstCreatedNode(kymera_chain_t* chain, const operator_path_t* path)
{
    const operator_path_node_t* first_node = path->nodes;

    if(isNodeCreated(chain, first_node))
        return first_node;

    return getNextCreatedNode(chain, path, first_node);
}

/******************************************************************************/
static const operator_path_node_t* getLastCreatedNode(kymera_chain_t* chain, const operator_path_t* path)
{
    const operator_path_node_t* last_node = getFirstCreatedNode(chain, path);
    const operator_path_node_t* next_node = getNextCreatedNode(chain, path, last_node);

    while(next_node)
    {
        last_node = next_node;
        next_node = getNextCreatedNode(chain, path, last_node);
    }

    return last_node;
}

/******************************************************************************/
static const operator_path_t* getPathFromPathRole(kymera_chain_t *chain, unsigned path_role)
{
    const chain_config_t *config;

    config = chain->config;

    const operator_path_t* path;

    for_all_paths(config, path)
    {
        if(path->path_role == path_role)
        {
            return path;
        }
    }
    return NULL;
}

/******************************************************************************/
Sink chainPathGetInput(kymera_chain_t *chain, unsigned input_role)
{
    if(chain)
    {
        const chain_config_t *config = chain->config;

        if(config->paths)
        {
            const operator_path_t* path;

            for_all_paths(config, path)
            {
                if(path->path_role == input_role && (path->type & path_with_input))
                {
                    const operator_path_node_t* node = getFirstCreatedNode(chain, path);
                    if (node)
                    {
                        Operator op = ChainGetOperatorByRole(chain, node->operator_role);
                    return StreamSinkFromOperatorTerminal(op, (uint16)node->input_terminal);
                    }
                }
            }
        }
    }
    return NULL;
}

/******************************************************************************/
Source chainPathGetOutput(kymera_chain_t *chain, unsigned output_role)
{
    if(chain)
    {
        const chain_config_t *config = chain->config;

        if(config->paths)
        {
            const operator_path_t* path;

            for_all_paths(config, path)
            {
                if(path->path_role == output_role && (path->type & path_with_output))
                {
                    const operator_path_node_t* node = getLastCreatedNode(chain, path);

                    Operator op = ChainGetOperatorByRole(chain, node->operator_role);
                    return StreamSourceFromOperatorTerminal(op, (uint16)node->output_terminal);
                }
            }
        }
    }
    return NULL;
}

/******************************************************************************/
void chainPathConnect(kymera_chain_t *chain)
{
    const chain_config_t *config;

    if(!chain)
        return;

    config = chain->config;

    if(config->paths)
    {
        const operator_path_t* path;

        for_all_paths(config, path)
        {
            chainPathConnectPath(chain, path->path_role);
        }
    }

}

void chainPathConnectPath(kymera_chain_t *chain, unsigned path_role)
{
    if(!chain)
        return;

    const operator_path_t* path;

    path = getPathFromPathRole(chain, path_role);

    if(!path)
        Panic();

    const operator_path_node_t* node = getFirstCreatedNode(chain, path);

    while(node)
    {
        const operator_path_node_t* next_node = getNextCreatedNode(chain, path, node);

        if(next_node)
        {
            chainConnectOperatorTerminals(chain,
                                          node->operator_role,
                                          node->output_terminal,
                                          next_node->operator_role,
                                          next_node->input_terminal);
        }

        node = next_node;
    }
}

/******************************************************************************/
unsigned chainStreamGetNumberOfStreams(kymera_chain_t *chain)
{
    const chain_config_t *config = chain->config;

    return (config->number_of_paths);
}

/******************************************************************************/
unsigned chainStreamGetRoleByStreamNumber(kymera_chain_t *chain, unsigned path_number)
{
    const chain_config_t *config = chain->config;
    const operator_path_t* path;

    if(path_number > config->number_of_paths)
        Panic();

    path = &config->paths[path_number];
    return (path->path_role);
}
