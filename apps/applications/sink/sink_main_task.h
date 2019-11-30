/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_main_task.h
 
DESCRIPTION
    Write a short description about what the sub module does and how it 
    should be used.
*/

#ifndef SINK_MAIN_TASK_H_
#define SINK_MAIN_TASK_H_

#include <message.h>

typedef struct
{
    TaskData    task;
} hsTaskData;

extern hsTaskData theSink ;

#define sinkGetMainTask() (&theSink.task)

#endif /* SINK_MAIN_TASK_H_ */
