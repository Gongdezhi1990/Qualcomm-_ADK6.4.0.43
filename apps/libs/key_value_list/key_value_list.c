/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      Source file for a data structure with a list of { key, value } elements.

*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <panic.h>
#include <vmtypes.h>

#include "key_value_list.h"


#define KEY_VALUE_TYPE_MASK     (0x3)
#define KEY_VALUE_TYPE_SMALL    (0x1)
#define KEY_VALUE_TYPE_LARGE    (0x2)

#define KEY_VALUE_SMALL_SIZE    (sizeof(((struct key_value_pair_t *)0)->value.u32))

#define KEY_VALUE_MAX_SIZE      ((1 << 12) - 1)


struct key_value_pair_t
{
    uint16 key;
    uint16 size:12;
    uint16 flags:4;
    union
    {
        void *ptr;
        uint32 u32;
    } value;
};

struct key_value_list_tag
{
    uint16 max_items;
    struct key_value_pair_t list[];
};

/*****************************************************************************/

static struct key_value_pair_t *getKeyValuePair(key_value_list_t list, key_value_key_t key);


static bool keyIsValid(key_value_key_t key)
{
    return (key != KEY_VALUE_LIST_INVALID_KEY);
}

static bool keyValueIsType(struct key_value_pair_t *key_value, uint16 type)
{
    return (type == (key_value->flags & KEY_VALUE_TYPE_MASK));
}

static void *getKeyValue(struct key_value_pair_t *key_value)
{
    void * value = NULL;

    if (keyValueIsType(key_value, KEY_VALUE_TYPE_LARGE))
    {
        value = key_value->value.ptr;
    }
    else
    {
        value = (void *)&key_value->value.u32;
    }

    return value;
}

static size_t getKeySize(struct key_value_pair_t *key_value)
{
    return key_value->size;
}

static bool addKeyValuePair(key_value_list_t list, key_value_key_t key, void * value, size_t size)
{
    struct key_value_pair_t *key_value = 0;
    bool success = FALSE;

    key_value = getKeyValuePair(list, KEY_VALUE_LIST_INVALID_KEY);
    if (key_value)
    {
        key_value->key = key;
        if (size <= KEY_VALUE_SMALL_SIZE)
        {
            key_value->flags = KEY_VALUE_TYPE_SMALL;
            memmove(&key_value->value.u32, value, size);
            key_value->size = size;
            success = TRUE;
        }
        else if (size <= KEY_VALUE_MAX_SIZE)
        {
            key_value->flags = KEY_VALUE_TYPE_LARGE;
            key_value->value.ptr = PanicUnlessMalloc(size);
            memmove(key_value->value.ptr, value, size);
            key_value->size = size;
            success = TRUE;
        }
        else
        {
            /* size is too large to store in the key_value_pair_t */
            Panic();
        }
    }

    return success;
}

static void destroyKeyValuePair(key_value_list_t list, struct key_value_pair_t *key_value)
{
    /* If the key-value list is implemented a different way, e.g a linked list,
       then you may need list in here. */
    UNUSED(list);

    if (keyValueIsType(key_value, KEY_VALUE_TYPE_LARGE))
    {
        free(key_value->value.ptr);
    }

    memset(key_value, 0, sizeof(*key_value));

    key_value->key = KEY_VALUE_LIST_INVALID_KEY;
}

static struct key_value_pair_t *getKeyValuePair(key_value_list_t list, key_value_key_t key)
{
    struct key_value_pair_t * key_value = 0;
    int i;

    for (i = 0; i < list->max_items; i++)
    {
        if (list->list[i].key == key)
        {
            key_value = &list->list[i];
            break;
        }
    }

    return key_value;
}

/*****************************************************************************/
key_value_list_t KeyValueList_Create(uint16 max_items)
{
    int i;
    size_t size = sizeof(struct key_value_list_tag) + (max_items * sizeof(struct key_value_pair_t));
    struct key_value_list_tag *list = PanicUnlessMalloc(size);

    /* Initialise the values of the list */
    memset(list, 0, size);

    list->max_items = max_items;
    for (i = 0; i < list->max_items; i++)
    {
        list->list[i].key = KEY_VALUE_LIST_INVALID_KEY;
    }

    return list;
}

void KeyValueList_Destroy(key_value_list_t list)
{
    KeyValueList_RemoveAll(list);
    free(list);
}

bool KeyValueList_Add(key_value_list_t list, key_value_key_t key, void *value, size_t size)
{
    bool success = FALSE;

    PanicNull(list);

    if (!keyIsValid(key) || KeyValueList_IsSet(list, key))
    {
        success = FALSE;
    }
    else
    {
        success = addKeyValuePair(list, key, value, size);
    }

    return success;
}

bool KeyValueList_Get(key_value_list_t list, key_value_key_t key, void **value, size_t *size)
{
    bool found = FALSE;
    struct key_value_pair_t * key_value = 0;

    PanicNull(value);
    PanicNull(size);

    key_value = getKeyValuePair(list, key);
    if (key_value)
    {
        *value = getKeyValue(key_value);
        *size = getKeySize(key_value);
        found = TRUE;
    }

    return found;
}

void KeyValueList_Remove(key_value_list_t list, key_value_key_t key)
{
    struct key_value_pair_t * key_value = 0;

    key_value = getKeyValuePair(list, key);
    if (key_value)
        destroyKeyValuePair(list, key_value);
}

void KeyValueList_RemoveAll(key_value_list_t list)
{
    int i;

    for (i = 0; i < list->max_items; i++)
    {
        if (list->list[i].key != KEY_VALUE_LIST_INVALID_KEY)
        {
            destroyKeyValuePair(list, &list->list[i]);
        }
    }
}

bool KeyValueList_IsSet(key_value_list_t list, key_value_key_t key)
{
    bool is_set = FALSE;

    if (keyIsValid(key))
    {
        if (getKeyValuePair(list, key))
        {
            is_set = TRUE;
        }
    }

    return is_set;
}
