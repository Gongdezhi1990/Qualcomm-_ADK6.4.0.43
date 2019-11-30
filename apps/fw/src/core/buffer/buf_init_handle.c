/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "buffer/buffer_private.h"

#ifdef DESKTOP_TEST_BUILD
#define BUF_NEW(type) pnew(type)
#define BUF_FREE(ptr) pfree(ptr)
#else
#define BUF_NEW(type) ipc_snew(type)
#define BUF_FREE(ptr) ipc_send_sfree(ptr)
#endif

void buf_init_from_handle(mmu_buffer_size msize, mmu_handle h, BUFFER *buf)
{
    memset((void *)buf, 0, sizeof(BUFFER));

    buf->handle = h;
    /*
     * Get the size:  may be 64K, so this has to be 32 bits.
     * However, the mask is (by the definition of the BUFFER API) a
     * 16-bit quantity.
     */
    buf->size_mask = (uint16)((1UL << (MMU_LOG_PAGE_BYTES + msize)) - 1UL);
    /*lint -e429 There's no custodial pointer loss here, thanks very much */
}

BUFFER *buf_new_from_handle(mmu_buffer_size msize, mmu_handle h)
{
    BUFFER *buf;
    buf = BUF_NEW(BUFFER);
    buf_init_from_handle(msize, h, buf);
    return buf;
}

mmu_handle buf_free_from_handle(BUFFER *buf)
{
    mmu_handle hdl = buf->handle;
    BUF_FREE(buf);
    return hdl;
}
