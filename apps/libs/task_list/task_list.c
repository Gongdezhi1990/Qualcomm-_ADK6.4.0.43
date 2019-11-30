/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       task_list.c
\brief      Implementation of a simple list of VM tasks.
*/

#include "task_list.h"

#include <stdlib.h>
#include <string.h>

#include <panic.h>

static message_sniffer_t message_sniffer = NULL;

/******************************************************************************
 * Internal functions
 ******************************************************************************/
/*! \brief Find the index in the task list array for a given task.

    \param list [IN] Pointer to a Tasklist.
    \param search_task [IN] Task to search for on list.
    \param index [OUT] Index at which search_task is found.

    \return bool TRUE search_task found and index returned.
                 FALSE search_task not found, index not valid.
 */
static bool findTaskIndex(task_list_t* list, Task search_task, uint16* index)
{
    bool task_index_found = FALSE;
    uint16 iter = 0;

    for (iter = 0; iter < list->size_list; iter++)
    {
        if (list->tasks[iter] == search_task)
        {
            *index = iter;
            task_index_found = TRUE;
        }
    }

    return task_index_found;
}

/*! \brief Helper function that iterates through a list but also returns the index.

    \param list [IN] Pointer to a Tasklist.
    \param next_task [IN/OUT] IN Task from which to continue iterating though list.
                              OUT Next task in the list after next_task passed in.
    \param index [OUT] Index in the list at which OUT next_task was returned.

    \return bool TRUE iteration successful, another next_task and index provided.
                 FALSE empty list or end of list reached. next_task and index not
                       valid.
 */
static bool iterateIndex(task_list_t* list, Task* next_task, uint16* index)
{
    bool iteration_successful = FALSE;
    uint16 tmp_index = 0;

    PanicNull(list);
    PanicNull(next_task);

    /* list not empty */
    if (list->size_list)
    {
        /* next_task == NULL to start at tmp_index 0 */
        if (*next_task == 0)
        {
            *next_task = list->tasks[tmp_index];
            *index = tmp_index;
            iteration_successful =  TRUE;
        }
        else
        {
            /* move to next task */
            if (findTaskIndex(list, *next_task, &tmp_index))
            {
                if (tmp_index + 1 < list->size_list)
                {
                    *next_task = list->tasks[tmp_index+1];
                    *index = tmp_index+1;
                    iteration_successful =  TRUE;
                }
            }
            else
            {
                /* end of the list */
                *next_task = 0;
            }
        }
    }

    return iteration_successful;
}

/******************************************************************************
 * External API functions
 ******************************************************************************/
/*! \brief Create a task_list_t.
 */
task_list_t* TaskList_Create(void)
{
    task_list_t* new_list = (task_list_t*)PanicUnlessMalloc(sizeof(task_list_t));

    if (new_list)
    {
        new_list->size_list = 0;
        new_list->tasks = NULL;
        new_list->data = NULL;
        new_list->list_type = TASKLIST_TYPE_STANDARD;
    }

    return new_list;
}

/*! \brief Create a task_list_t that can also store associated data.
 */
task_list_t* TaskList_WithDataCreate(void)
{
    task_list_t* new_list = TaskList_Create();

    if (new_list)
    {
        new_list->list_type = TASKLIST_TYPE_WITH_DATA;
    }

    return new_list;
}

/*! \brief Destroy a task_list_t.
 */
void TaskList_Destroy(task_list_t* list)
{
    PanicNull(list);

    free(list->tasks);

    if (list->list_type == TASKLIST_TYPE_WITH_DATA)
    {
        free(list->data);
    }

    free(list);
}

/*! \brief Add a task to a list.
 */
bool TaskList_AddTask(task_list_t* list, Task add_task)
{
    bool task_added = FALSE;

    PanicNull(list);
    PanicNull(add_task);

    /* if not in the list */
    if (!TaskList_IsTaskOnList(list, add_task))
    {
        /* Resize list */
        list->tasks = realloc(list->tasks, sizeof(Task) * (list->size_list + 1));
        PanicNull(list->tasks);

        /* Add task to list */
        list->tasks[list->size_list] = add_task;
        list->size_list += 1;

        task_added = TRUE;
    }

    return task_added;
}

/*! \brief Add a task and data to a list.
 */
bool TaskList_AddTaskWithData(task_list_t* list, Task add_task, const task_list_data_t* data)
{
    bool task_with_data_added = FALSE;

    if (TaskList_IsTaskListWithData(list) && TaskList_AddTask(list, add_task))
    {
        /* create space in 'data' for new data item, size_list already
         * accounts for the +1 after call to TaskList_AddTask */
        list->data = realloc(list->data, sizeof(task_list_data_t) * (list->size_list));
        PanicNull(list->data);
        /* but do need to use size_list-1 to access the new last
         * entry in the data array */
        list->data[list->size_list-1] = *data;
        task_with_data_added = TRUE;
    }

    return task_with_data_added;
}

/*! \brief Determine if a task is on a list.
 */
bool TaskList_IsTaskOnList(task_list_t* list, Task search_task)
{
    uint16 tmp;

    PanicNull(list);

    return findTaskIndex(list, search_task, &tmp);
}

/*! \brief Remove a task from a list.
 */
bool TaskList_RemoveTask(task_list_t* list, Task del_task)
{
    bool task_removed = FALSE;
    uint16 index = 0;

    PanicNull(list);
    PanicNull(del_task);

    if (findTaskIndex(list, del_task, &index))
    {
        uint16 tasks_to_end = list->size_list - index - 1;
        memmove(&list->tasks[index], &list->tasks[index] + 1, sizeof(Task) * tasks_to_end);
        if (list->list_type == TASKLIST_TYPE_WITH_DATA)
        {
            memmove(&list->data[index], &list->data[index] + 1, sizeof(task_list_data_t) * tasks_to_end);
        }
        list->size_list -= 1;

        if (!list->size_list)
        {
            free(list->tasks);
            list->tasks = NULL;
            if (list->list_type == TASKLIST_TYPE_WITH_DATA)
            {
                free(list->data);
                list->data = NULL;
            }
        }
        else
        {
            /* resize list, if not an empty list now, then check realloc successfully
             * returned the memory */
            list->tasks = realloc(list->tasks, sizeof(Task) * list->size_list);
            PanicNull(list->tasks);
            if (list->list_type == TASKLIST_TYPE_WITH_DATA)
            {
                list->data = realloc(list->data, sizeof(task_list_data_t) * list->size_list);
                PanicNull(list->data);
            }
        }

        task_removed = TRUE;
    }

    return task_removed;
}

/*! \brief Return number of tasks in list.
 */
uint16 TaskList_Size(task_list_t* list)
{
    return list ? list->size_list : 0;
}

/*! \brief Iterate through all tasks in a list.
 */
bool TaskList_Iterate(task_list_t* list, Task* next_task)
{
    uint16 tmp_index = 0;
    return iterateIndex(list, next_task, &tmp_index);
}

/*! \brief Iterate through all tasks in a list returning a copy of data as well.
 */
bool TaskList_IterateWithData(task_list_t* list, Task* next_task, task_list_data_t* data)
{
    bool iteration_successful = FALSE;
    task_list_data_t *raw_data = NULL;

    if (TaskList_IterateWithDataRaw(list, next_task, &raw_data))
    {
        *data = *raw_data;
        iteration_successful = TRUE;
    }

    return iteration_successful;
}

/*! \brief Iterate through all tasks in a list returning raw data stored in the
    task_list_t.
 */
bool TaskList_IterateWithDataRaw(task_list_t* list, Task* next_task, task_list_data_t** raw_data)
{
    bool iteration_successful = FALSE;
    uint16 tmp_index = 0;

    if (TaskList_IsTaskListWithData(list) && iterateIndex(list, next_task, &tmp_index))
    {
        *raw_data = &list->data[tmp_index];
        iteration_successful = TRUE;
    }

    return iteration_successful;
}

/*! \brief Iterate through all tasks in a list calling handler each iteration. */
bool TaskList_IterateWithDataRawFunction(task_list_t *list, TaskListIterateWithDataRawHandler handler, void *arg)
{
    Task next_task = 0;
    uint16 index = 0;
    bool proceed = TRUE;

    PanicFalse(TaskList_IsTaskListWithData(list));

    while (proceed && iterateIndex(list, &next_task, &index))
    {
        proceed = handler(next_task, &list->data[index], arg);
    }

    return proceed;
}

/*! \brief Create a duplicate task list.
 */
task_list_t *TaskList_Duplicate(task_list_t* list)
{
    task_list_t *new_list = NULL;

    PanicNull(list);

    if (list->list_type == TASKLIST_TYPE_STANDARD)
    {
        new_list = TaskList_Create();
    }
    else
    {
        new_list = TaskList_WithDataCreate();
    }

    if (new_list)
    {
        new_list->size_list = list->size_list;
        new_list->tasks = PanicUnlessMalloc(sizeof(Task) * new_list->size_list);
        memcpy(new_list->tasks, list->tasks, sizeof(Task) * new_list->size_list);

        if (new_list->list_type == TASKLIST_TYPE_WITH_DATA)
        {
            new_list->data = PanicUnlessMalloc(sizeof(task_list_data_t) * new_list->size_list);
            memcpy(new_list->data, list->data, sizeof(task_list_data_t) * new_list->size_list);
        }
    }

    return new_list;
}

/*! \brief Send a message (with message body) to all tasks in the task list.
*/
void TaskList_MessageSendWithSize(task_list_t *list, MessageId id, void *data, size_t size_data)
{
    PanicNull(list);

    if (message_sniffer)
        message_sniffer(id, data, size_data);

    if (list->size_list)
    {
        int index;
        for (index = 1; index < list->size_list; index++)
        {
            if (size_data && (data != NULL))
            {
                void *copy = PanicUnlessMalloc(size_data);
                memcpy(copy, data, size_data);
                MessageSend(list->tasks[index], id, copy);
            }
            else
                MessageSend(list->tasks[index], id, NULL);
        }

        /* Send last message */
        MessageSend(list->tasks[0], id, size_data ? data : NULL);
    }
    else
    {
        MessageFree(id, size_data ? data : NULL);
    }
}

/*! \brief Get a copy of the data stored in the list for a given task.
*/
bool TaskList_GetDataForTask(task_list_t* list, Task search_task, task_list_data_t* data)
{
    bool data_copied = FALSE;
    task_list_data_t *raw_data = NULL;

    if (TaskList_GetDataForTaskRaw(list, search_task, &raw_data))
    {
        *data = *raw_data;
        data_copied = TRUE;
    }

    return data_copied;
}

/*! \brief Get the address of the data stored in the list for a given task.
*/
bool TaskList_GetDataForTaskRaw(task_list_t* list, Task search_task, task_list_data_t** raw_data)
{
    bool data_copied = FALSE;
    uint16 tmp;

    if (TaskList_IsTaskListWithData(list) && findTaskIndex(list, search_task, &tmp))
    {
        *raw_data = &list->data[tmp];
        data_copied = TRUE;
    }

    return data_copied;
}

/*! \brief Set the data stored in the list for a given task.
*/
bool TaskList_SetDataForTask(task_list_t* list, Task search_task, const task_list_data_t* data)
{
    bool data_set = FALSE;
    uint16 tmp;

    if (TaskList_IsTaskListWithData(list) && findTaskIndex(list, search_task, &tmp))
    {
        list->data[tmp] = *data;
        data_set = TRUE;
    }

    return data_set;
}

/*! \brief Determine if the list is one that supports data.
*/
bool TaskList_IsTaskListWithData(task_list_t* list)
{
    PanicNull(list);
    return list->list_type == TASKLIST_TYPE_WITH_DATA;
}

void TaskList_RegisterMessageSniffer(message_sniffer_t sniffer)
{
    message_sniffer = sniffer;
}

void TaskList_Init(void)
{
    message_sniffer = NULL;
}