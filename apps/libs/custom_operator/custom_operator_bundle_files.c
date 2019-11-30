/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    custom_operator_bundle_files.c

DESCRIPTION
    Custom operator bundle management implementation.
*/

#include "custom_operator_bundle_files.h"
#include "custom_operator_dkcs_reader.h"
#include "custom_operator.h"
#include "file_list.h"

#include <panic.h>
#include <stdlib.h>
#include <print.h>


typedef struct operator_list_t
{
    Operator operator;
    struct operator_list_t *next;
} operator_list_t;

typedef struct loaded_bundle_list_t
{
    /* The file index (file-system) of the bundle, used to uniquely identify a bundle file */
    FILE_INDEX file_index;
    /* A bundle handle used to unload a loaded bundle file */
    BundleID handle;
    /* The list of operators using the bundle */
    operator_list_t *head_of_operator_list;
    /* Next element in list */
    struct loaded_bundle_list_t *next;
    /* The number of capabilities offered by this bundle */
    uint32 num_cap_ids;
    /* The IDs of the capabilities offered by this bundle */
    uint16 capability_ids[];
} loaded_bundle_list_t;

static loaded_bundle_list_t *head_of_bundle_list = NULL;

static loaded_bundle_list_t * getBundleNodeWithThisFileIndex(FILE_INDEX file_index)
{
    loaded_bundle_list_t *bundle_node;

    for(bundle_node = head_of_bundle_list; bundle_node != NULL; bundle_node = bundle_node->next)
    {
        if (bundle_node->file_index == file_index)
            return bundle_node;
    }

    return NULL;
}

static void addBundleNode(BundleID bundleId,
                          FILE_INDEX file_index,
                          dkcs_header_t *dkcs_header)
{
    unsigned cap_count;
    loaded_bundle_list_t *new_node = PanicUnlessMalloc(sizeof(*new_node)
                                     + (dkcs_header->num_cap_ids * sizeof(new_node->capability_ids[0])));

    new_node->file_index = file_index;
    new_node->handle = bundleId;
    new_node->head_of_operator_list = NULL;

    new_node->num_cap_ids = dkcs_header->num_cap_ids;
    for (cap_count = 0; cap_count < dkcs_header->num_cap_ids; cap_count++)
        new_node->capability_ids[cap_count] = dkcs_header->capability_ids[cap_count];

    new_node->next = head_of_bundle_list;
    head_of_bundle_list = new_node;
}

static BundleID loadBundle(file_list_handle file_handle,
                           FILE_INDEX file_index)
{
    BundleID bundleId = BUNDLE_ID_INVALID;
    const capability_bundle_processor_availability_t *processor = FileListGetFileRelatedData(file_handle);
    
    PanicFalse(processor != NULL);

    bundleId = OperatorBundleLoad(file_index, (uint16) *processor);
    
    PanicFalse(bundleId != BUNDLE_ID_INVALID);
   
    return bundleId;
}

static bool isCapabilityIdInCapabilityIds(capability_id_t capability_id, uint16 *capability_ids, unsigned number_of_capability_ids)
{
    unsigned i;

    for(i = 0; i < number_of_capability_ids; i++)
    {
        if (capability_ids[i] == (uint16) capability_id)
            return TRUE;
    }

    return FALSE;
}

static bool isCapabilityInBundle(capability_id_t cap_id, dkcs_header_t *dkcs_header)
{
    return isCapabilityIdInCapabilityIds(cap_id, dkcs_header->capability_ids, dkcs_header->num_cap_ids);
}

static void freeOperatorNode(operator_list_t *operator_node)
{
    free(operator_node);
}

static void deleteFromOperatorList(operator_list_t *operator_node, operator_list_t **head_of_operator_list)
{
    operator_list_t **node;

    for(node = head_of_operator_list; *node != NULL; node = &(*node)->next)
    {
        if (*node == operator_node)
        {
            *node = (*node)->next;
            freeOperatorNode(operator_node);
            return;
        }
    }
}

static void freeBundleNode(loaded_bundle_list_t *bundle_node)
{
    while(bundle_node->head_of_operator_list != NULL)
        deleteFromOperatorList(bundle_node->head_of_operator_list, &bundle_node->head_of_operator_list);

    free(bundle_node);
}

static void deleteFromBundleList(loaded_bundle_list_t *bundle_node)
{
    loaded_bundle_list_t **node;
    

    for(node = &head_of_bundle_list; *node != NULL; node = &(*node)->next)
    {
        if (*node == bundle_node)
        {
            *node = (*node)->next;
            freeBundleNode(bundle_node);
            return;
        }
    }
 }

static void unloadBundleIfNoUsers(loaded_bundle_list_t *bundle_node)
{
    if (bundle_node->head_of_operator_list == NULL)
    {
        PanicFalse(OperatorBundleUnload(bundle_node->handle));
        PRINT(("CUSTOM_OPERATOR: unloaded bundle with file index %u\n", bundle_node->file_index));
        deleteFromBundleList(bundle_node);
    }
}

static bool removeOperatorFromBundleNode(loaded_bundle_list_t *bundle_node, Operator operator)
{
    operator_list_t *operator_node;

    for(operator_node = bundle_node->head_of_operator_list; operator_node != NULL; operator_node = operator_node->next)
    {
        if (operator_node->operator == operator)
        {
            deleteFromOperatorList(operator_node, &bundle_node->head_of_operator_list);
            return TRUE;
        }
    }

    return FALSE;
}

static void addOperatorToBundleNode(Operator op, loaded_bundle_list_t *bundle_node)
{
    operator_list_t *new_node = PanicUnlessMalloc(sizeof(*new_node));

    new_node->operator = op;
    new_node->next = bundle_node->head_of_operator_list;
    bundle_node->head_of_operator_list = new_node;
}

static FILE_INDEX findCapabilityInLoadedBundles(capability_id_t cap_id)
{
    loaded_bundle_list_t *bundle_node;

    for(bundle_node = head_of_bundle_list; bundle_node != NULL; bundle_node = bundle_node->next)
    {
        if (isCapabilityIdInCapabilityIds(cap_id, bundle_node->capability_ids, bundle_node->num_cap_ids))
        {
            return bundle_node->file_index;
        }
    }

    return FILE_NONE;
}

static FILE_INDEX findAndLoadBundleFromFilesystem(capability_id_t cap_id)
{
    FILE_INDEX file_index = FILE_NONE;
    file_list_handle file_handle;
    dkcs_header_t *dkcs_header;
    unsigned i, number_of_bundles = FileListGetNumberOfFiles(downloadable_capabilities_file_role);

    for (i = 0; i < number_of_bundles && file_index == FILE_NONE; i++)
    {
        file_handle = FileListGetFileHandle(downloadable_capabilities_file_role, i);
        file_index = FileListGetFileIndex(file_handle);
        dkcs_header = dkcsHeaderRead(file_index);

        PanicFalse(dkcs_header != NULL);

        if (isCapabilityInBundle(cap_id, dkcs_header))
        {
            BundleID bundleId = loadBundle(file_handle, file_index);
            addBundleNode(bundleId, file_index, dkcs_header);
        }
        else
            file_index = FILE_NONE;
            
        dkcsHeaderFree(dkcs_header);
    }
    
    return file_index;
}

void customOperatorAddOperatorToBundleFile(Operator op, FILE_INDEX bundle_file_index)
{
    loaded_bundle_list_t *bundle_node = PanicNull(getBundleNodeWithThisFileIndex(bundle_file_index));

    addOperatorToBundleNode(op, bundle_node);
}

FILE_INDEX customOperatorLoadBundle(capability_id_t cap_id)
{
    FILE_INDEX file_index = findCapabilityInLoadedBundles(cap_id);

    if (file_index == FILE_NONE)
    {
        file_index = findAndLoadBundleFromFilesystem(cap_id);
    }
    
    PanicFalse(file_index != FILE_NONE);
    
    return file_index;
}

void customOperatorUnloadBundle(Operator operator)
{
    loaded_bundle_list_t **bundle_node;

    for(bundle_node = &head_of_bundle_list; *bundle_node != NULL; bundle_node = &(*bundle_node)->next)
    {
        if (removeOperatorFromBundleNode(*bundle_node, operator))
        {
            unloadBundleIfNoUsers(*bundle_node);
            break;
        }
    }
}

#ifdef HOSTED_TEST_ENVIRONMENT
void CustomOperatorBundleFilesTestReset(void)
{
    while(head_of_bundle_list != NULL)
        deleteFromBundleList(head_of_bundle_list);
}
#endif
