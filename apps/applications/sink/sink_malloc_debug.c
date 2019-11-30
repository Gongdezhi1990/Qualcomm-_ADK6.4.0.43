/*
Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd.

*/

/*!
@file
@ingroup sink_app
@brief   
    Debug functions for adding trace / debug functionality around memory 
    allocation and free.
        
    This is the source impplementation of functions to support the macros 
    in the header.

*/

/****************************************************************************
    Header files
*/
#include "sink_malloc_debug.h"

#include <string.h>

#ifdef DEBUG_MALLOC
#include<vm.h>

void *SinkMallocZDebug ( const char file[], int line, size_t pSize )
{
    void *allocated_ptr = SinkMallocDebug(file,line,pSize);

    if (allocated_ptr)
    {
        memset(allocated_ptr,0,pSize);
    }

    return allocated_ptr;
}

void *SinkReallocDebug(const char file[], int line,void *ptr,size_t size)
{
    void *reallocated_ptr;

    printf("REALLOC:+%s,l[%d] a[%d] s[%d]",file , line ,(uint16)VmGetAvailableAllocations(), size );

    reallocated_ptr = realloc(ptr,size);
    printf("@[0x%p]\n", (void*)reallocated_ptr);

    return reallocated_ptr;
}

void * SinkMallocDebug ( const char file[], int line , size_t pSize )
{
    static uint32 lSize = 0 ;
    static uint16 lCalls = 0 ;
    void * lResult;

    lCalls++ ;
    lSize += pSize ;
    printf("+%s,l[%d]c[%d] t[%ld] a[%d] s[%d]",file , line ,lCalls, lSize , (uint16)VmGetAvailableAllocations(), pSize );

    lResult = malloc ( pSize ) ;

    printf("@[0x%p]\n", (void*)lResult);

    return lResult ;

}

void SinkFreeDebug ( const char file[], int line, void * ptr )
{
    static uint16 lCalls = 0 ;
    lCalls++ ;

    printf("-%s,l[%d]c[%d] a[%d] @[0x%p]\n",file , line ,lCalls, (uint16)VmGetAvailableAllocations()-1, (void*)ptr);

    free( ptr ) ;
}

#else /* DEBUG_MALLOC */

void *SinkMallocZ ( size_t pSize ) 
{
    void *allocated_ptr = malloc(pSize);

    if (allocated_ptr)
    {
        memset(allocated_ptr,0,pSize);
    }

    return allocated_ptr;
}

#endif /* DEBUG_MALLOC */

