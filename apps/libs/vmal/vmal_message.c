/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal_message.c

DESCRIPTION
    Each Source has an associated Sink, therefore Source functions can use the 
    associated Sink to access the Message Task.
*/

#include <vmal.h>
#include <stream.h>

Task VmalMessageSinkTask(Sink sink, Task task)
{
    return MessageStreamTaskFromSink(sink, task);
}

Task VmalMessageSinkGetTask(Sink sink)
{
    return MessageStreamGetTaskFromSink(sink);
}

Task VmalMessageSourceTask(Source source, Task task)
{
    return MessageStreamTaskFromSink(StreamSinkFromSource(source), task);
}

Task VmalMessageSourceGetTask(Source source)
{
    return MessageStreamGetTaskFromSink(StreamSinkFromSource(source));
}
