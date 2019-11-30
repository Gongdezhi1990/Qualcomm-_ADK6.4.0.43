/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    file_list.c
DESCRIPTION
    Library to store file specific information and primarily its file system index.
    The file entries are organised in groups based on a role.
*/

#include "file_list.h"
#include <string.h>
#include <vmtypes.h>
#include <stdlib.h>

struct file_list_entry_t
{
    unsigned role;
    FILE_INDEX index;
    void *data;
};

typedef struct file_list_node_t
{
    struct file_list_entry_t file_entry;
    struct file_list_node_t *next;
} file_list_node_t;

static file_list_node_t *head_node = NULL;

static bool isRelatedDataValid(const file_related_data_t *related_data)
{
    if ((related_data != NULL) && (related_data->data != NULL) && (related_data->size_of_data > 0))
        return TRUE;
    else
        return FALSE;
}

static void * getCopyOfFileListEntryData(const file_related_data_t *related_data)
{
    void *data_copy;

    data_copy = malloc(related_data->size_of_data);
    if (data_copy != NULL)
        memcpy(data_copy, related_data->data, related_data->size_of_data);

    return data_copy;
}

static void freeNode(file_list_node_t *node)
{
    free(node->file_entry.data);
    free(node);
}

static bool initNewFileListNode(file_list_node_t *node, unsigned role, FILE_INDEX index, const file_related_data_t *related_data)
{
    node->next = NULL;
    node->file_entry.role = role;
    node->file_entry.index = index;

    if (isRelatedDataValid(related_data))
    {
        node->file_entry.data = getCopyOfFileListEntryData(related_data);
        if (node->file_entry.data == NULL)
        {
            freeNode(node);
            return FALSE;
        }
    }
    else
        node->file_entry.data = NULL;

    return TRUE;
}

static file_list_node_t * createNewFileListNode(unsigned role, FILE_INDEX index, const file_related_data_t *related_data)
{
    file_list_node_t *new_node;

    new_node = (file_list_node_t *) malloc(sizeof(file_list_node_t));

    if ((new_node != NULL) && (initNewFileListNode(new_node, role, index, related_data)))
        return new_node;
    else
        return NULL;
}

bool FileListAddFile(unsigned role, FILE_INDEX index, const file_related_data_t *related_data)
{
    file_list_node_t *new_node;

    new_node = createNewFileListNode(role, index, related_data);

    if (new_node == NULL)
        return FALSE;
    else
    {
        new_node->next = head_node;
        head_node = new_node;

        return TRUE;
    }
}

file_list_handle FileListGetFileHandle(unsigned role, unsigned number)
{
    unsigned current_number;
    file_list_node_t *node = head_node;

    for(current_number = 0; node != NULL; node = node->next)
    {
        if (node->file_entry.role == role)
        {
            if (current_number == number)
                return &(node->file_entry);
            else
                current_number++;
        }
    }

    return NULL;
}

unsigned FileListGetNumberOfFiles(unsigned role)
{
    unsigned number_of_files;
    file_list_node_t *node = head_node;

    for(number_of_files = 0; node != NULL; node = node->next)
    {
        if (node->file_entry.role == role)
            number_of_files++;
    }

    return number_of_files;
}

FILE_INDEX FileListGetFileIndex(file_list_handle file_handle)
{
    if (file_handle == NULL)
        return FILE_NONE;
    else
        return file_handle->index;
}

const void * FileListGetFileRelatedData(file_list_handle file_handle)
{
    if (file_handle == NULL)
        return NULL;
    else
        return file_handle->data;
}

void FileListRemoveFiles(unsigned role)
{
    file_list_node_t *node, *next_node, *previous_node = NULL;

    for(node = head_node; node != NULL; node = next_node)
    {
        next_node = node->next;

        if(node->file_entry.role == role)
        {
            if(node == head_node)
                head_node = next_node;
            else
                previous_node->next = next_node;

            freeNode(node);
        }
        else
            previous_node = node;
    }
}

#ifdef HOSTED_TEST_ENVIRONMENT
void FileListLibraryTestReset(void)
{
    file_list_node_t *node, *next_node;

    for(node = head_node; node != NULL; node = next_node)
    {
        next_node = node->next;
        freeNode(node);
    }

    head_node = NULL;
}
#endif
