/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 *
 * \defgroup IPC audio Thread
 * \ingroup IPC
 *
 * IPC audio Thread
 *
 */
#ifndef IPC_AUDIO_THREAD_OFFLOAD_H
#define IPC_AUDIO_THREAD_OFFLOAD_H

extern void audio_thread_rpc_call(unsigned addr, unsigned r0, unsigned r1, unsigned r2, unsigned r3);
extern void audio_thread_copy_fft_twiddle(void);
extern void audio_thread_set_up_aptx_offload(void);
extern bool audio_thread_rpc_is_active(void);
extern bool audio_thread_offload_is_active(void);
extern bool audio_thread_offload_is_enabled(void);
extern uintptr_t audio_thread_get_rpc_active(void);

#endif 
