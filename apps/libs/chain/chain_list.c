/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    chain_list.c

DESCRIPTION
    Linked list of all created chains
*/

#include <panic.h>
#include "chain_list.h"

static kymera_chain_t *chain_list = NULL;

/******************************************************************************/
void chainListAdd(kymera_chain_t *chain)
{
    kymera_chain_t **head;
    chain->next = NULL;
    for (head = &chain_list; *head != NULL; head = &(*head)->next);
    *head = chain;
}

/******************************************************************************/
void chainListRemove(kymera_chain_t *chain)
{
    kymera_chain_t **head;

    for (head = &chain_list; *head != NULL; head = &(*head)->next)
    {
        if (chain == *head)
        {
            *head = chain->next;
            break;
        }
    }
}

/******************************************************************************/
kymera_chain_handle_t ChainFindById(chain_id_t id, unsigned index)
{
    kymera_chain_t *item;
    kymera_chain_handle_t chain = NULL;

#ifndef HOSTED_TEST_ENVIRONMENT
    Panic();
#endif

    for (item = chain_list; item != NULL; item = item->next)
    {
        if (item->config->chain_id == id)
        {
            if (index == 0)
            {
                chain = item;
                break;
            }
            else
            {
                index--;
            }
        }
    }
    return chain;
}

/******************************************************************************/
#ifdef HOSTED_TEST_ENVIRONMENT
void ChainTestReset(void)
{
    kymera_chain_t *item;
    for (item = chain_list; item != NULL; item = chain_list)
    {
        ChainDestroy(item);
        ChainSetDownloadableCapabilityBundleConfig(NULL);
    }
}
#endif
