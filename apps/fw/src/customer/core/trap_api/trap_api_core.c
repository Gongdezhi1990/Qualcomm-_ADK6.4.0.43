/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "trap_api/trap_api_private.h"

#ifdef DESKTOP_TEST_BUILD
#include "trap_api/csr_stdlib.h"
#else
#include <string.h>
#include <stdlib.h>
#endif

#include <panic.h>
#include <util.h>
#include <os.h>
#include "pmalloc/pmalloc.h"
#include "panic/panic.h"
#include "trap_api/panicdefs.h"
#include "hydra_log/hydra_log.h"
#include "longtimer/longtimer.h"
#include "init/init.h"




#ifdef DESKTOP_TEST_BUILD
#define DESKTOP_NAME_MUNGE(name) name##_csr
#else
#define DESKTOP_NAME_MUNGE(name) name
#endif

#ifdef PMALLOC_TRACE_OWNER_PC_ONLY
/* !!! portability issue
 * inline assembler function to return the caller address
 * This in-line function can only be used early on in a
 * function, certainly before any other function is called.
 */
/*lint -e{*}*/
asm size_t getReturnAddresses(void)
{
    /* using @ { } as return register (instead of r0) allows the
     * compiler to optimize which register is actually used.
     * In all probability, it will be r0, but I don't need to care.
     */
    @{} = rlink;
}
#endif

#ifndef DESKTOP_TEST_BUILD
void *calloc(size_t nitems, size_t size)
{
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    size_t caller = getReturnAddresses();
#endif
    size_t bytes_to_allocate = nitems * size;
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    /* call a traceable zeroing memory allocator */
    uint8* malloced_memory = xzpmalloc_trace(bytes_to_allocate, caller);
#else
    /* call directly to the underlying zeroing allocator */
    uint8* malloced_memory = xzpmalloc(bytes_to_allocate);
#endif
    return (void*)malloced_memory;
}
#endif

void * DESKTOP_NAME_MUNGE(malloc)(size_t sz)
{
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    size_t caller = getReturnAddresses();
    return xpmalloc_trace(sz, caller);
#else
    return xpmalloc(sz);
#endif
}

void DESKTOP_NAME_MUNGE(free)(void * mem)
{
    pfree(mem);
}

void * DESKTOP_NAME_MUNGE(realloc)(void * mem, size_t sz)
{
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    size_t caller = getReturnAddresses();
    return xprealloc_trace(mem, sz, caller);
#else
    return xprealloc(mem, sz);
#endif
}

/*
 * This implementation was initially stolen from vm_trap_core.c in BlueCore with
 * "vm_block_copy" calls replaced with memcpy ones, but logic for doing a
 * safe move "left" was added because we shouldn't rely on memcpy to work in a
 * way that makes this safe.
 */
void * DESKTOP_NAME_MUNGE(memmove)(void * dst, const void * src, size_t len)
{
    if(src < dst)
    {
        /* Checking overlap region and copying them safely */
        while((const char *)src + len > (char *)dst)
        {
            /* calculate the length of safe region to copy */
            size_t safe_region_len = (size_t)((char *)dst - (const char *)src);
            /* overlapping regions, copy blocks which are safe */
            const void * src_ptr;
            void *dst_ptr;

            /* find the safe address to block copy */
            src_ptr = (const char *)src + len - safe_region_len;
            dst_ptr = (char *)dst + len - safe_region_len;

            /* decrement the len before copying the content */
            len -= safe_region_len;
            /* copy the block to safe region */
            memcpy(dst_ptr, src_ptr, safe_region_len);
        }
    }
    else if(src == dst)
    {
        /* There is nothing to do -- tell function that copying is complete*/
        len = 0;
    }
    else /*src > dst*/
    {
        while((char *)dst + len > (const char *)src)
        {
            /* calculate the length of safe region to copy */
            size_t safe_region_len = (size_t)((const char *)src - (char *)dst);

            /* Copy the non-overlapping data from the current src to the current
             * dst and decrement the amount left to copy */
            len -= safe_region_len;
            memcpy(dst, src, safe_region_len);

            /* Move src to the new start of the not-yet-copied region, and
             * dst correspondingly to the end of the just-copied region */
            src = (const char *)src + safe_region_len;
            dst = (char *)dst + safe_region_len;
        }
    }


    if (len != 0)
    {
        /* non-overlapping region -- just block copy */
        memcpy(dst, src, len);
    }
    return dst;
}

void DESKTOP_NAME_MUNGE(putchar)(uint16 c)
{
    /**
     * This is the trap to log a character in the Apps P1 processor.
     */
    DBG_PUTCHAR(c);
}


void Panic(void)
{
    panic_diatribe(PANIC_APP_FAILED, PANIC_APP_GENERIC);
}

void * PanicNull(void *ptr)
{
    if(!ptr)
    {
        panic_diatribe(PANIC_APP_FAILED, PANIC_APP_POINTER_IS_NULL);
    }
    return ptr;
}

void PanicNotNull(const void *ptr)
{
    if(ptr)
    {
        panic_diatribe(PANIC_APP_FAILED, PANIC_APP_POINTER_IS_NOT_NULL);
    }
}

void * PanicUnlessMalloc(size_t sz)
{
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    size_t caller = getReturnAddresses();
    void* ptr = xpmalloc_trace(sz, caller);
    if (ptr == NULL)
    {
        panic(PANIC_HYDRA_PRIVATE_MEMORY_EXHAUSTION);
    }
    return ptr;
#else
    return pmalloc(sz); /* pmalloc panics internally */
#endif
}

uint32 VmGetClock(void)
{
    return get_milli_time();
}

uint32 VmGetTimerTime(void)
{
    return hal_get_reg_timer_time();
}

uint16 VmGetAvailableAllocations(void)
{
#ifdef DESKTOP_TEST_BUILD
    return 0;
#else
    return (uint16)pmalloc_available(0);
#endif
}

void OsInit(void)
{
#ifndef DESKTOP_TEST_BUILD
    initialise_os();
#endif
}

