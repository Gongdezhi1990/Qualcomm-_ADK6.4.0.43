/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

#ifndef CHAIN_LIST_H_
#define CHAIN_LIST_H_

#include "chain.h"

typedef struct kymera_chain_tag kymera_chain_t;

typedef struct
{
    unsigned num_operator_filters;
    operator_config_t* operator_filters;
} operator_filters_internal_t;

struct kymera_chain_tag
{
    const chain_config_t *config;
    Operator *operator_list;
    operator_filters_internal_t filters;
    kymera_chain_t *next;
    bool chain_enabled;
};

/****************************************************************************
DESCRIPTION
    Add a chain to the chain list
*/
void chainListAdd(kymera_chain_t *chain);

/****************************************************************************
DESCRIPTION
    Remove a chain from the chain list
*/
void chainListRemove(kymera_chain_t *chain);

#endif /* CHAIN_LIST_H_ */
