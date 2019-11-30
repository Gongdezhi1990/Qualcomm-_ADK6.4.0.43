/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_tasklist.h
\brief      Interface to simple list of VM tasks.
*/

#ifndef AV_HEADSET_TASKLIST_H
#define AV_HEADSET_TASKLIST_H

/*! \brief Types of TaskList.
 */
typedef enum
{
    /*! Standard list of Tasks. */
    TASKLIST_TYPE_STANDARD,

    /*! TaskList with associated data. */
    TASKLIST_TYPE_WITH_DATA
} TaskListType;

/*! 64-bit TaskListData type */
typedef unsigned long long tl_uint64;

/*! \brief Definition of 'data' that can be stored in a TaskListWithData.
 */
typedef union
{
    /*! Unsigned 8-bit data. */
    uint8 u8;

    /*! Unsigned 16-bit data. */
    uint16 u16;

    /*! Unsigned 32-bit data. */
    uint32 u32;

    /*! Pointer. */
    void* ptr;

    /*! Unsigned 64-bit data. */
    tl_uint64 u64;

    /*! Signed 8-bit data. */
    int8 s8;

    /*! Signed 16-bit data. */
    int16 s16;

    /*! Signed 32-bit data. */
    int32 s32;

    /*! Unsigned 8-bit array */
    uint8 arr_u8[8];

    /*! Signed 8-bit array */
    int8 arr_s8[8];
} TaskListData;

/*! \brief List of VM Tasks.
 */
typedef struct
{
    /*! List of tasks. */
    Task* tasks;

    /*! Number of tasks in #tasks. */
    uint16 size_list;

    /*! List of data items. */
    TaskListData* data;

    /*! Standard TaskList or one that can support data. */
    TaskListType list_type;
} TaskList;

/*! \brief A function type to handle one iteration of #appTaskListIterateWithDataRawFunction.
    \param task The current task.
    \param data The current task's data.
    \param arg An abstract object passed by the caller to #appTaskListIterateWithDataRawFunction.
    \return TRUE if the iteration should continue, FALSE if the iteration should stop.
    \note The data pointer is guaranteed valid only for the current call. The TaskList module
    reserves the right to invalidate this pointer (e.g. through reallocation)
    in any subsequent TaskList function call.
 */
typedef bool (*TaskListIterateWithDataRawHandler)(Task task, TaskListData *data, void *arg);

/*! \brief Create a TaskList.

    \return TaskList* Pointer to new TaskList.
 */
TaskList* appTaskListInit(void);

/*! \brief Create a TaskList that can also store associated data.

    \return TaskList* Pointer to new TaskList.
 */
TaskList* appTaskListWithDataInit(void);

/*! \brief Destroy a TaskList.

    \param[in] list Pointer to a Tasklist.

    \return bool TRUE TaskList destroyed successfully.
                 FALSE TaskList destroy failure.

 */
void appTaskListDestroy(TaskList* list);

/*! \brief Add a task to a list.
 
    \param[in] list     Pointer to a Tasklist.
    \param[in] add_task Task to add to the list.

    \return FALSE if the task is already on the list, otherwise TRUE.
 */
bool appTaskListAddTask(TaskList* list, Task add_task);

/*! \brief Add a task and data to a list.
 
    \param[in] list     Pointer to a Tasklist.
    \param[in] add_task Task to add to the list.
    \param[in] data     Data to store with the task on the list.

    \return FALSE if the task is already on the list, otherwise TRUE.
 */
bool appTaskListAddTaskWithData(TaskList* list, Task add_task, const TaskListData* data);

/*! \brief Remove a task from a list.

    \param[in] list     Pointer to a Tasklist.
    \param[in] del_task Task to remove from the list.

    \return FALSE if the task was not on the list, otherwise TRUE.
 */
bool appTaskListRemoveTask(TaskList* list, Task del_task);

/*! \brief Return number of tasks in list.
 
    \param[in] list Pointer to a Tasklist.

    \return uint16 Number of Tasks in the list. Zero if list is set to NULL.
 */
uint16 appTaskListSize(TaskList* list);

/*! \brief Iterate through all tasks in a list.

    Pass NULL to next_task to start iterating at first task in the list.
    On each subsequent call next_task should be the task previously returned
    and appTaskListIterate will return the next task in the list.

    \param[in]     list      Pointer to a Tasklist.
    \param[in,out] next_task Pointer to task from which to iterate.

    \return bool TRUE next_task is returning a Task.
                 FALSE end of list, next_task is not returning a Task.
 */
bool appTaskListIterate(TaskList* list, Task* next_task);

/*! \brief Iterate through all tasks in a list returning a copy of data as well.
    
    Pass NULL to next_task to start iterating at first task in the list.
    On each subsequent call next_task should be the task previously returned
    and appTaskListIterate will return the next task in the list.

    \param[in]     list      Pointer to a Tasklist.
    \param[in,out] next_task Pointer to task from which to iterate.
    \param[out]    data      Pointer into which data associated with the task is copied.

    \return bool TRUE next_task is returning a Task.
                 FALSE end of list, next_task is not returning a Task.
 */
bool appTaskListIterateWithData(TaskList* list, Task* next_task, TaskListData* data);

/*! \brief Iterate through all tasks in a list returning raw data stored in the
    TaskList.

    Useful for efficiently modifying the data for all items in a #TaskList.

    Pass NULL to next_task to start iterating at first task in the list.
    On each subsequent call next_task should be the task previously returned
    and appTaskListIterate will return the next task in the list.

    \param[in]     list      Pointer to a Tasklist.
    \param[in,out] next_task Pointer to task from which to iterate.
    \param[out]    raw_data  Address of pointer to the task's raw data stored by tasklist.

    \return bool TRUE next_task is returning a Task.
                 FALSE end of list, next_task is not returning a Task.

    \note The lifetime of the raw_data pointer is limited. The TaskList module
    reserves the right to invalidate this pointer (e.g. through reallocation)
    in any subsequent TaskList function call.
 */
bool appTaskListIterateWithDataRaw(TaskList* list, Task* next_task, TaskListData** raw_data);

/*! \brief Iterate through all tasks in a list calling handler each iteration.

    \param[in] list     Pointer to a Tasklist.
    \param[in] handler  Function to handle each iteration. If the handler returns
            FALSE, the iteration will stop.
    \param[in] arg      Abstract object to pass to handler every iteration.

    \return TRUE if the iteration completed, FALSE if the iteration stopped because
            a call to handler returned FALSE.
*/
bool appTaskListIterateWithDataRawFunction(TaskList *list, TaskListIterateWithDataRawHandler handler, void *arg);

/*! \brief Determine if a task is on a list.

    \param[in] list         Pointer to a Tasklist.
    \param[in] search_task  Task to search for on list.

    \return bool TRUE search_task is on list, FALSE search_task is not on the list.
 */
bool appTaskListIsTaskOnList(TaskList* list, Task search_task);

/*! \brief Create a duplicate task list.

    \param[in] list Pointer to a Tasklist.

    \return TaskList * Pointer to duplicate task list.
 */
TaskList *appTaskListDuplicate(TaskList* list);

/*! \brief Send a message (with message body) to all tasks in the task list.

    \param[in]  list Pointer to a TaskList.
    \param id   The message ID to send to the TaskList.
    \param[in]  data Pointer to the message content.
    \param size_data The sizeof the message content.
*/
void appTaskListMessageSendWithSize(TaskList *list, MessageId id, void *data, uint16 size_data);

/*! \brief Send a message (with data) to all tasks in the task list.
 
    \param[in] list    Pointer to a TaskList.
    \param     id      The message ID to send to the TaskList.
    \param     message Pointer to the message content.

    \note Assumes id is of a form such that appending a _T to the id creates the
    message structure type string.
*/
#define appTaskListMessageSend(list, id, message) \
    appTaskListMessageSendWithSize(list, id, message, sizeof(id##_T))

/*! \brief Send a message without content to all tasks in the task list.

    \param[in] list Pointer to a TaskList.
    \param     id   The message ID to send to the TaskList.
*/
#define appTaskListMessageSendId(list, id) \
    appTaskListMessageSendWithSize(list, id, NULL, 0)

/*! \brief Get a copy of the data stored in the list for a given task.

    \param[in]  list        Pointer to a Tasklist.
    \param[in]  search_task Task to search for on list.
    \param[out] data        Pointer to copy the associated task data.

    \return bool TRUE search_task is on the list and data copied.
                 FALSE search_task is not on the list.
*/
bool appTaskListGetDataForTask(TaskList* list, Task search_task, TaskListData* data);

/*! \brief Get the address of the data stored in the list for a given task.

    \param[in]  list        Pointer to a Tasklist.
    \param[in]  search_task Task to search for on list.
    \param[out] raw_data    Address of pointer to the associated task data.

    \return bool TRUE search_task is on the list and raw_data address set.
                 FALSE search_task is not on the list.

    \note The lifetime of the raw_data pointer is limited. The TaskList module
    reserves the right to invalidate this pointer (e.g. through reallocation)
    in any subsequent TaskList function call.
*/
bool appTaskListGetDataForTaskRaw(TaskList* list, Task search_task, TaskListData** raw_data);

/*! \brief Set the data stored in the list for a given task.

    \param[in] list        Pointer to a Tasklist.
    \param[in] search_task Task to search for on list.
    \param[in] data        Pointer to new associated task data.

    \return bool TRUE search_task is on the list and data changed.
                 FALSE search_task is not on the list.
*/
bool appTaskListSetDataForTask(TaskList* list, Task search_task, const TaskListData* data);

/*! \brief Determine if the list is one that supports data.

    \param[in] list Pointer to a Tasklist.

    \return bool TRUE list supports data, FALSE list does not support data.
*/
bool appTaskListIsTaskListWithData(TaskList* list);

#endif /* AV_HEADSET_TASKLIST_H */


