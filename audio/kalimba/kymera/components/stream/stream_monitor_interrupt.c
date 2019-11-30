/****************************************************************************
 * Copyright (c) 2012 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file stream_monitor_interrupt.c
 * \ingroup stream
 *
 * Code for setting up and handling the RD_STREAM_MON and WR_STREAM_MON interrupts.
 */

/****************************************************************************
Include Files
*/
#include "stream_private.h"
#include "hal_audio_monitor.h"
#include "platform/pl_trace.h"

/****************************************************************************
Private Variable Definitions
*/
static void stream_monitor_service_routine(void* kick_object);

/****************************************************************************
Private Function Definitions
*/
/**
 * \brief Stream monitor interrupt call back routine that gets called from the ISR
 *
 * \param kick_object kick object associated with the stream monitor
 *
 */
void stream_monitor_service_routine(void* kick_object)
{
    KICK_OBJECT* ko = (KICK_OBJECT*)kick_object;
    PL_PRINT_P0(TR_STREAM, "stream_service_routine\n");

    /* Need to check whether our kick object is NULL, because unfortunate
     * timing could see an interrupt being handled just after we disabled
     * the monitor. */
    if (ko != NULL)
    {
        /* We need to kick the kick object, not the endpoint, because it's
         * the kick object which knows who has responsibility for the chain.
         */
        kick_obj_kick(ko);
    }
}

/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 *
 * stream_monitor_int_rd_enable
 *
 */
bool stream_monitor_int_rd_enable_with_event_type(ENDPOINT* ep, mmu_handle handle,
                                                  KICK_OBJECT *ko,
                                                  KICK_EVENT_TYPE etype)
{
    int octets_per_sample = 1;
    mmu_index index;
    uint32 audio_format;

    /* Software patchpoint just in case */
    patch_fn_shared(stream);

    /* If the endpoint isn't supported, fail hard. */
    if (ep->stream_endpoint_type != endpoint_audio)
    {
        panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_ENDPOINT_NOT_SUPPORTED, ep->stream_endpoint_type);
    }

    PL_PRINT_P2(TR_STREAM, "stream_monitor_int_rd_enable: endpoint 0x%x at threshold %d \n",
            stream_external_id_from_endpoint(ep), ep->state.audio.monitor_threshold);

    index = mmu_handle_index(handle);

    stream_get_endpoint_config(ep, EP_DATA_FORMAT, &audio_format);
    if ((AUDIO_DATA_FORMAT)audio_format != AUDIO_DATA_FORMAT_8_BIT)
    {
        /* Threshold is supplied in samples, but the audio HW measures
         * it in octets, so do the conversion here. */
        octets_per_sample = 2;
    }

    return hal_audio_monitor_rd_enable(index, ep->state.audio.monitor_threshold, (MONITOR_EVENT_TYPE)etype,
            stream_monitor_service_routine, (void*)ko, octets_per_sample);
}

/****************************************************************************
 *
 * stream_monitor_int_wr_enable
 *
 */
bool stream_monitor_int_wr_enable_with_event_type(ENDPOINT* ep, mmu_handle handle,
                                                  KICK_OBJECT *ko,
                                                  KICK_EVENT_TYPE etype)
{
    int octets_per_sample = 1;
    mmu_index index;
    uint32 audio_format;

    /* Software patchpoint just in case */
    patch_fn_shared(stream);

    /* If the endpoint isn't supported, fail hard. */
    if (ep->stream_endpoint_type != endpoint_audio)
    {
        panic_diatribe(PANIC_AUDIO_STREAM_MONITOR_ENDPOINT_NOT_SUPPORTED, ep->stream_endpoint_type);
    }

    PL_PRINT_P2(TR_STREAM, "stream_monitor_int_wr_enable: endpoint 0x%x at threshold %d \n",
            stream_external_id_from_endpoint(ep), ep->state.audio.monitor_threshold);

    index = mmu_handle_index(handle);

    stream_get_endpoint_config(ep, EP_DATA_FORMAT, &audio_format);
    if ((AUDIO_DATA_FORMAT)audio_format != AUDIO_DATA_FORMAT_8_BIT)
    {
        /* Threshold is supplied in samples, but the audio HW measures
         * it in octets, so do the conversion here. */
        octets_per_sample = 2;
    }

    return hal_audio_monitor_wr_enable(index, ep->state.audio.monitor_threshold, (MONITOR_EVENT_TYPE)etype,
            stream_monitor_service_routine, (void*)ko, octets_per_sample);
}

#ifdef INSTALL_SPDIF
/****************************************************************************
 *
 * stream_spdif_monitor_int_wr_enable
 *
 */
bool stream_spdif_monitor_int_wr_enable_with_event_type(ENDPOINT* ep, mmu_handle handle,
                                                        KICK_OBJECT *ko,
                                                        KICK_EVENT_TYPE etype)
{
    int octets_per_sample = 2;

    /* Software patchpoint just in case */
    patch_fn_shared(stream);

    PL_PRINT_P2(TR_STREAM, "stream_monitor_spdif_int_wr_enable: endpoint 0x%x at threshold %d \n",
            stream_external_id_from_endpoint(ep), ep->state.spdif.monitor_threshold);


    return hal_audio_monitor_wr_enable(mmu_handle_index(handle), ep->state.spdif.monitor_threshold, (MONITOR_EVENT_TYPE)etype,
            stream_monitor_service_routine, (void*)ko, octets_per_sample);
}
#endif /* #ifdef INSTALL_SPDIF */
/****************************************************************************
 *
 * stream_monitor_int_rd_disable
 *
 */
bool stream_monitor_int_rd_disable(KICK_OBJECT *ko)
{
    /* Software patchpoint just in case */
    patch_fn_shared(stream);

    PL_PRINT_P1(TR_STREAM, "stream_monitor_int_rd_disable: endpoint 0x%x\n", stream_external_id_from_endpoint(kick_get_sched_ep(ko)));

    /* HAL will know which monitor slot to reset */
    return hal_audio_monitor_rd_disable((void*)ko);
}

/****************************************************************************
 *
 * stream_monitor_int_wr_disable
 *
 */
bool stream_monitor_int_wr_disable(KICK_OBJECT *ko)
{
    /* Software patchpoint just in case */
    patch_fn_shared(stream);

    PL_PRINT_P1(TR_STREAM, "stream_monitor_int_wr_disable: endpoint 0x%x\n", stream_external_id_from_endpoint(kick_get_sched_ep(ko)));

    /* HAL will know which monitor slot to reset */
    return hal_audio_monitor_wr_disable((void*)ko);
}

