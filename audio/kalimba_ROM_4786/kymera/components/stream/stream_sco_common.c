/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_sco_common.c
 * \ingroup stream
 *
 * stream sco type file. <br>
 * This file contains stream functions for sco endpoints. <br>
 *
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"
#include "cbops_mgr/cbops_mgr.h"
#include "opmgr/opmgr_endpoint_override.h"


/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Macro Declarations
*/
/* Macros for bt clock manipulation and comparison */
/* This file deals with 32 bit bt clocks; outside they're 28 bits. */
#define BTCLKMASK       (0x0FFFFFFFUL)
#define uint32_to_uint28(c)     ((c) & BTCLKMASK)
#define btclock_rawadd(x, y) ((x) + (y))
#define btclock_add(x, y) uint32_to_uint28(btclock_rawadd((x), (y)))

#ifdef INSTALL_SCO_EP_CLRM
/** SCO packet rates are measured as if they were 8k sample rates */
#define SCO_RM_PSEUDO_SAMPLE_RATE   (8000)
#define SCO_RM_PSEUDO_SAMPLE_PERIOD (125)
#endif /* INSTALL_SCO_EP_CLRM */

/****************************************************************************
Private Variable Definitions
*/
#ifdef DEBUG_KICK_TIMERS
#define DEBUG_KICK_TIME_MAX 10
TIME debug_from_air_kick_time[DEBUG_KICK_TIME_MAX];
static int debug_kick_time_count;
#endif

/****************************************************************************
Private Function Declarations
*/
#ifdef INSTALL_SCO_EP_CLRM
static bool get_sco_rate_measurement(ENDPOINT *ep);
#endif /* INSTALL_SCO_EP_CLRM */
static int get_sco_rate(ENDPOINT *ep);
static bool is_locally_clocked(ENDPOINT *ep);

/****************************************************************************
Private Function Definitions
*/
/**
 * \brief Calculates the first kick time of a chain containing a stream and a
 * sco endpoint. The function takes into account the effect of the wallclock
 * and the current TIMER_TIME to ensure the kick is accurately placed
 * just in the future.
 *
 * \param data_due A pointer to the time the kick is due without compensation.
 * \param hci_handle The hci_handle of the sco link.
 * \param ep_dir The direction of the sco endpoint in this chain sink/source.
 * \param initial_wallclock_value used for debugging.
 *
 * \return The time to schedule the timer for the first kick of the chain.
 */
static TIME stream_sco_start_core(TIME *data_due, unsigned hci_handle,
        ENDPOINT_DIRECTION ep_dir
#ifdef DEBUG_KICK_TIMERS
        ,int32 *initial_wallclock_value
#endif
        )
{
    unsigned tesco;
    unsigned n;
    ENDPOINT *other_sco_e;
    TIME fire_time;
    TIME_INTERVAL project_fwd;
    TIME_INTERVAL delta;

    TIME now = hal_get_time();

    int32 wallclock_value = sco_wallclock_get(hci_handle);
    patch_fn_shared(stream_sco);

    /* If the sco chain in the other direction is already running we need to
     * make sure that we schedule off the same packet as the other chain to
     * avoid scheduling contention.
     */
    tesco = sco_tesco_get(hci_handle);
    if (SINK == ep_dir)
    {
        if (sco_from_air_endpoint_run_state_get(hci_handle) == CHAIN_RUNNING)
        {
            other_sco_e = (ENDPOINT *)sco_from_air_endpoint_get(hci_handle);
            /* This will never be too far in the future as data_due is calculated
             * from the next_slot_time given in sco_params and is static, hence
             * the chain that has already started is either scheduled off the
             * same slot or one in the future relative to next_slot_time.
             *
             * We use the next from-air slot not the current one as it may well
             * result in a to-air time that is in the past and the logic below
             * that ensures the fire-time is in the future will kick in */
            TIME next_from_air = other_sco_e->state.sco.data_avail_time +
                    STREAM_KICK_PERIOD_TO_USECS(other_sco_e->state.sco.kick_period);
            if (*data_due + tesco < next_from_air)
            {
                delta = next_from_air - *data_due;
                /* (delta/tesco) is an integer divide so the tesco's don't cancel */
                *data_due += (TIME_INTERVAL)tesco * (delta / tesco);
            }
        }
    }
    else
    {
        if (sco_to_air_endpoint_run_state_get(hci_handle) == CHAIN_RUNNING)
        {
            other_sco_e = (ENDPOINT *)sco_to_air_endpoint_get(hci_handle);
            /* This will never be too far in the future as data_due is calculated
             * from the next_slot_time given in sco_params and is static, hence
             * the chain that has already started is either scheduled off the
             * same slot or one in the future relative to next_slot_time. */
            if (*data_due < other_sco_e->state.sco.data_avail_time)
            {
                delta = other_sco_e->state.sco.data_avail_time - *data_due;
                /* (delta/tesco) is an integer divide so the tesco's don't cancel */
                *data_due += (TIME_INTERVAL)tesco * ((delta / tesco) + 1);
            }
        }

        /* There is a horrible scenario with esco where the retry window can
         * consume all the non-reserved slots and we'll end up scheduling
         * from-air processing during the next packet's reserved Rx slot.
         * See B-132690 for all the detail, there is no bullet proof solution
         * but we add an extra 2 slots to the scheduled run time as this pushes
         * us outside the reserved slot and gives us improved performance.
         *
         *
         * It would be nice if this could go in the from-air transform specific
         * code but I think if you do it before the alignment step above the
         * alignment process can go wrong so it has to go here. Unless you can
         * prove otherwise then it lives here.
         */
        /* The only way that from-air latency is greater than Tsco is if this
         * situation arises. The actual check is from-air latency + 1 slot
         * (625us) in case we are slave as Rx activities occur 1 slot earlier
         * relative to the master slot in this case.*/
        if (sco_from_air_latency_get(hci_handle) + US_PER_SLOT > tesco)
        {
            *data_due += US_PER_SLOT * 2;
        }
    }

    /* Calculate when the first kick is due (with and without the
     * wallclock). The without wallclock calculation is needed
     * for future scheduling. The wallclock represents the displacement of the
     * BT clock ahead of TIMER_TIME hence it is subtracted from the data_due
     * time. data_due is the BT clock time (in us) that the data is due to be
     * provided/received in the sco buffer.
     */

#ifdef DEBUG_KICK_TIMERS
    *initial_wallclock_value = wallclock_value;
#endif

    fire_time = time_sub(*data_due, wallclock_value);
    delta = 0;
    /* If the time is in the past then bring it into the future, calculate
     * the required delta here */
    if (time_le(fire_time, now) )
    {
        delta = time_sub(now, fire_time);
    }
    /* If the time is more than three Tsco periods in the future reduce it to
     * a single Tsco period so we don't wait around to start. (In this case
     * TIMER_TIME has probably wrapped since we received sco_params. It is
     * possible that BT gives us a start time more than a single Tsco period in the
     * future so we give a little room for manoeuvre, as if we compensate for a
     * wrap that hasn't occurred our kicks will end up out of sync with the BT radio!
     *
     * The maths here is:
     *
     * Work out the displacement of firetime relative to current
     * time.   DELTA = CURRENT_TIME - FIRE_TIME
     *
     * This results in a negative number; the displacement (back) to
     * a fire time that is 'now'. We are going back because the timer
     * has wrapped, so we actually want the delta to be the
     * equivalent displacement into the future to account for the
     * wrap. This is achieved by adding the size of the
     * timer. This gives a positive time that is in the future.
     *      DELTA = DELTA + TIMER_SIZE
     */
    else if (time_gt(fire_time, time_add(now, 3 * tesco)))
    {
        delta = time_sub(now, fire_time);
        delta += MAX_TIME + 1;
    }
    if (0 != delta)
    {
        /* Work out how many tesco periods will put the fire time into the
         * future. Add 1 to the result as it probably wasn't an exact
         * division and even if it was we need a bit of time to schedule
         * the timer. */

        /* delta is a 32-bit time interval, and tesco is at least 1250us
         * so the result is guaranteed to fit in 24 bits
         */
        n = (unsigned)(delta / tesco + 1);

        /* How far into the future does the fire time need to be projected
         * N.B. (delta/tesco) * tesco != delta */
        project_fwd = n * (TIME_INTERVAL)tesco;
        /* We do the maths on the values with and
         * without the wallclock value. We need the with wallclock value to
         * be ahead of TIMER_TIME and the without wallclock value for future
         * kick scheduling. */
        *data_due = time_add(*data_due, project_fwd);
        fire_time = time_add(fire_time, project_fwd);
    }

    return fire_time;
}

/****************************************************************************
Public Function Definitions
*/


/****************************************************************************
 *
 * stream_sco_get_hci_handle
 *
 */
unsigned int stream_sco_get_hci_handle(ENDPOINT *endpoint)
{
    /* Internally called so not checking if the pointer is NULL */
    if (endpoint->stream_endpoint_type != endpoint_sco)
    {
        panic_diatribe(PANIC_AUDIO_STREAM_INVALID_SCO_ENDPOINT,
                       stream_external_id_from_endpoint(endpoint));
    }
    /* the hci handle is the key */
    return endpoint->key;
}


/****************************************************************************
 *
 * stream_sco_params_update
 *
 */
void stream_sco_params_update(unsigned int hci_handle)
{
    ENDPOINT *sco_ep_source, *sco_ep_sink;
    KICK_OBJECT *ko;
    bool source_stopped = FALSE;
    bool sink_stopped = FALSE;
    bool source_connected = TRUE;
    bool sink_connected = TRUE;
    patch_fn_shared(stream_sco);
    /* Provoke streams to recalculate it's kick timing for the source and sink.
     * Priming and ratematching are recalculated here as well.
     *
     * Both chains must be stopped before moving to the new
     * parameters, otherwise the second may see the wallclock change vastly
     * under its feet.
     *
     * It is also necessary to stop the chains so that priming and ratematching
     * levels can be determined given the new parameters.
     */
    sco_ep_source = sco_from_air_endpoint_get(hci_handle);
    /* Stop the source endpoint if it exists and is connected */
    if (NULL == sco_ep_source || NULL == sco_ep_source->connected_to)
    {
        source_connected = FALSE;
    }
    else
    {
        source_stopped = sco_ep_source->functions->stop(sco_ep_source);
    }

    /* Stop the sink endpoint if it exists and is connected. Then update it to
     * the new parameters, which is now safe as the source is also stopped. */
    sco_ep_sink = sco_to_air_endpoint_get(hci_handle);
    if (NULL == sco_ep_sink || NULL == sco_ep_sink->connected_to)
    {
        sink_connected = FALSE;
    }
    else
    {
        sink_stopped = sco_ep_sink->functions->stop(sco_ep_sink);
        set_timing_information_for_real_sink(sco_ep_sink);
    }
    if ((!source_connected) && (!sink_connected))
    {
        /* If neither is connected up yet then there is nothing worth doing.
         * Hopefully the compiler optimises this collection of if statements
         * somewhat.
         */
        return;
    }

    /* reschedule the source now, it's safe because the sink is stopped. */
    if (source_connected)
    {
        set_timing_information_for_real_source(sco_ep_source);

    /* Start the endpoints that we stopped. They are only marked as stopped if
     * they exist and are connected in the first place so don't need to check
     * that too */
        if (source_stopped)
        {
            ko = kick_obj_from_sched_endpoint(sco_ep_source);
            sco_ep_source->functions->start(sco_ep_source, ko);
        }
    }
    if (sink_stopped)
    {
        ko = kick_obj_from_sched_endpoint(sco_ep_sink);
        sco_ep_sink->functions->start(sco_ep_sink, ko);
    }

    return;
}


/****************************************************************************
Private Function Definitions
*/

/****************************************************************************
 *
 * set_from_air_sco_info_cback
 *
 */
static bool set_from_air_sco_info_cback(unsigned con_id, unsigned status,
        unsigned op_id, unsigned num_resp_params, unsigned *resp_params)
{
    return TRUE;
}


/****************************************************************************
 *
 * set_from_air_sco_info
 *
 */
static void set_from_air_sco_info(unsigned int sink_id,
                                  unsigned int expected_ts,
                                  unsigned int from_air_length,
                                  unsigned int tesco,
                                  unsigned int exp_pkts)
{
    /* Create a message and send it to the operator
     * We assume it's SCO_RCV / WBS_DEC, and the message IDs
     * and content are the same in both cases
     */
    unsigned params[5];
    patch_fn_shared(stream_sco);

    params[0] = OPMSG_SCO_RCV_ID_SET_FROM_AIR_INFO;

    params[1] = from_air_length;
    params[2] = exp_pkts;
    params[3] = tesco;
    params[4] = expected_ts;

    opmgr_operator_message(RESPOND_TO_OBPM, sink_id, sizeof(params)/sizeof(unsigned),
        params, set_from_air_sco_info_cback);
}


/****************************************************************************
 *
 * set_to_air_sco_info_cback
 *
 */
static bool set_to_air_sco_info_cback(unsigned con_id, unsigned status,
        unsigned op_id, unsigned num_resp_params, unsigned *resp_params)
{
    return TRUE;
}

/****************************************************************************
 *
 * set_to_air_sco_info
 *
 */
static void set_to_air_sco_info(unsigned int src_id, unsigned int frame_size)
{
    /* Create a message and send it to the operator
     * We assume it's SCO_SEND / WBS_ENC, and the message IDs
     * and content are the same in both cases
     */
    unsigned params[2];
    patch_fn_shared(stream_sco);

    params[0] = OPMSG_SCO_SEND_ID_SET_TO_AIR_INFO;
    params[1] = frame_size;

    opmgr_operator_message(RESPOND_TO_OBPM, src_id, sizeof(params)/sizeof(unsigned),
        params, set_to_air_sco_info_cback);
}


#ifdef CHIP_NAPIER
static void set_initial_timestamp(ENDPOINT *endpoint, TIME initial_time_stamp)
{
        /* store the expected timestamp of the first RX sco packet */
        endpoint->state.sco.time_stamp_init = initial_time_stamp;
        return;
}
#endif


static void sco_reinit_ratematching(ENDPOINT *endpoint)
{
    int32 wallclock_value = sco_wallclock_get(endpoint->key);
    endpoint_sco_state* ep_sco = &endpoint->state.sco;
    ep_sco->rate_measurement = 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
    ep_sco->rm_wclock = wallclock_value;
    ep_sco->rm_start_time = time_sub(ep_sco->data_avail_time, wallclock_value);
#ifdef INSTALL_SCO_EP_CLRM
    rate_measure_stop(&ep_sco->rm_measure);
    ep_sco->rm_result.sp_deviation = 0;
    ep_sco->rm_result.measurement.valid = FALSE;
    ep_sco->rm_result.measurement.q.restarted = TRUE;
    ep_sco->rm_result.measurement.q.unreliable = FALSE;
    ep_sco->rm_result.measurement.q.delta_usec = SECOND;
    ep_sco->rm_result.measurement.q.num_samples = SCO_RM_PSEUDO_SAMPLE_RATE;
    ep_sco->rm_result.measurement.nominal_rate_div25 = SCO_RM_PSEUDO_SAMPLE_RATE/25;
    if (ep_sco->rm_enable_clrm_trace)
    {
        L2_DBG_MSG1("sco 0x%04x reinit_ratematching", stream_external_id_from_endpoint(endpoint));
    }
#endif /* INSTALL_SCO_EP_CLRM */
}

#ifdef __KCC__
asm int sco_calc_rate(int expected, int measured)
{
    @[    .change rMACB
          .restrict expected:bank1_with_rmac, measured:bank1_with_rmac
     ]
    /* Make use of rMACB as the C compiler doesn't */
    rMACB = @{expected} ASHIFT -2 (56bit);
    Div = rMACB / @{measured};
    @{} = DivResult;
    @{} = @{} ASHIFT -(DAWTH-24);
}
#else /* __GNUC__ */
static int sco_calc_rate(int expected, int measured)
{
    return 1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT;
}
#endif /* __GNUC__ */

void sco_sched_kick(ENDPOINT *endpoint, KICK_OBJECT *ko)
{
    unsigned hci_handle = endpoint->key; /* The endpoint key is the hci-handle */
    int32 wallclock_value = sco_wallclock_get(hci_handle);
    TIME new_kick_time;
    unsigned tesco = STREAM_KICK_PERIOD_TO_USECS(endpoint->state.sco.kick_period);
    unsigned delta_time;

#ifdef DEBUG_KICK_TIMERS
    TIME now;
    endpoint->state.sco.last_wallclock_value = wallclock_value;
#endif
    /* Schedule to kick for when the next data packet is due to be received. We'll
     * allow for drift with the wallclock when we schedule the timer.
     */
    endpoint->state.sco.data_avail_time = time_add(endpoint->state.sco.data_avail_time,tesco);
    
    new_kick_time = time_sub(endpoint->state.sco.data_avail_time, wallclock_value);
    
#ifdef DEBUG_KICK_TIMERS

    now = hal_get_time();

    if (time_lt(new_kick_time, now) || time_gt(new_kick_time, time_add(now, 3*tesco)))
    {
        /* Time looks dubious, so rebase and emit a fault
         * This code is very similar to that in stream_sco_start_core, but replicated here for clarity
         */
        TIME_INTERVAL project_fwd;
        TIME_INTERVAL delta = 0;
        unsigned n;

        fault_diatribe(FAULT_AUDIO_SCO_TIMER_REBASED, (DIATRIBE_TYPE)new_kick_time);

        if (time_le(new_kick_time, now))
        {
            delta = time_sub(now, new_kick_time);
        }
        else
        {
            delta = time_sub(now, new_kick_time);
            delta += MAX_TIME + 1;
        }
        if (0 != delta)
        {
            n = (unsigned)(delta / tesco + 1);
            project_fwd = n * (TIME_INTERVAL)tesco;
            endpoint->state.sco.data_avail_time = time_add(endpoint->state.sco.data_avail_time, project_fwd);
            sco_reinit_ratematching(endpoint);
            new_kick_time = time_add(new_kick_time, project_fwd);
        }
    }
#endif

#ifdef INSTALL_SCO_EP_CLRM
    if (endpoint->state.sco.rm_enable_clrm_measurement)
    {
        /* Convert the sco rate to 8000Hz sample rate equivalent,
         * i.e. equivalent number of samples per packet = tesco/125
         */
        unsigned equiv_num_samples = tesco/125;
        if (tesco == (equiv_num_samples * 125))
        {
            rate_measure_update(&endpoint->state.sco.rm_measure,
                                equiv_num_samples, new_kick_time, 0);
        }
        else
        {
            /* Unexpected -- tesco should always be a multiple of 1250 */
            rate_measure_stop(&endpoint->state.sco.rm_measure);
        }
    }
#endif /* INSTALL_SCO_EP_CLRM */

    delta_time = (unsigned)time_sub(new_kick_time, endpoint->state.sco.rm_start_time);

    if(delta_time > SECOND)
    {
        int diff = (int) (wallclock_value - endpoint->state.sco.rm_wclock);
        unsigned expected = (unsigned)((int)delta_time + diff);

        endpoint->state.sco.rm_start_time = new_kick_time;
        endpoint->state.sco.rm_wclock = wallclock_value;

        /* Rate is 2^22*((accumulated_time/measured_time)) */
        endpoint->state.sco.rate_measurement = (int)sco_calc_rate(expected, delta_time);
     }

    endpoint->state.sco.kick_id = timer_schedule_event_at(new_kick_time, kick_obj_kick, (void*)ko);

#ifdef DEBUG_KICK_TIMERS
    debug_from_air_kick_time[debug_kick_time_count] = new_kick_time;

    if (++debug_kick_time_count >= DEBUG_KICK_TIME_MAX)
    {
        debug_kick_time_count = 0;
    }
#endif

    return;
}

bool sco_start(ENDPOINT *endpoint, KICK_OBJECT *ko)
{
    /* This might not be needed however, if we are already running
       then don't do anything */
    if(TIMER_ID_INVALID == endpoint->state.sco.kick_id)
    {
        TIME fire_time;
        /* The source is the SCO side here as it is from air */
        unsigned hci_handle = stream_sco_get_hci_handle(endpoint);

        /*
         * The kick is based on the timing information that is/will be provided
         * by the sco_params and the wallclock. This calculation takes account
         * of clock drift and from-air latency. The wallclock is included in the
         * calculation in stream_sco_start_core. N.B. The wallclock is
         * subtracted rather than added due to the way it is calculated. This is
         * explained in the comments for sps_wallclock_get().
         */
         if (SOURCE == endpoint->direction)
         {
             /* The maths performed to calculate the first kick is:
             *
             * data_available_us = next_slot_time_us - wallclock + from_air_latency
             *
             * If this time has passed increment by Tesco (in us) until the kick is
             * in the future.
             */
             endpoint->state.sco.data_avail_time =
                             time_add(sco_next_slot_get(hci_handle),
                             sco_from_air_latency_get(hci_handle));
         }
         else
         {
             /* The maths performed to calculate the first kick is:
              *
              * data_due_us = next_slot_time_us - wallclock - to_air_latency - audio_processing_time
              *
              * If this time has passed, increment by Tesco (in us) until the kick is
              * in the future.
              */
             endpoint->state.sco.data_avail_time =
                     time_sub(time_sub(sco_next_slot_get(hci_handle),
                     sco_to_air_latency_get(hci_handle)),
                     endpoint->state.sco.proc_time);
         }


        /* Apply the wallclock and transform us to run JIT for the next SCO packet */
#ifndef DEBUG_KICK_TIMERS
        fire_time = stream_sco_start_core(&endpoint->state.sco.data_avail_time,
                hci_handle, endpoint->direction);
#else
        fire_time = stream_sco_start_core(&endpoint->state.sco.data_avail_time,
                        hci_handle, endpoint->direction,
                        &endpoint->state.sco.initial_wallclock_value);
        endpoint->state.sco.initial_kick_time = fire_time;
#endif

        sco_reinit_ratematching(endpoint);
#ifdef INSTALL_SCO_EP_CLRM
        rate_measure_set_nominal_rate(&endpoint->state.sco.rm_measure, SCO_RM_PSEUDO_SAMPLE_RATE);
#endif /* INSTALL_SCO_EP_CLRM */
 
        timer_schedule_event_at_atomic(fire_time, kick_obj_kick, (void*)ko, &endpoint->state.sco.kick_id);

        /* update the chains running state in case the other side needs
         * timing info to schedule in relation to this chain */
        if (SOURCE == endpoint->direction)
        {
            TIME expected_ts;
            TIME_INTERVAL delta;
            unsigned n, bt_ticks,tesco;

            sco_from_air_endpoint_run_state_set(hci_handle, CHAIN_RUNNING);

            /*
             * Normally at this point we have received the SCO parameters from
             * BT, however in rare cases it might be that the SCO operator has been started
             * before SCO parameters are received from BT. If we haven't received sco
             * parameters yet, we don't send the run state this time, it will reach here again once
             * the parameters are received.
             */
            if(sco_params_received_get(hci_handle))
            {
                /* BT doesn't really have any use for frame length so we'll send 0.
                 * If we come up with a use for it in the future then we'll change
                 * the code to send something constructive. */
                sco_send_frame_length_and_run_state(hci_handle, SCO_DIR_FROM_AIR, 0);
            }

            /* Calculate the expected timestamp in the metadata header on
             * first kick and pass it to the relevant operator.
             */
            tesco = sco_tesco_get(hci_handle);
            expected_ts = ((sco_next_slot_get(hci_handle) * 2) + US_PER_SLOT - 1) / US_PER_SLOT;
            delta = time_sub(endpoint->state.sco.data_avail_time,
                    time_add(sco_next_slot_get(hci_handle),
                    sco_from_air_latency_get(hci_handle)));
            n = (unsigned)(delta / tesco);
            bt_ticks = 2 * n * (tesco / US_PER_SLOT);
            expected_ts = btclock_add(expected_ts, bt_ticks);


#ifdef CHIP_NAPIER
            set_initial_timestamp(endpoint, expected_ts&0xffff);
#endif
            set_from_air_sco_info(endpoint->connected_to->id,
                   (unsigned)(expected_ts&0xffff),
                   sco_from_air_length_get(hci_handle), tesco/US_PER_SLOT, 1);
        }
        else
        {
            unsigned block_size;
            unsigned frame_size, tsco;

            sco_to_air_endpoint_run_state_set(hci_handle, CHAIN_RUNNING);

            /* The to-air frame length actually has two meanings. It is:
             * 1. The data frame so BT rate-matching doesn't throw away a portion
             * of an encoded frame.
             * 2. The maximum length that audio will deliver data into the sco
             * buffer. If the operator will run more than once per kick then it
             * may produce more than a packet worth. BT needs to know otherwise
             * it could discard data and then later will have insufficient to
             * service a later packet.
             *
             * These can both be calculated with the equation:
             *
             *  Operator o/p block size * CEIL(Kp/Tsco)
             *
             *  Where:
             *      Kp = Kick period
             *      Tsco = Sco period
             */
            opmgr_get_block_size(endpoint->connected_to->id, &block_size);

            /* If we don't have a fixed size (i.e. SCO NB), make the frame size
             * the same as the packet size
             */
            unsigned to_air_len = sco_to_air_length_get(hci_handle);

            if (block_size == 0)
            {
                block_size = to_air_len;
            }
            else if(block_size < to_air_len)
            {
                /* We have a block size that is less than packet length. Typical example is
                 * WB encoder reporting its minimum default, i.e. 30 words.
                 * Double up the block size, and if this is still less than packet length,
                 * then log a fault as we can't buffer and manage something that large.
                 * In addition, must ensure that enough data
                 * comes in to the sco sink, so e.g. WB encoder needs then to deliver two
                 * of its output blocks.
                 */
                block_size *= 2;

                if(block_size < to_air_len)
                {
                    fault_diatribe(FAULT_AUDIO_PACKET_SIZE_LARGER_THAN_MANAGEABLE_LENGTH,
                                   to_air_len);
                }
            }

            tsco = sco_tesco_get(hci_handle);

            /* CEIL(Kp/Tsco) */
            frame_size = (STREAM_KICK_PERIOD_TO_USECS(endpoint->state.sco.kick_period)
                            + tsco - 1) / tsco;

            frame_size = block_size * frame_size;

            /* Tell the connected operator what the frame size is */
            set_to_air_sco_info(endpoint->connected_to->id, frame_size);

            /*
             * Normally at this point we have received the SCO parameters from
             * BT, however in rare cases it might be that the SCO operator has been started
             * before SCO parameters are received from BT. We can only send run state
             * with valid frame_size therefore if we haven't received sco parameters yet,
             * we don't send the run state this time, it will reach here again once
             * the parameters are received.
             */
            if(sco_params_received_get(hci_handle))
            {
                /* For BT, the frame length needs to be specified in octets so turn our word
                 * based definition into octets. */
                frame_size *= 2;
                sco_send_frame_length_and_run_state(hci_handle, SCO_DIR_TO_AIR, (uint16)frame_size);
            }
            else
            {
                /* The request for starting SCO operators has arrived earlier,
                 * very rare but can happen, SCO processing will run based on our guessed
                 * parameters until we receive actual parameters, could have consequences
                 * but hopefully actual parameters will arrive very shortly.
                 * 
                 * FIXME: The right solution to avoid starting SCO with guessed parameters
                 * is to defer the start of SCO endpoints/operators until we have received the actual SCO
				 * parameters from BT.
                 */
                L2_DBG_MSG("Warning: Starting SCO before seeing SCO_PARAMS! Using guessed parameters");
            }
        }

        /* empty the buffer, show is about to start */
        flush_endpoint_buffers(endpoint);
    }

    return TRUE;
}

bool sco_stop(ENDPOINT *endpoint)
{
    unsigned hci_handle;

    /* update the running state to indicate the chain is no longer running */
    hci_handle = endpoint->key;
    if (SOURCE == endpoint->direction)
    {
        sco_from_air_endpoint_run_state_set(hci_handle, CHAIN_NOT_RUNNING);
    }
    else
    {
        sco_to_air_endpoint_run_state_set(hci_handle, CHAIN_NOT_RUNNING);
    }

    /* Block interrupts to make sure the timer ID can't change
     * while we're trying to cancel it
     */
    interrupt_block();
    /* This transform owns the kick timer. Cancel the timer if it is running */
    if(TIMER_ID_INVALID != endpoint->state.sco.kick_id)
    {
        timer_cancel_event(endpoint->state.sco.kick_id);
        /* The transform is no longer running */
        endpoint->state.sco.kick_id = TIMER_ID_INVALID;
        interrupt_unblock();
        return TRUE;
    }
    interrupt_unblock();
    return FALSE;
}

bool sco_configure(ENDPOINT *endpoint, unsigned int key, uint32 value)
{
    /* SCO specific endpoint configuration code to go here.
     */
    if ((key & ENDPOINT_INT_CONFIGURE_KEYS_MASK) != 0)
    {
        switch(key)
        {
        case EP_CBOPS_PARAMETERS:
            /* Cbops parameter passing is not supported be SCO.
             * Free the cbops_parameters to avoid any potential memory leak and fail. */
            free_cbops_parameters((CBOPS_PARAMETERS *)(uintptr_t) value);
            return FALSE;
        case EP_DATA_FORMAT:
            return sco_set_data_format(endpoint, (AUDIO_DATA_FORMAT)value);
        case EP_KICK_PERIOD:
            endpoint->state.sco.kick_period = (unsigned int)value;
#ifdef INSTALL_SCO_EP_CLRM
            rate_measure_stop(&endpoint->state.sco.rm_measure);
#endif /* INSTALL_SCO_EP_CLRM */
            return TRUE;
        case EP_PROC_TIME:
            endpoint->state.sco.proc_time = (unsigned int)value;
            return TRUE;

        default:
            return FALSE;
        }
    }
    else
    {
#ifdef USE_ACCMD
        switch (key)
        {
        case ACCMD_CONFIG_KEY_STREAM_RM_ENABLE_FAST_MEASUREMENT:
#ifdef INSTALL_SCO_EP_CLRM
            endpoint->state.sco.rm_enable_clrm_measurement = (value != 0);
            return TRUE;
#else /* INSTALL_SCO_EP_CLRM */
            /* Reject if not implemented */
            return (value == 0);
#endif /* INSTALL_SCO_EP_CLRM */

        case ACCMD_CONFIG_KEY_STREAM_RM_RATE_MATCH_TRACE:
#ifdef INSTALL_SCO_EP_CLRM
            /* This is for diagnostic purposes -- ignore if not implemented */
            endpoint->state.sco.rm_enable_clrm_trace = (value != 0);
#endif /* INSTALL_SCO_EP_CLRM */
            return TRUE;

        default:
            return FALSE;
        }
#else /* USE_ACCMD */
        return FALSE;
#endif /* USE_ACCMD */
    }
}

#ifdef INSTALL_SCO_EP_CLRM
static bool get_sco_rate_measurement(ENDPOINT *ep)
{
    endpoint_sco_state* ep_sco = &ep->state.sco;
    bool result;

    /* Prevent sco_sched_kick and thus rate_measure_update
     * running in the middle of taking a measurement.
     */
    interrupt_block();
    /* Passing last_timestamp as the time the measurement is taken
     * will keep the age and interval of the measurement equal.
     */
    result = rate_measure_take_measurement(&ep_sco->rm_measure,
                                           &ep_sco->rm_result.measurement.q,
                                           /*&sco_rm_validity*/&rate_measurement_validity_default,
                                           ep_sco->rm_measure.last_timestamp);
    interrupt_unblock();

    if (result)
    {
        if (! rate_deviation(ep_sco->rm_measure.sample_rate_div25,
                             &ep_sco->rm_result.measurement.q,
                             &ep_sco->rm_result.sp_deviation))
        {
            result = FALSE;
        }
        if (ep_sco->rm_enable_clrm_trace)
        {
            /* These only differ in the last word, quasi to fit a sixth, bool parameter in the log */
            if (result)
            {
                L2_DBG_MSG5("sco 0x%04x get_sco_rate_measurement num_samples %d delta_usec %d last_ts %d sp_dev %d ok",
                            stream_external_id_from_endpoint(ep),
                            ep_sco->rm_measure.last_measurement.num_samples,
                            ep_sco->rm_measure.last_measurement.delta_usec,
                            ep_sco->rm_measure.last_measurement.last_timestamp,
                            ep_sco->rm_result.sp_deviation);
            }
            else
            {
                L2_DBG_MSG5("sco 0x%04x get_sco_rate_measurement num_samples %d delta_usec %d last_ts %d sp_dev %d ng",
                            stream_external_id_from_endpoint(ep),
                            ep_sco->rm_measure.last_measurement.num_samples,
                            ep_sco->rm_measure.last_measurement.delta_usec,
                            ep_sco->rm_measure.last_measurement.last_timestamp,
                            ep_sco->rm_result.sp_deviation);
            }
        }
    }
    else
    {
        if (ep_sco->rm_enable_clrm_trace)
        {
            L2_DBG_MSG1("sco 0x%04x get_sco_rate_measurement failed",
                        stream_external_id_from_endpoint(ep));
        }
    }
    ep_sco->rm_result.measurement.valid = result;
    ep_sco->rm_result.measurement.nominal_rate_div25 = SCO_RM_PSEUDO_SAMPLE_RATE / 25;
    return result;
}
#endif /* INSTALL_SCO_EP_CLRM */

/**
 *
 */
static int get_sco_rate(ENDPOINT *ep)
{
    patch_fn_shared(stream_sco);

    return ep->state.sco.rate_measurement;
}

static bool is_locally_clocked(ENDPOINT *ep)
{
    /* All Kymera firmware is post B-111491 so we can tell if we are master
     * or slave in the link by looking at the to-air latency parameter. If the
     * to-air latency is negative then this device is slave. */
    if (sco_to_air_latency_get(ep->key) < 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

bool sco_get_config(ENDPOINT *endpoint, unsigned int key, ENDPOINT_GET_CONFIG_RESULT* result)
{
    /* SCO specific endpoint configuration code to go here.
     */
    endpoint_sco_state* ep_sco = &endpoint->state.sco;
    switch(key)
    {
    case EP_DATA_FORMAT:
        result->u.value = sco_get_data_format(endpoint);
        return TRUE;
    case EP_KICK_PERIOD:
        result->u.value = ep_sco->kick_period;
        return TRUE;
    case EP_PROC_TIME:
        result->u.value = ep_sco->proc_time;
        return TRUE;
    case EP_RATEMATCH_ABILITY:
        /* Sco endpoints can't ratematch as the data might be encoded. */
        result->u.value = (uint32)RATEMATCHING_SUPPORT_NONE;
        return TRUE;
    case EP_RATEMATCH_RATE:
        result->u.value = (uint32)(int32)get_sco_rate(endpoint);
        L3_DBG_MSG2("SCO rate : %d EP: %06X", result->u.value, (uintptr_t)endpoint);
        return TRUE;

    case EP_RATEMATCH_MEASUREMENT:
#ifdef INSTALL_SCO_EP_CLRM
        if (ep_sco->rm_enable_clrm_measurement)
        {
            get_sco_rate_measurement(endpoint);
            result->u.rm_meas = ep_sco->rm_result;
            return TRUE;
        }
#else /* INSTALL_SCO_EP_CLRM */
        result->u.rm_meas.sp_deviation =
                STREAM_RATEMATCHING_RATE_TO_FRAC(get_sco_rate(endpoint));
        result->u.rm_meas.measurement.valid = FALSE;
#endif /* INSTALL_SCO_EP_CLRM */
        return TRUE;

#ifdef STREAM_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION
    case STREAM_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION:
#ifdef INSTALL_SCO_EP_CLRM
        if (ep_sco->rm_enable_clrm_measurement)
        {
            result->u.value = ep_sco->rm_result.sp_deviation;
            return TRUE;
        }
#else /* INSTALL_SCO_EP_CLRM */
        result->u.value =
                STREAM_RATEMATCHING_RATE_TO_FRAC(get_sco_rate(endpoint));
#endif /* INSTALL_SCO_EP_CLRM */
        return TRUE;
#endif /* STREAM_INFO_KEY_AUDIO_SAMPLE_PERIOD_DEVIATION */

#ifdef STREAM_INFO_KEY_AUDIO_LOCALLY_CLOCKED
    case STREAM_INFO_KEY_AUDIO_LOCALLY_CLOCKED:
        result->u.value = is_locally_clocked(endpoint);
        return TRUE;
#endif /* STREAM_INFO_KEY_AUDIO_LOCALLY_CLOCKED */
     default:
        return FALSE;
    }
}

void sco_common_get_timing (ENDPOINT *endpoint, ENDPOINT_TIMING_INFORMATION *time_info)
{
    patch_fn_shared(stream_sco);
    /* SCO specific endpoint timing information code to go here */
    time_info->period = STREAM_KICK_PERIOD_FROM_USECS(sco_tesco_get(endpoint->key));
    if (SINK == endpoint->direction)
    {
        time_info->block_size = sco_to_air_length_get(endpoint->key);
    }
    else
    {
        time_info->block_size = sco_from_air_length_get(endpoint->key);
    }
    time_info->has_deadline = TRUE;
    time_info->locally_clocked = is_locally_clocked(endpoint);

    return;
}

unsigned stream_sco_get_wallclock_id(ENDPOINT *ep)
{
    unsigned hci_handle = ep->key;

    return sco_get_wallclock_id(hci_handle);
}
