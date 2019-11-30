/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */


#include "ipc/ipc_private.h"
#include "buffer/buffer.h"
#include "appcmd/appcmd.h"
#define IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES
#define IO_DEFS_MODULE_K32_MISC
#include "io/io_map.h"
#include "hal/hal_macros.h"
#include "hydra_log/hydra_log.h"
#include "hydra/hydra_macros.h"
#include "hal/hal_macros.h"
#include "ipc/ipc.h"
#include "sched/sched.h"
#include "pmalloc/pmalloc.h"
#include "panic/panic.h"

#ifdef FW_IPC_UNIT_TEST
/**
 * Possible test results
 * \ingroup ipc_test
 */
typedef enum
{
    IPC_TEST_INCOMPLETE = 0,
    IPC_TEST_PASSED = 1,
    IPC_TEST_FAILED = 2
} IPC_TEST_RESULT;

/**
 * Run-time data for the shared memory test
 * \ingroup ipc_test
 */
static struct
{
    uint32 num_loops; /**< Number of loops through the buffer that we're doing */
    uint32 write_chunk_size; /**< Number of bytes to write at once */
    uint32 read_chunk_size; /**< Number of bytes to read at once */
    taskid bg_int_id; /**< ID of the dynamically-created test bg interrupt */
    BUFFER *buf; /**< \c BUFFER the test is reading and writing */
    uint32 iloop; /**<  Current loop */
    IPC_TEST_RESULT result; /**< Current result value */
} smem_test_data;

/**
 * List of test phases
 * \ingroup ipc_test
 */
typedef enum
{
    IPC_TEST_SMEM_PHASE_START,
    IPC_TEST_SMEM_PHASE_LOOP,
    IPC_TEST_SMEM_PHASE_CLEANUP
} IPC_TEST_SMEM_PHASE;

/* QCC512x, QCC302x, and QCC303x series has less pool memory and 
 * so the loop needs to yield more frequently 
 */
#define IPC_TEST_MAX_LOOP_COUNT (200)
#define IPC_TEST_YIELD_MULTIPLE (5)


/**
 * Submit a packet to the Appcmd response buffer indicating success or failure
 * \ingroup ipc_test
 */
static void ipc_smem_report_completion(void)
{
    uint32 res = (uint32)smem_test_data.result;
    appcmd_add_test_rsp_packet(APPCMD_TEST_ID_IPC,
                                APPCMD_RESPONSE_SUCCESS,
                                &res, 1);
}

/**
 * Handle failure of the test
 * \ingroup ipc_test
 */
static void ipc_smem_test_failed(void)
{
    smem_test_data.result = IPC_TEST_FAILED;
    ipc_smem_report_completion();
    /* Trigger cleanup */
    GEN_BG_INT_FROM_ID(smem_test_data.bg_int_id);
}

/**
 * Handle successful completion of the test
 * \ingroup ipc_test
 */
static void ipc_smem_test_passed(void)
{
    smem_test_data.result = IPC_TEST_PASSED;
    ipc_smem_report_completion();
    /* Trigger cleanup */
    GEN_BG_INT_FROM_ID(smem_test_data.bg_int_id);
}

/**
 * Test handler
 * @param phase  Phase of the test to run
 * @param params Test parameters (only used during \c IPC_TEST_SMEM_PHASE_START)
 * @param results Not used
 * \ingroup ipc_test
 */
static void ipc_test_shared_mem_handler(IPC_TEST_SMEM_PHASE phase,
                                        uint32 *params,
                                        uint32 *results);

/**
 * Background interrupt handler.  Determines which phase to kick off next: will
 * be \c IPC_TEST_SMEM_PHASE_LOOP unless they've all completed, in which case
 * it will be \c IPC_TEST_SMEM_PHASE_CLEANUP
 * @param ppriv Private memory area, unused
 * \ingroup ipc_test
 */
static void smem_test_bg_int_handler(void **ppriv)
{
    uint16f raised;
    (void)get_highest_bg_int(smem_test_data.bg_int_id, &raised);
    UNUSED(ppriv);
    UNUSED(raised);

    if (smem_test_data.result == IPC_TEST_INCOMPLETE)
    {
        ipc_test_shared_mem_handler(IPC_TEST_SMEM_PHASE_LOOP, NULL, NULL);
    }
    else
    {
        ipc_test_shared_mem_handler(IPC_TEST_SMEM_PHASE_CLEANUP, NULL, NULL);
    }
}


static void ipc_test_shared_mem_handler(IPC_TEST_SMEM_PHASE phase,
                                        uint32 *params,
                                        uint32 *results)
{
    IPC_MMU_HANDLE_ALLOC_REQ buf_hdl_req;
    IPC_MMU_HANDLE_ALLOC_RSP buf_hdl_rsp;
    mmu_buffer_size buf_size;

    UNUSED(results);/* These are reported via the rsp_buf */

    switch(phase)
    {
    case IPC_TEST_SMEM_PHASE_START:

        memset(&smem_test_data, 0, sizeof(smem_test_data));
        smem_test_data.num_loops = params[0];
        smem_test_data.write_chunk_size = params[1];
        smem_test_data.read_chunk_size = params[2];
        buf_size = (mmu_buffer_size)params[3];
        L2_DBG_MSG3("Starting IPC shared mem test with buf_size %d, "
                "write chunk %d, read chunk %d", params[3], params[1], params[2]);

        /* Allocate an MMU.  P0 simply creates one for itself, whereas P1 must
         * request one from P0.  For simplicity it performs a blocking receive. */
        /* Request an MMU handle from P1, blocking on the response since we've got
         * nothing else to do at this point */
        buf_hdl_req.size = buf_size;
        ipc_send(IPC_SIGNAL_ID_MMU_HANDLE_ALLOC_REQ, &buf_hdl_req,
                                                           sizeof(buf_hdl_req));
        (void)ipc_recv(IPC_SIGNAL_ID_MMU_HANDLE_ALLOC_RSP, &buf_hdl_rsp);
        if (buf_hdl_rsp.hdl == MMU_INDEX_NULL)
        {
            L1_DBG_MSG("ipc_test_shared_mem_handler: no handle available");
            ipc_smem_test_failed();
            return;
        }
        smem_test_data.buf = buf_new_from_handle(buf_size, buf_hdl_rsp.hdl);

        /* Create a scheduler task for this test */
        (void)create_uncoupled_bgint(MID_PRIORITY, NULL,
                                     smem_test_bg_int_handler,
                                     &smem_test_data.bg_int_id);
        GEN_BG_INT_FROM_ID(smem_test_data.bg_int_id);
        break;

    case IPC_TEST_SMEM_PHASE_LOOP:
    {
        uint8 *buf_write;
        const uint8 *buf_read;
        BUFFER *buf = smem_test_data.buf;
        uint16f i;
        uint16f while_loop_count = 0;

         /* Loop writing and reading data in chunks */
        while (smem_test_data.iloop < smem_test_data.num_loops)
        {
            if (BUF_GET_FREESPACE(buf) >= smem_test_data.write_chunk_size)
            {
                buf_write = buf_raw_write_only_map_8bit(buf);
                for (i = 0; i < smem_test_data.write_chunk_size; ++i)
                {
                    buf_write[i] = (uint8)((buf->index + i) & buf->size_mask);
                }
                buf_raw_write_update(buf, smem_test_data.write_chunk_size);
            }
            if (BUF_GET_AVAILABLE(buf) >= smem_test_data.read_chunk_size)
            {
                buf_read = buf_raw_read_map_8bit(buf);
                for (i = 0; i < smem_test_data.read_chunk_size; ++i)
                {
                    if (buf_read[i] != (uint8)((buf->outdex + i) &
                                                                buf->size_mask))
                    {
                        L1_DBG_MSG("ipc_test_shared_mem_handler: buffer "
                                    "contents bad");
                        L1_DBG_MSG2("Read 0x%02x, expected 0x%02x",
                                    buf_read[i],
                                   (uint8)((buf->outdex + i) & buf->size_mask));
                        ipc_smem_test_failed();
                        return;
                    }
                }
                buf_raw_read_update(buf, smem_test_data.read_chunk_size);
                buf_raw_update_tail_free(buf, buf->outdex);
                if (buf->outdex < smem_test_data.read_chunk_size)
                {
                    /* We've wrapped. */
                    smem_test_data.iloop++;
                    if (smem_test_data.iloop == smem_test_data.num_loops)
                    {
                        ipc_smem_test_passed();
                    }
                    /* Report progress periodically */
                    else if (smem_test_data.iloop %
                                            (smem_test_data.num_loops/10) == 0)
                    {
                        appcmd_add_test_rsp_packet(APPCMD_TEST_ID_IPC,
                                                APPCMD_RESPONSE_RESULT_PENDING,
                                                &smem_test_data.iloop, 1);
                    }
                }
                /* We need to drop out frequently to give other background
                 * tasks a chance */
                if (buf->outdex % (IPC_TEST_YIELD_MULTIPLE*smem_test_data.read_chunk_size) <
                                            smem_test_data.read_chunk_size)
                {
                    GEN_BG_INT_FROM_ID(smem_test_data.bg_int_id);
                    return; /* We'll let the scheduler call another loop */
                }
            }
            while_loop_count++;
            if (while_loop_count > IPC_TEST_MAX_LOOP_COUNT)
            {
                GEN_BG_INT_FROM_ID(smem_test_data.bg_int_id);
                return; /* We'll let the scheduler call another loop */
            }
        }
    }
        break;

    case IPC_TEST_SMEM_PHASE_CLEANUP:
    {
        L2_DBG_MSG("Test complete: cleaning up");
        {
            IPC_MMU_HANDLE_FREE mmu_hdl_free;
            mmu_hdl_free.hdl = buf_free_from_handle(smem_test_data.buf);
            ipc_send(IPC_SIGNAL_ID_MMU_HANDLE_FREE, &mmu_hdl_free,
                                                        sizeof(mmu_hdl_free));
        }
        delete_task(smem_test_data.bg_int_id);
    }
    }
}

/**
 * Enumeration of test modes
 */
typedef enum
{
    HI_PRI_MODE_CALL_FAST_TRAP = 0, /**< Have the handler call a fast trap */
    HI_PRI_MODE_CALL_SLOW_TRAP = 1  /**< Have the handler call a slow trap */
} IPC_TEST_HI_PRI_MODE;

/**
 * Static configuration/result data for the hi_pri_stream_handler_test
 */
static struct
{
    IPC_TEST_HI_PRI_MODE mode; /*< Indicates what the high-priority handler should do */
    uint32 value;     /*< Arbitrary data that the handler interprets based on
                          the mode it's in */
    uint32 outcome;   /*< Arbitrary data that the handler writes when it is
                          done and the debugger interprets */
} hi_pri_data;


/**
 * Configure the high-priority handler
 * \param mode Value to set in the \c hi_pri_data mode field
 * \param value Value to set in the \c hi_pri_data value field
 */
static void ipc_test_configure_hi_pri_hdlr(uint32 mode, uint32 value)
{
    UNUSED(value);
    hi_pri_data.mode = (IPC_TEST_HI_PRI_MODE)mode;
    hi_pri_data.value = value;
}

void ipc_test_high_priority_handler(Task task, MessageId msg_id, Message msg)
{
    UNUSED(task);
    UNUSED(msg_id);
    UNUSED(msg);
    switch(hi_pri_data.mode)
    {
    case HI_PRI_MODE_CALL_FAST_TRAP:
        hi_pri_data.outcome = 0;
        /* Note: we pass 1 as if it were a pre-allocated pointer to space for
         * the message body, but it's safe because the message body has
         * zero length */
        (void)ipc_recv_atomic((IPC_SIGNAL_ID)hi_pri_data.value, (void *)1);
        hi_pri_data.outcome = 1;
        break;
    case HI_PRI_MODE_CALL_SLOW_TRAP:
        hi_pri_data.outcome = 0;
        /* Note: we pass 1 as if it were a pre-allocated pointer to space for
         * the message body, but it's safe because the message body has
         * zero length */
        (void)ipc_recv((IPC_SIGNAL_ID)hi_pri_data.value, (void *)1);
        hi_pri_data.outcome = 1;
        break;
    }
}



/**
 * Test master
 * @param command  Must be APPCMD_TEST_ID_IPC_TEST
 * @param params   Parameters alongside the test ID; first of these must be
 * sub-test
 * @param results  Storage for results to pass back to client
 * @return Response
 * \ingroup ipc_test
 */
static APPCMD_RESPONSE ipc_test_appcmd_handler(APPCMD_TEST_ID command,
                                               uint32 *params,
                                               uint32 *results)
{
    APPCMD_TEST_IPC_TEST_ID subcmd = (APPCMD_TEST_IPC_TEST_ID)params[0];

    UNUSED(command);

    switch(subcmd)
    {
    case APPCMD_IPC_TEST_ID_SHARED_MEM:
        ipc_test_shared_mem_handler(IPC_TEST_SMEM_PHASE_START, &params[1],
                                                                      results);
        return APPCMD_RESPONSE_RESULT_PENDING;
    case APPCMD_IPC_TEST_ID_HI_PRI_HDLR:
        ipc_test_configure_hi_pri_hdlr(params[1], params[2]);
        return APPCMD_RESPONSE_SUCCESS;

    default:
        L1_DBG_MSG1("ipc_test_handler: subcmd %d not recognised", subcmd);
        return APPCMD_RESPONSE_UNIMPLEMENTED;
    }
}

void ipc_test_init(void)
{
    L2_DBG_MSG("ipc_test: adding handler");
    if (!appcmd_add_test_handler(APPCMD_TEST_ID_IPC, ipc_test_appcmd_handler))
    {
        L0_DBG_MSG("Failed to add IPC test handler!");
    }
}

#endif /* FW_IPC_UNIT_TEST */


