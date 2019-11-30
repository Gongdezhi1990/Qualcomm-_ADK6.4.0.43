/****************************************************************************
 * Copyright (c) 2018 - 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  thread_offload.h
 * \ingroup capabilities
 *
 * Thread offload RPC definitions. <br>
 *
 */

#ifndef THREAD_OFFLOAD_H
#define THREAD_OFFLOAD_H

#include "types.h"
#include "ipc/ipc_audio_thread.h"

static inline unsigned rpc_is_active(void)
{
    return audio_thread_rpc_is_active();
}

static inline void celt_rpc_wrapper(unsigned addr, unsigned r0, unsigned r1, unsigned r2, unsigned r3)
{
    if (audio_thread_offload_is_active())
    {
        /* Call the common RPC function */
        audio_thread_rpc_call(addr, r0, r1, r2, r3);
    }
    else
    {
        /* RPC not enabled, so just call the target function directly */
        unsigned (*f)(unsigned r0, unsigned r1, unsigned r2, unsigned r3);
        f = (unsigned (*)(unsigned r0, unsigned r1, unsigned r2, unsigned r3))(uintptr_t)addr;
        f(r0, r1, r2, r3);
    }
}


#endif /* THREAD_OFFLOAD_H */
