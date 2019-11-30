/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    custom_operator_capability_id_translator.c

DESCRIPTION
    API used to override the original/hard-coded capability IDs.
*/

#include "custom_operator_capability_id_translator.h"
#include "custom_operator.h"
#include <stdlib.h>

typedef struct
{
    capability_id_t original;
    capability_id_t new;
} capability_ids_t;

typedef struct capability_ids_list_node_t
{
    capability_ids_t capability_id;
    struct capability_ids_list_node_t *next;
} capability_ids_list_node_t;

static capability_ids_list_node_t *head_node = NULL;

static capability_ids_list_node_t initCapabilityIdNode(capability_id_t original_cap_id, capability_id_t new_cap_id, capability_ids_list_node_t *next_node)
{
    capability_ids_list_node_t node;

    node.capability_id.original = original_cap_id;
    node.capability_id.new = new_cap_id;
    node.next = next_node;

    return node;
}

static capability_ids_list_node_t * createNewCapabilityIdNode(capability_ids_list_node_t nodes_values)
{
    capability_ids_list_node_t *new_node;

    new_node = (capability_ids_list_node_t *) malloc(sizeof(capability_ids_list_node_t));

    if (new_node != NULL)
        *new_node = nodes_values;

    return new_node;
}

static capability_ids_list_node_t * returnNodeWithThisOriginalCapabilityID(capability_id_t original_capability_id)
{
    capability_ids_list_node_t *node;

    for(node = head_node; node != NULL; node = node->next)
    {
        if (node->capability_id.original == original_capability_id)
            return node;
    }

    return NULL;
}

static void addThisNodeToTheList(capability_ids_list_node_t *node)
{
    node->next = head_node;
    head_node = node;
}

capability_id_t customOperatorGetCapabilityId(capability_id_t original_capability_id)
{
    capability_ids_list_node_t *node = returnNodeWithThisOriginalCapabilityID(original_capability_id);

    if (node != NULL)
        return node->capability_id.new;
    else
        return original_capability_id;
}

bool CustomOperatorReplaceCapabilityId(capability_id_t original_cap_id, capability_id_t new_cap_id)
{
    capability_ids_list_node_t *node = returnNodeWithThisOriginalCapabilityID(original_cap_id);

    if (node == NULL)
    {
        capability_ids_list_node_t *new_node = createNewCapabilityIdNode(initCapabilityIdNode(original_cap_id, new_cap_id, NULL));

        if (new_node == NULL)
            return FALSE;
        else
            addThisNodeToTheList(new_node);
    }
    else
        node->capability_id.new = new_cap_id;

    return TRUE;
}

#ifdef HOSTED_TEST_ENVIRONMENT
void CustomOperatorCapabilityIdTranslatorTestReset(void)
{
    capability_ids_list_node_t *node;

    while(head_node != NULL)
    {
        node = head_node;
        head_node = node->next;
        free(node);
    }
}
#endif
