/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  stream_ratematch_mgr.c
 * \ingroup stream
 *
 * ratematching main file. <br>
 * This file contains functionality for making decisions on where and how much
 * to rate match. <br>
 *
 * \section sec1 Contains:
 */

/****************************************************************************
Include Files
*/

#include "stream_private.h"

/****************************************************************************
Private Type Declarations
*/

/** This enum encapsulates the information about an endpoint
 * which is needed to sort endpoints by preference to enact
 * rate adjustment. The least suitable endpoint types
 * are first.
 */
typedef enum {
    /** Not rate adjustment capable */
    RM_ORD_NONE,

    /** Software rate adjustment capable and locally clocked */
    RM_ORD_SW_LC,

    /** Software rate adjustment capable and non-locally clocked */
    RM_ORD_SW_NLC,

    /** Hardware rate adjustment capable and locally clocked */
    RM_ORD_HW_LC,

    /** Hardware rate adjustment capable and non-locally clocked */
    RM_ORD_HW_NLC,

    /** Monitoring rates, should always "enact" in order to receive rates */
    RM_ORD_MON,

    /** Number of endpoint ranks which need to be sorted */
    RM_NUM_ORDS,

    /** EPs with this value will not need to be sorted */
    RM_ORD_DISCARD
}
RM_ENACT_RANK;

/**
 * Used to create a list of pairs of clock sources to ratematch between and the
 * endpoint responsible for applying the ratematch enacting.
 */
typedef struct RATEMATCH_PAIR
{
    ENDPOINT *ep1;
    ENDPOINT *ep2;
    ENDPOINT *enacting_ep;
    struct RATEMATCH_PAIR *next;
}RATEMATCH_PAIR;

/** A structure for managing a list of endpoints. */
typedef struct {
    /** Length of the ep array */
    unsigned                length;

    /** Information about the first, i.e. least rate match suitable endpoint,
     * used for selecting the non-enacting endpoint */
    RATEMATCHING_SUPPORT    ep0_rank;

    /** Flag indicating if one or more of the endpoints in the list are externally clocked */
    bool                    not_local;

    /** Variable length array containing real endpoints either sources or sinks */
    ENDPOINT                *ep[];
} ENDPOINT_LIST;


/****************************************************************************
Private Constant Declarations
*/
/** The frequency at which to adjust for a rate missmatch change. */
#define RATEMATCH_DECISION_PERIOD (SECOND/20)

/** The factor by which the old rate match system was slower */
#define RATEMATCH_DECISION_SLOW_RATIO (5)

/** The maximum number of operators supported in a graph. */
#define MAX_OPERATORS 40
/** The maximum number of real endpoints supported in a graph. */
#define MAX_REAL_EPS 30

/** Abuse the SINK BIT to indicate if an operator is on a remote or local core */
#define OP_IS_REMOTE (STREAM_EP_SINK_BIT)
/****************************************************************************
Private Macro Declarations
*/


/****************************************************************************
Private Variable Definitions
*/
/** The list of endpoints to ratematch between. */
/* This is static except for unit tests which want to check it's value */
#ifndef UNIT_TEST_BUILD
static
#endif
DM_P0_RW_ZI RATEMATCH_PAIR *rm_list = NULL;

/** Flag indicating whether a RM background timer is already in existence. */
DM_P0_RW_ZI static bool rm_timer_running = FALSE;

/** Counter to divide faster to slower rate matching period. */
DM_P0_RW_ZI static unsigned rm_timer_slow_period = 0;

/** The maximum number of operators find_graph_real_sources_and_sinks supports.
 * This is global so that it can be patched trivially. */
DM_P0_RW static volatile unsigned max_operators = MAX_OPERATORS;

/****************************************************************************
Private Constant Data Definitions
*/
static const RATEMATCHING_SUPPORT rm_ord_to_rm_support[RM_NUM_ORDS] =
{
    /* RM_ORD_NONE:     */  RATEMATCHING_SUPPORT_NONE,
    /* RM_ORD_SW_LC:    */  RATEMATCHING_SUPPORT_SW,
    /* RM_ORD_SW_NLC:   */  RATEMATCHING_SUPPORT_SW,
    /* RM_ORD_HW_LC:    */  RATEMATCHING_SUPPORT_HW,
    /* RM_ORD_HW_NLC:   */  RATEMATCHING_SUPPORT_HW,
    /* RM_ORD_MON:      */  RATEMATCHING_SUPPORT_MONITOR
};

/****************************************************************************
Private Function Definitions
*/
/****************************************************************************
 * 
 *  \brief  Enables rate matching, based on adjustments to the head endpoint,
 *          for all endpoints in the sync chain.
 *
 *  \param  head_ep Head endpoint of sync chain
 */
static void ratematch_point_to_head(ENDPOINT *head_ep)
{
    ENDPOINT * ep;
    if(head_ep->stream_endpoint_type == endpoint_audio)
    {
        for(ep = head_ep->state.audio.nep_in_sync; ep !=NULL; ep= ep->state.audio.nep_in_sync)
        {
            ENDPOINT_CONFIGURE(ep, EP_RATEMATCH_ENACTING,  (uint32)TRUE);
        }
    }
}
 
/****************************************************************************
 * 
 *  \brief  Disables rate matching on all endpoints in the chain below the
 *          endpoint being removed from sync chain.
 *
 *  \param  ep endpoint being removed from sync list
 */
static void ratematch_undo_point_to_head(ENDPOINT *ep)
{
    if(ep->stream_endpoint_type ==endpoint_audio)
    {
        for(ep = ep->state.audio.nep_in_sync; ep !=NULL; ep= ep->state.audio.nep_in_sync)
        {
            ENDPOINT_CONFIGURE(ep, EP_RATEMATCH_ENACTING,  (uint32)FALSE);
        }
    }
}

/**
 * \brief inline assembly function which performs a fractional divide of the 2
 * rates to give a value based around a normalised value of 0.5. This value is
 * then left shifted to give a fractional value which represents how much the
 * rates differ from 1.0.
 *
 * \param rate1 the rate to divide by
 * \param rate2 the rate for the numerator
 * 
 * \return Fractional part of the ratio of the missmatch between the rates
 */
#ifndef __GNUC__
asm int calc_diff(int rate1, int rate2)
{
    @[    .change rMACB
          .restrict rate1:bank1_with_rmac, rate2:bank1_with_rmac
     ]
    /* Make use of rMACB as the C compiler doesn't */
    rMACB = @{rate2} ASHIFT -2 (56bit);
    Div = rMACB / @{rate1};
    @{} = DivResult;
    @{} = @{} - 0.5;
    @{} = @{} ASHIFT 1;
}
#else /* __GNUC__ */
static int calc_diff(int rate1, int rate2)
{
    int ans;
    rate2 >>= 2;
    ans = rate2/rate1;
    ans -= FRACTIONAL(0.5);
    ans <<= 1;
    return ans;
}
#endif /* __GNUC__ */


/****************************************************************************
 * \brief
 */
static int calc_missmatch(ENDPOINT *ep1, ENDPOINT *ep2)
{
    ENDPOINT_GET_CONFIG_RESULT res_ep1, res_ep2;
    res_ep1.u.value = RM_PERFECT_RATE;
    res_ep2.u.value = RM_PERFECT_RATE;

    if (! ENDPOINT_GET_CONFIG(ep1, EP_RATEMATCH_RATE, &res_ep1))
    {
        res_ep1.u.value = RM_PERFECT_RATE;
    }
    if (! ENDPOINT_GET_CONFIG(ep2, EP_RATEMATCH_RATE, &res_ep2))
    {
        res_ep2.u.value = RM_PERFECT_RATE;
    }

    return calc_diff((int)(int32)res_ep1.u.value, (int)(int32)res_ep2.u.value);
}
/****************************************************************************
 *  
 *  \brief Calculates the rate adjustment needed on the enacting endpoint based
 *        on the relative difference between rates. 
 *
 *  \param  data not used
 */
static void ratematch_decision(void *data)
{
    RATEMATCH_PAIR *pair;
    NOT_USED(data);

    patch_fn(stream_ratematch_decision);

    if (rm_list == NULL)
    {
        /* Indicate we are no running a timer for rate-match decisions */
        rm_timer_running = FALSE;
        return;
    }

    /* For every ratematching pair work out the differential rate and tell the
     * enacting endpoint. */
    for (pair = rm_list; pair != NULL; pair = pair->next)
    {
        ENDPOINT* non_enacting_ep;
        if (pair->ep1 == pair->enacting_ep)
        {
            non_enacting_ep = pair->ep2;
        }
        else
        {
            non_enacting_ep = pair->ep1;
        }
        ENDPOINT_GET_CONFIG_RESULT result;
        if (ENDPOINT_GET_CONFIG(non_enacting_ep,EP_RATEMATCH_MEASUREMENT,&result)
            && result.u.rm_meas.measurement.valid)
        {
            ENDPOINT_RATEMATCH_REFERENCE_PARAMS ref;
            ref.sp_deviation = result.u.rm_meas.sp_deviation;
            ref.ref = result.u.rm_meas.measurement;

            /* Provide some information about the reference endpoint */
            ref.ref_endpoint_id = stream_external_id_from_endpoint(non_enacting_ep);

            if (ENDPOINT_CONFIGURE(pair->enacting_ep,EP_RATEMATCH_REFERENCE,
                                   (uint32)(uintptr_t)(&ref)))
            {
                /* Skip the legacy adjustment call */
                continue;
            }
        }

        /* Only run the slower method every N ratematch cycles */
        if (rm_timer_slow_period > 0)
        {
            continue;
        }

        int32 diff = (int32)calc_missmatch(pair->ep1, pair->ep2);
        ENDPOINT_CONFIGURE(pair->enacting_ep, EP_RATEMATCH_ADJUSTMENT,
                           (uint32)diff);
    }

    /* Schedule the next decision */
    rm_timer_running = TRUE;
    timer_schedule_bg_event_in(RATEMATCH_DECISION_PERIOD, ratematch_decision, NULL);

    if (rm_timer_slow_period > 0)
    {
        rm_timer_slow_period -= 1;
    }
    else
    {
        rm_timer_slow_period = RATEMATCH_DECISION_SLOW_RATIO - 1;
    }
}

/**
 * \brief Adds an endpoint to a list of endpoints
 *
 * \param ep The endpoint to add to the list.
 * \param epl The list to add ep to.
 */
static void add_ep_to_ep_list(ENDPOINT *ep, ENDPOINT_LIST *epl)
{
    if (epl->length == MAX_REAL_EPS)
    {
        panic_diatribe(PANIC_AUDIO_RATEMATCHING_LIMITATION, ep->id);
    }
    /* Add this endpoint to the list */
    epl->ep[epl->length++] = ep;
}

static void add_opid_to_graph_list(unsigned opid, unsigned graph_ops[], unsigned *next_new_op)
{
    unsigned j;
    bool already_on_list = FALSE;
    patch_fn_shared(stream_ratematch);

    for (j = 0; j < *next_new_op; j++)
    {
        if (graph_ops[j] == opid)
        {
            already_on_list = TRUE;
            break;
        }
    }
    if (!already_on_list)
    {
        if (*next_new_op >= max_operators)
        {
            fault_diatribe(FAULT_AUDIO_RM_GRAPH_TOO_BIG, opid);
        }
        else
        {
            graph_ops[(*next_new_op)++] = opid;
        }
    }
}

/**
 * \brief Query an endpoint whether it wants to rate-monitor
 * \param ep Endpoint to check
 * \return TRUE if the endpoint requests rate monitoring
 */
static bool is_rate_monitor(ENDPOINT* ep)
{
    uint32 val = RATEMATCHING_SUPPORT_NONE;
    if ( ep->is_rate_match_aware
         && stream_get_endpoint_config(ep, EP_RATEMATCH_ABILITY, &val)
         && (RATEMATCHING_SUPPORT_MONITOR == (RATEMATCHING_SUPPORT)val))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * \brief Visits all the connected terminals of an operator in search of
 * operators and real endpoints that it is connected to, which haven't yet been
 * visited in the graph. New operators and real endpoints are added to the
 * appropriate list.
 *
 * \param op_ep_mask The base ep id of the operators source/sink terminal. Comprised
 * of opid | STREAM_EP_OP_SOURCE/SINK
 * \param *next_new_op index into graph_ops to add a new operator at.
 * \param graph_ops The array of operator ids that have already been visited or
 * will be visited.
 * \param num_terminals The number of source/sink terminals that this operator has.
 * \param ep_list The list of real_sources/sinks that is being compiled.
 */
static void visit_operator_terminals(unsigned op_ep_mask, unsigned *next_new_op,
                                    unsigned graph_ops[], unsigned num_terminals,
                                    ENDPOINT_LIST *ep_list)
{
    ENDPOINT *ep;
    unsigned fnd_cnt = 0; /* Counter to track if we've found all terminals of the operator */
    /* Go through the list of endpoints and find all that exist for this operator */
    if (op_ep_mask & STREAM_EP_SINK_BIT)
    {
        ep = sink_endpoint_list;
    }
    else
    {
        ep = source_endpoint_list;
    }

    while ((ep != NULL) && (fnd_cnt < num_terminals))

    {
        if (((ep->stream_endpoint_type == endpoint_operator) &&
                ((ep->key & ~STREAM_EP_CHAN_MASK) == op_ep_mask)))
        {
            fnd_cnt++;
            if (NULL == ep->connected_to || ep->is_real)
            {
                /* Terminal not connected so move to next one.
                 * OR
                 * The terminal is real and is part of a different graph so we
                 * don't want to traverse that graph so ignore it. */
            }
            else if (ep->connected_to->is_real)
            {
                add_ep_to_ep_list(ep->connected_to, ep_list);
            }
            else
            {
                unsigned opid;
#ifdef INSTALL_DUAL_CORE_SUPPORT
                if( STREAM_EP_IS_SHADOW(ep->connected_to))
                {
                    opid = (ep->connected_to->key & STREAM_EP_OPID_MASK)
                                            | (STREAM_EP_OP_BIT | OP_IS_REMOTE);
                }
                else
#endif /* INSTALL_DUAL_CORE_SUPPORT */
                {
                    /* It's an operator so work out if it's already on the graph_ops
                     * list and add it if it isn't. */
                    opid = ep->connected_to->key & (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT);
                }
                add_opid_to_graph_list(opid, graph_ops, next_new_op);
            }
        }
        ep = ep->next;
    }
}

#ifdef INSTALL_DUAL_CORE_SUPPORT
/**
 * \brief Visits all the connected terminals of an operator on a remote core in search of
 * operators and real endpoints that it is connected to, which haven't yet been
 * visited in the graph. New operators and real endpoints are added to the
 * appropriate list.
 *
 * \param opid The base ep id of the operators source/sink terminal. Comprised
 * of opid | STREAM_EP_OP_SOURCE/SINK
 * \param *next_new_op index into graph_ops to add a new operator at.
 * \param graph_ops The array of operator ids that have already been visited or
 * will be visited.
 * \param num_src_terminal The number of source terminals that this operator has.
 * \param sink_ep_list The list of real_sinks that is being compiled.
 * \param num_src_terminal The number of sinks terminals that this operator has.
 * \param sink_ep_list The list of real_sources that is being compiled.
 */
static void visit_remote_operator_terminals(unsigned opid, unsigned *next_new_op,
        unsigned graph_ops[], unsigned num_src_terminals, ENDPOINT_LIST *sink_ep_list,
        unsigned num_sink_terminals, ENDPOINT_LIST *src_ep_list)
{
    STREAM_KIP_TRANSFORM_INFO *tfm = kip_transform_list;
    unsigned shadow_id, fnd_sources = 0, fnd_sinks = 0;
    ENDPOINT *ep;
    ENDPOINT_LIST *ep_list;

    if (tfm == NULL)
    {
        /* The list is empty so no connections to search */
        return;
    }

    do
    {
        unsigned new_opid;
        /* Slight Fudge here we don't have the possibility of a real endpoint
         * existing in P1 so we don't test for it. So there are only two
         * possibilities. The endpoint is connected to another operator or to a
         * shadow endpoint. If it's a shadow endpoint then we can get hold of
         * physical endpoint on P0 if it's real and add it to the list. */
        if ((tfm->source_id & (STREAM_EP_OP_BIT | STREAM_EP_OPID_MASK)) == opid)
        {
            fnd_sources++;
            new_opid = tfm->sink_id;
            /* We may not need all this information but it saves us duplicating
             * code between branches of this if statemt to populate variables here.
             */
            shadow_id = tfm->source_id & (~STREAM_EP_SHADOW_MASK);
            ep = source_endpoint_list;
            ep_list = sink_ep_list;
        }
        else if ((tfm->sink_id & (STREAM_EP_OP_BIT | STREAM_EP_OPID_MASK)) == opid)
        {
            fnd_sinks++;
            new_opid = tfm->source_id;
            /* We may not need all this information but it saves us duplicating
             * code between branches of this if statemt to populate variables here.
             */
            shadow_id = tfm->sink_id & (~STREAM_EP_SHADOW_MASK);
            ep = sink_endpoint_list;
            ep_list = src_ep_list;
        }
        else
        {
            /* Else means this connection isn't related to the operator being
             * searched for. */
            continue;
        }

        /* Work out if it's shadow and act accordingly */
        if (STREAM_EP_IS_SHADOW_ID(new_opid))
        {
            /* Find the endpoint on P0 and work out if it's real or an op.
             * If it's an op then fall through, if real add to list and continue */
            while (ep != NULL)
            {
                if (ep->id == shadow_id)
                {
                    break;
                }
                ep = ep->next;
            }
            if (ep == NULL || ep->connected_to == NULL)
            {
                /* The counterpart doesn't exist so nothing more we can do.
                 * with this connection. */
                continue;
            }
            if (ep->connected_to->is_real)
            {
                /* Add to the list and continue */
                add_ep_to_ep_list(ep->connected_to, ep_list);
                continue;
            }
            new_opid = ep->connected_to->id & (~OP_IS_REMOTE);
        }
        else
        {
            new_opid |= OP_IS_REMOTE;
        }

        /* Mask out terminal IDs and endpoint bits */
        new_opid &= OP_IS_REMOTE | STREAM_EP_OP_BIT | STREAM_EP_OPID_MASK;

        add_opid_to_graph_list(new_opid, graph_ops, next_new_op);

    } while ((tfm = tfm->next) != NULL && (fnd_sources < num_src_terminals || fnd_sinks < num_sink_terminals));

}
#endif /* INSTALL_DUAL_CORE_SUPPORT */

/**
 * \brief This function finds connected rate monitor operator endpoints
 *        of the same direction as passed in op_ep_mask.
 */
static void find_op_rate_monitor_terminals(unsigned op_ep_mask, unsigned num_terminals,
                                           ENDPOINT_LIST *ep_list)
{
    ENDPOINT *ep;
    unsigned fnd_cnt = 0; /* Counter to track if we've found all terminals of the operator */
    /* Go through the list of endpoints and find all that exist for this operator */
    if (op_ep_mask & STREAM_EP_SINK_BIT)
    {
        ep = sink_endpoint_list;
    }
    else
    {
        ep = source_endpoint_list;
    }

    while ((ep != NULL) && (fnd_cnt < num_terminals))
    {
        if ((ep->stream_endpoint_type == endpoint_operator)
            && ((ep->key & ~STREAM_EP_CHAN_MASK) == op_ep_mask))
        {
            fnd_cnt++;
            if ((ep->connected_to != NULL) && is_rate_monitor(ep))
            {
                add_ep_to_ep_list(ep, ep_list);
#ifdef VERBOSE_GRAPH_TRAVERSAL
                L2_DBG_MSG1("        find_op_rate_monitor_terminals ep 0x%04x monitor", ep->key);
#endif
            }
        }
        ep = ep->next;
    }
}

/**
 * \brief This function works out all the real endpoints that are present in
 * a graph. This is done by finding all the operators that are connected to each
 * other and on the way noting all the real endpoints connected to this set of
 * operators.
 *
 * \param start The endpoint to use as a starting point for the topological search.
 * \param sources The location to store the list of sources that are found.
 * \param sinks The location to store the list of sinks that are found.
 */
static void find_graph_real_sources_and_sinks(unsigned start_id,
                                ENDPOINT_LIST **sources, ENDPOINT_LIST **sinks)
{
    unsigned curr_op, next_new_op = 0;
    ENDPOINT *start = NULL;
    unsigned *graph_ops;
    patch_fn_shared(stream_ratematch);

    /* If the start endpoint is on P0 (if we have multiple cores) then it might
     * be real and a simple chain. So establish that and get hold of the physical
     * endpoint if it's on P0. */
    start = endpoint_from_id(start_id);

#ifdef INSTALL_DUAL_CORE_SUPPORT
    /* Ensure this only executes on P0. It might be nice to re-arrange the code
     * to ensure that this is called only on P0 . */
    if (!KIP_PRIMARY_CONTEXT())
    {
        *sources = *sinks = NULL;
        return;
    }
#endif /* INSTALL_DUAL_CORE_SUPPORT */

    /* If this is a single real source or 2 real sources connected together then
     * this is trivial. */
    if (start != NULL && start->is_real)
    {
        /* If this is a real endpoint not connected to anything then there is
         * no chain. */
        if (NULL == start->connected_to)
        {
            if (SOURCE == start->direction)
            {
                *sinks = NULL;
                *sources = (ENDPOINT_LIST *)xzpmalloc(sizeof(ENDPOINT_LIST) + sizeof(ENDPOINT *));
                if(*sources != NULL)
                {
                    (*sources)->length = 1;
                    (*sources)->ep[0] = start;
                }
            }
            else
            {
                *sources = NULL;
                *sinks = (ENDPOINT_LIST *)xzpmalloc(sizeof(ENDPOINT_LIST) + sizeof(ENDPOINT *));
                if(*sinks != NULL)
                {
                    (*sinks)->length = 1;
                    (*sinks)->ep[0] = start;
                }
            }
            return;
        }
        /* If this is 2 real sources connected together*/
        else if (start->connected_to->is_real)
        {
            *sources = (ENDPOINT_LIST *)xzpmalloc(sizeof(ENDPOINT_LIST) + sizeof(ENDPOINT *));
            *sinks = (ENDPOINT_LIST *)xzpmalloc(sizeof(ENDPOINT_LIST) + sizeof(ENDPOINT *));
            if(*sources == NULL || *sinks == NULL)
            {
                /* If there's no RAM then search isn't possible. Perhaps this should
                 * return FALSE or issue a fault. */
                pfree(*sources);
                pfree(*sinks);
                *sources = NULL;
                *sinks = NULL;
                return;
            }
            (*sources)->length = (*sinks)->length = 1;
            if (SOURCE == start->direction)
            {
                (*sources)->ep[0] = start;
                (*sinks)->ep[0] = start->connected_to;
            }
            else
            {
                (*sinks)->ep[0] = start;
                (*sources)->ep[0] = start->connected_to;
            }
            return;
        }
    }

    /* It's not just loopback so lets visit everything in the graph and see
     * what there is. Starting by making some storage for everything that is
     * found. */
    *sources = (ENDPOINT_LIST *)xzpmalloc(sizeof(ENDPOINT_LIST) + MAX_REAL_EPS * sizeof(ENDPOINT *));
    *sinks = (ENDPOINT_LIST *)xzpmalloc(sizeof(ENDPOINT_LIST) + MAX_REAL_EPS * sizeof(ENDPOINT *));

    if(*sources == NULL || *sinks == NULL)
    {
        /* If there's no RAM then search isn't possible. Perhaps this should
         * return FALSE or issue a fault. */
        pfree(*sources);
        pfree(*sinks);
        *sources = NULL;
        *sinks = NULL;
        return;
    }

    /* Need to traverse a graph so allocate some space to work in */
    graph_ops = xpnewn(max_operators, unsigned);

    if (graph_ops == NULL)
    {
        *sources = *sinks = NULL;
        return;
    }

    /* The start endpoint might be real, if it is work
     * out what operator it is connected to and start there. If it's on an
     * operator then that is where we want to start working from.
     */
    if (start != NULL)
    {
        if (start->is_real)
        {
#ifdef INSTALL_DUAL_CORE_SUPPORT
            if (STREAM_EP_IS_SHADOW(start->connected_to))
            {
                /* This is on P0 so must be shadowing an operator endpoint on P1
                 * so lets get that operator id and start from there. */
                graph_ops[next_new_op++] = (start->connected_to->key & STREAM_EP_OPID_MASK) | STREAM_EP_OP_BIT | OP_IS_REMOTE;
            }
            else
#endif /* INSTALL_DUAL_CORE_SUPPORT */
            {
                graph_ops[next_new_op++] = start->connected_to->key &
                                        (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT);
            }
        }
        else
        {
#ifdef INSTALL_DUAL_CORE_SUPPORT
            if (STREAM_EP_IS_SHADOW(start))
            {
                /* This is on P0 so must be shadowing an operator endpoint on P1
                 * so lets get that operator id and start from there. */
                graph_ops[next_new_op++] = (start->key & STREAM_EP_OPID_MASK) | STREAM_EP_OP_BIT | OP_IS_REMOTE;
            }
            else
#endif /* INSTALL_DUAL_CORE_SUPPORT */
            {
                graph_ops[next_new_op++] = start->key &
                                        (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT);
            }
        }
    }
#ifdef INSTALL_DUAL_CORE_SUPPORT
    else
    {
        /* The endpoint is on P1 so we'll need to look it up. That means it's
         * either a shadow endpoint or an operator endpoint. If it's a shadow
         * endpoint find the operator it's connected to on P1 as it's guaranteed
         * to be an operator. */
        if (!STREAM_EP_IS_SHADOW_ID(start_id))
        {
            graph_ops[next_new_op++] = (start_id & (STREAM_EP_OPID_MASK | STREAM_EP_OP_BIT))
                                            | OP_IS_REMOTE;
        }
        else
        {
            graph_ops[next_new_op++] = (stream_kip_connected_to_epid(start_id) & STREAM_EP_OPID_MASK)
                                            | (STREAM_EP_OP_BIT | OP_IS_REMOTE);
        }
    }
#endif /* INSTALL_DUAL_CORE_SUPPORT */

    /* Now go through the list of operators and make a note of all the connected
     * real endpoints as we go. If we find an operator connected that isn't on
     * the existing graph_ops list then we add it to the list. This approach
     * means that every node in the graph is only visited once. */
    for (curr_op = 0; curr_op < next_new_op; curr_op++)
    {
        unsigned opid = graph_ops[curr_op];
#ifdef INSTALL_DUAL_CORE_SUPPORT
        if ((opid & OP_IS_REMOTE) == OP_IS_REMOTE)
        {
            opid &= ~OP_IS_REMOTE; /* mask out the remote flag */
            visit_remote_operator_terminals(opid, &next_new_op, graph_ops,
                                opmgr_get_num_sink_terminals(opid), *sources,
                                opmgr_get_num_source_terminals(opid), *sinks);
        }
        else
#endif /* INSTALL_DUAL_CORE_SUPPORT */
        {
            /* Arbitrarily go through all the connected sinks first, anything real
             * is a real source that we haven't encountered yet. Any operator needs
             * adding to the list if it isn't already on the list.
             */
            unsigned num_sinks = opmgr_get_num_sink_terminals(opid);
            unsigned num_sources = opmgr_get_num_source_terminals(opid);
            visit_operator_terminals(opid | STREAM_EP_OP_SINK, &next_new_op, graph_ops,
                                     num_sinks, *sources);

            /* Now the connected sources */
            visit_operator_terminals(opid | STREAM_EP_OP_SOURCE, &next_new_op, graph_ops,
                                     num_sources, *sinks);

            find_op_rate_monitor_terminals(opid | STREAM_EP_OP_SINK, num_sinks, *sinks);
            find_op_rate_monitor_terminals(opid | STREAM_EP_OP_SOURCE, num_sources, *sources);
        }
    }

    /* If either of the source or sink lists are empty then release the memory */
    if ((*sources)->length == 0)
    {
        pfree(*sources);
        *sources = NULL;
    }
    if ((*sinks)->length == 0)
    {
        pfree(*sinks);
        *sinks = NULL;
    }

    /* Free the graph_ops working structure as it's no longer of any use */
    pdelete(graph_ops);
}

/**
 * \brief Determines whether two audio endpoints share a clock source.
 *
 * \param ep1 Endpoint to compare with ep2.
 * \param ep2 Endpoint to compare with ep1.
 * \param both_local boolean indicating if both endpoints are locally clocked.
 *
 * \return TRUE if ep1 and ep2 share a clock source, otherwise FALSE.
 */
static bool audio_clock_source_same(ENDPOINT *ep1, ENDPOINT *ep2, bool both_local)
{
    patch_fn_shared(stream_ratematch);

#if !defined(TODO_A7DA_LOOPBACK_RATEMATCHING) && !defined(TODO_CRESCENDO)
    unsigned ep1_hw = get_hardware_type(ep1);
    unsigned ep2_hw = get_hardware_type(ep2);
    /* Codecs are annoying even if they are locally clocked they can have rate
     * miss matches depending on the rate. TODO perhaps a neater implementation
     * would be for a codec configured at an innaccurate rate reports itself
     * as not locally_clocked, then this logic can be simplified.
     *
     * N.B. Digital MICs use the same underlying hardware as Codecs and have the
     * problematic same behaviour*/
    if (STREAM_DEVICE_CODEC == ep1_hw || STREAM_DEVICE_CODEC == ep2_hw ||
            STREAM_DEVICE_DIGITAL_MIC == ep1_hw || STREAM_DEVICE_DIGITAL_MIC == ep2_hw)
    {
        /* If they are both ADCs or both DACs and are synchronised then they have
         * to be at the same rate and have the same clock source. */
        if(ep1->direction == ep2->direction &&
                ep1->state.audio.head_of_sync == ep2->state.audio.head_of_sync)
        {
            return TRUE;
        }
        /* TODO This is where B-168161 should come into play. Also see B-41725
         * On Crescendo & Gordon, for CODECs running at 48K & similar clock rates, this
         * is fine. However, if it is 44K1 and the codecs are not clocked off ADPLL,
         * then ratematching is required. */
        return FALSE;
    }
#endif /* !TODO_A7DA_LOOPBACK_RATEMATCHING */

    if (both_local)
    {
        return TRUE;
    }
    /* At least one of the devices is a slave, if they are the same type of
     * hardware, and the same hardware instance than they must have the same
     * clock. */
    if (get_hardware_type(ep1) == get_hardware_type(ep2))
    {
        if (get_hardware_instance(ep1) == get_hardware_instance(ep2))
        {
            return TRUE;
        }
    }

    return FALSE;
}


/**
 * \brief Determines whether two endpoints share a clock source.
 *
 * \param ep1 Endpoint to compare with ep2.
 * \param ep2 Endpoint to compare with ep1.
 * \param both_local boolean indicating if both endpoints are locally clocked.
 *
 * \return TRUE if ep1 and ep2 share a clock source, otherwise FALSE.
 */
static bool have_same_clock_source(ENDPOINT *ep1, ENDPOINT *ep2, bool both_local)
{
    patch_fn_shared(stream_ratematch);
    /* If the endpoints are not of the same type then they aren't unless both
     * are locally clocked which is already handled separately. */
    if (ep1->stream_endpoint_type != ep2->stream_endpoint_type)
    {
        if (both_local)
        {
            return TRUE;
        }
        return FALSE;
    }

    /* The endpoints are of the same type so work out whether they have the same
     * clock source or not. */
    switch(ep1->stream_endpoint_type)
    {
        case endpoint_operator:
        {
            /* Unless overriding a real endpoint thes are typically both locally
             * clocked. */
            if (both_local || stream_operator_get_clock_id(ep1) == stream_operator_get_clock_id(ep2))
            {
                return TRUE;
            }
            return FALSE;
            break;
        }
#ifdef INSTALL_SCO
        case endpoint_sco:
        {
            /* If these endpoints use the same wallclock then they have the
             * same clock source. Or we are the master of both links in which
             * case they are both locally clocked and can save the function call
             * effort. */
            if (both_local || stream_sco_get_wallclock_id(ep1) == stream_sco_get_wallclock_id(ep2))
            {
                return TRUE;
            }
            return FALSE;
            break;
        }
#endif /* INSTALL_SCO */
        case endpoint_audio:
        {
            /* This is the most complicated case, if the hardware instance is
             * the same then they might be the same
             */
            /* TODO decide how this works */
            return audio_clock_source_same(ep1, ep2, both_local);
        }
#ifdef INSTALL_SPDIF
    case endpoint_spdif:
        {
            /* in two-channel config both share the same external clock. */
            return ep1->state.spdif.twin_endpoint == ep2;
        }
#endif /* INSTALL_SPDIF */
        
        default:
        {
            /* These can't possibly have the same clock source unless they are
             * both locally clocked.*/
            return both_local;
            break;
        }
    }
}

static unsigned rm_ord(ENDPOINT* ep, bool* non_local)
{
    unsigned result;
    uint32 val = RATEMATCHING_SUPPORT_NONE;
    ENDPOINT_TIMING_INFORMATION tinfo;

    if (! stream_get_endpoint_config(ep, EP_RATEMATCH_ABILITY, &val))
    {
        return RM_ORD_NONE;
    }

    switch((RATEMATCHING_SUPPORT)val)
    {
    case RATEMATCHING_SUPPORT_NONE:
        /* This type of EP goes first, regardless if it is locally clocked */
        result = RM_ORD_NONE;
        /* Continue to check if locally clocked, but don't update the sort key */
        break;

    case RATEMATCHING_SUPPORT_SW:
        result = RM_ORD_SW_LC;
        /* Continue to check if locally clocked */
        break;

    case RATEMATCHING_SUPPORT_HW:
        result = RM_ORD_HW_LC;
        /* Continue to check if locally clocked */
        break;

    case RATEMATCHING_SUPPORT_MONITOR:
        /* This type of EP only consumes measurements */
        *non_local = TRUE;
        return RM_ORD_MON;

    case RATEMATCHING_SUPPORT_AUTO:
    default:
        return RM_ORD_DISCARD;
    }

    ep->functions->get_timing_info(ep, &tinfo);
    if (! tinfo.locally_clocked)
    {
        *non_local = TRUE;
        if (result != RM_ORD_NONE)
        {
            result += 1;
        }
    }
    return result;
}

/**
 * \brief Takes a list of endpoints and orders them so that those that can't ratematch
 * are at the top of the list, then those with SW adjustment ability and finally those with
 * hardware adjustment functionality.
 *
 * \param eps  List of endpoints that will be ordered
 *
 * \return True if it was possible to order the list and the endpoints can be
 * rateadjusted to one another. False when it is not possible to rateadjust the set.
 */
static bool sort_ep_list(ENDPOINT_LIST *eps)
{
    /* Empty list is trivially sorted */
    if (eps == NULL)
    {
        return TRUE;
    }

    unsigned i, sorted_length, length = eps->length;
    ENDPOINT **list = eps->ep;
    bool not_local = FALSE;
    unsigned num_eps_per_ord[RM_NUM_ORDS];

    for (i = 0; i < RM_NUM_ORDS; ++ i)
    {
        num_eps_per_ord[i] = 0;
    }
    eps->not_local = FALSE;
    eps->ep0_rank = RATEMATCHING_SUPPORT_AUTO;

    for (sorted_length = 0, i = 0; i < length; i++)
    {
        unsigned ep_ord;
        ENDPOINT *element = list[i];

        /* Endpoints that aren't head of their sync group or have RATEMATCHING_SUPPORT_AUTO
         * can be ignored. So discard them from the list. */
        if (element != stream_get_head_of_sync(element))
        {
            continue;
        }

        /* While inserting an element, all the information needed to determine
         * where it goes is contained in ep_ord. That is its ratematch ability,
         * and where it matters, whether it is locally clocked.
         *
         * Once the element is in the list, that information is preserved in
         * its position in the list, together with the counts in
         * num_eps_per_ord.
         */
        ep_ord = rm_ord(element, &not_local);
        if (RM_ORD_DISCARD == ep_ord)
        {
            continue;
        }

        /* In-place sorting always starts by leaving the first element where it is */
        if (i != 0)
        {
            unsigned j;
            unsigned insert_at;
            if ((RM_ORD_NONE == ep_ord) && (i > 0))
            {
                if (num_eps_per_ord[RM_ORD_NONE] == 0)
                {
                    /* First EP is not NONE, move all up */
                    insert_at = 0;
                }
                else
                {
                    /* We can only support one endpoint that can't be rate-adjusted.
                     * Unless any other endpoint with ability NONE also has the same
                     * clock, in which case it's a duplicate and we don't need to
                     * have the duplicate in the list.
                     */
                    if (stream_rm_endpoints_have_same_clock_source(list[0], element))
                    {
                        /* duplicate don't bother adding to the list */
                        continue;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
            }
            else
            {
                /* Insert after already sorted elements up to
                 * the same order key as the new element
                 */
                insert_at = 0;
                for (j = RM_ORD_NONE; j <= ep_ord; ++ j)
                {
                    insert_at += num_eps_per_ord[j];
                }
            }

            for (j = sorted_length; j > insert_at; -- j)
            {
                list[j] = list[j-1];
            }
            list[insert_at] = element;
        }
        num_eps_per_ord[ep_ord] += 1;

        /* An element was added to the sorted list, record the length of the sorted list */
        sorted_length++;
    }


    eps->length = sorted_length;
    eps->not_local = not_local;

    /*
     * Get the first endpoint's rank.
     *
     * Note: The old sorting algorithm only flagged if there is an endpoint
     * with RM ability none, which is then always the first in the list.
     * If that was not the case, it would later query the first endpoint
     * in the list again about its RM ability.
     *
     * Discard the locally clocked information by mapping back to
     * RATEMATCHING_SUPPORT, because the sink/source choice in
     * ratematch_real_eps with the old sorting algorithm did not
     * consider this.
     * (I.e. emulate the old behaviour to simplify regression testing.)
     */
    for (i = 0; i < RM_NUM_ORDS; ++ i)
    {
        if (num_eps_per_ord[i] != 0)
        {
            eps->ep0_rank = rm_ord_to_rm_support[i];
            break;
        }
    }
    return TRUE;
}

/**
 * \brief Creates a pair of endpoints which are to be ratematched between.
 *
 * \param enacting  The endpoint in the pair which should apply the enactment
 * \param non_encating  The endpoint in the pair which will just report it's rate
 *
 * \return Whether the pair was allocated successfully or not.
 */
static bool create_pair(ENDPOINT *enacting, ENDPOINT *non_enacting)
{
    /* Some adjustment is required so make a new pair */
    RATEMATCH_PAIR *new_pair = xpnew(RATEMATCH_PAIR);
    if (new_pair == NULL)
    {
        /* Allocation failure. There may have been others that succeeded, but
         * they will be cleared up when cease_ratematching gets called on
         * subsequent disconnects. */
        return FALSE;
    }

    new_pair->enacting_ep = enacting;
    if (SINK == enacting->direction)
    {
        new_pair->ep2 = enacting;
        new_pair->ep1 = non_enacting;
    }
    else
    {
        new_pair->ep1 = enacting;
        new_pair->ep2 = non_enacting;
    }

    ENDPOINT_CONFIGURE(enacting, EP_RATEMATCH_ENACTING, (uint32)TRUE);
    /* If the endpoint has any other endpoints synced to it we want rate matching to also apply.
     *
     * TODO might not need this any longer. Leave this in for safety??*/
    ratematch_point_to_head(new_pair->enacting_ep);

    /* Add the new element to the list */
    new_pair->next = rm_list;
    rm_list = new_pair;
    return TRUE;
}

/**
 * \brief Create a set of ratematching pairs that adjust all the pairs to the non_enacting
 * endpoint's rate.
 *
 * \param list  List of endpoints that should adjust to the non-enacting endpoint's rate
 * \param first_element  The index of the first endpoint in the list to create a pair for
 * \param non_enacting  The endpoint who's rate every other endpoint is adjusting to
 * \param non_enacting_is_ep1  Whether the non-enacting endpoint should be
 * ep1(TRUE) or ep2(FALSE) in the pair
 *
 * \return Whether all the pairs were allocated successfully or not.
 */
static bool create_pairs_from_list(ENDPOINT_LIST *list, unsigned first_element,
                            ENDPOINT *non_enacting)
{
    unsigned i;
    if (list != NULL)
    {
        for (i = first_element; i < list->length; i++)
        {
            /* If the two endpoints already have the same clock then there is no
             * need to ratematch between them. */
            if (stream_rm_endpoints_have_same_clock_source(non_enacting, list->ep[i]))
            {
                continue;
            }

            /* Some adjustment is required so make a new pair */
            if(!create_pair(list->ep[i], non_enacting))
            {
                /* An allocation failed so we can't go any further */
                return FALSE;
            }
        }
    }

    return TRUE;
}

/**
 * \brief Setup ratematching between all the real endpoints comprising a graph
 *
 * \param sources  List of source endpoints in the graph
 * \param sinks  List of sink endpoints in the graph
 *
 * \return Whether it was possible to setup ratematching for the graph.
 */
static bool ratematch_real_eps(ENDPOINT_LIST *sources, ENDPOINT_LIST *sinks)
{
    unsigned first_sink_ep = 0, first_source_ep = 0;
    ENDPOINT *non_enacting;
    unsigned count;

    patch_fn_shared(stream_ratematch);

    count = 0;
    if (sources != NULL)
    {
        if (!sort_ep_list(sources))
        {
            return FALSE;
        }
        count += sources->length;
    }
    if (sinks != NULL)
    {
        if (!sort_ep_list(sinks))
        {
            return FALSE;
        }
        count += sinks->length;
    }
    if (count <= 1)
    {
        return TRUE;
    }

    /* If all the endpoints involved are locally clocked then there is no
     * ratematching to do */
    if (! ( ((sources != NULL) && sources->not_local)
            || ((sinks != NULL) && sinks->not_local) ) )
    {
        return TRUE;
    }

    if (   (sources != NULL)
        && (RATEMATCHING_SUPPORT_NONE == sources->ep0_rank)
        && (sinks != NULL)
        && (RATEMATCHING_SUPPORT_NONE == sinks->ep0_rank))
    {
        /* It's only possible to solve this scenario if the endpoints with
         * RATEMATCHING_SUPPORT_NONE have the same clock source. */
        if (!stream_rm_endpoints_have_same_clock_source(sources->ep[0], sinks->ep[0]))
        {
            return FALSE;
        }
        non_enacting = sources->ep[0];
        first_sink_ep = 1;
        first_source_ep = 1;
    }
    else if ((sinks == NULL)
             || (sinks->length == 0)
             || ((sources != NULL) && (sources->length != 0)
                 && (sources->ep0_rank <= sinks->ep0_rank)))
    {
        /* If the least RA-suitable source and sink have the
         * same rank, prefer to enact at the sink.
         * Otherwise, enact at the endpoint with the higher
         * suitability.
         */
        non_enacting = sources->ep[0];
        first_source_ep = 1;
    }
    else
    {
        non_enacting = sinks->ep[0];
        first_sink_ep = 1;
    }

    /* At this point the plan has been formed. Now setup the pairs so that everything
     * ratematches to the non-enacting endpoint.
     *
     * Sources:
     *
     * Warp is calculated as: source EP Rnon_enacting/Rsource_n = WARPsource_n
     *
     * Sinks:
     *
     * Warp is calculated as: Rsink_n/Rnon_enacting = WARPsink_n
     */
    if(!create_pairs_from_list(sources, first_source_ep, non_enacting) ||
            !create_pairs_from_list(sinks, first_sink_ep, non_enacting))
    {
        return FALSE;
    }

    /* If the rm_list is no longer empty then start a timer if it hasn't happened already */
    if (!rm_timer_running && rm_list != NULL)
    {
        timer_schedule_bg_event_in(RATEMATCH_DECISION_PERIOD, ratematch_decision, NULL);
        rm_timer_running = TRUE;
        rm_timer_slow_period = RATEMATCH_DECISION_SLOW_RATIO - 1;
    }

    return TRUE;
}

/****************************************************************************
Public Function Definitions
*/

bool setup_ratematching(unsigned ep_id)
{
    ENDPOINT_LIST *sources = NULL, *sinks = NULL;

    bool result = TRUE;

    /* First get hold of all the real endpoints in the graph of this endpoint,
     * then we'll work out how they're related in a ratematching context. */
    find_graph_real_sources_and_sinks(ep_id, &sources, &sinks);

    result = ratematch_real_eps(sources, sinks);

    pfree(sources);
    pfree(sinks);

    return result;
}

/* stream_rm_endpoints_have_same_clock_source */
bool stream_rm_endpoints_have_same_clock_source(ENDPOINT *ep1, ENDPOINT *ep2)
{
    ENDPOINT_TIMING_INFORMATION tinfo_ep1;
    ENDPOINT_TIMING_INFORMATION tinfo_ep2;
    bool both_local;

    ep1->functions->get_timing_info(ep1, &tinfo_ep1);
    ep2->functions->get_timing_info(ep2, &tinfo_ep2);

    both_local = tinfo_ep1.locally_clocked && tinfo_ep2.locally_clocked;

    return have_same_clock_source(ep1, ep2, both_local);
}


static void remove_rm_pairs(ENDPOINT_LIST *remove_list)
{
    RATEMATCH_PAIR **p, *fnd_pair;
    unsigned i;
    patch_fn_shared(stream_ratematch);

    for (i = 0; i < remove_list->length; i++)
    {
        /* This implementation simply looks for the endpoint being disconnected.
         * This is sufficient for clearing up a chain. It might not be sufficient
         * when there is more than one ratematching pair for a graph if a subgraph
         * is reused for another use case. */

        for (p = &rm_list; *p != NULL; p = &((*p)->next))
        {
            /* If this endpoint is in this ratematching pair then we can remove it
             * and break out.
             */
            if ((*p)->ep1 == remove_list->ep[i] || (*p)->ep2 == remove_list->ep[i])
            {
                fnd_pair = *p;
                *p = fnd_pair->next;
                /* The enacting endpoint is no longer responsible for enactment at
                 * this time. */
                ENDPOINT_CONFIGURE(fnd_pair->enacting_ep,
                                   EP_RATEMATCH_ENACTING, (uint32)FALSE);

                /*If any endpoints are following the enacting endpoint we need to adjust*/
                ratematch_undo_point_to_head(fnd_pair->enacting_ep);

                pdelete(fnd_pair);
                /* It's possible at this point that rm_list is empty at this point
                 * and we could stop the background timer here. But it will disable
                 * itself if it finds that the list has emptied so just let it
                 * decide for itself. */
                if (NULL == *p)
                {
                    /* If the list is empty then *p is NULL and iterating the
                     * loop again will dereference a NULL pointer! Having reached
                     * the end of the list this search is finished anyway.
                     */
                    break;
                }
            }
        }
    }
}
/*
 * This is hard we need to work out all the real sources/sinks that are still
 * in this chain. Remove any pairs that had any one or more of the the real sources
 * in and then... Re-evaluate the chain.
 */
bool cease_ratematching(unsigned ep_id)
{
    ENDPOINT_LIST *sources = NULL, *sinks = NULL;
    bool result = TRUE;
    patch_fn_shared(stream_ratematch);

    find_graph_real_sources_and_sinks(ep_id, &sources, &sinks);

    if (NULL == sources && NULL == sinks)
    {
        /* There isn't anything that could have been affected and nothing has
         * been allocated so we can return */
        return TRUE;
    }

    if (sources)
    {
        remove_rm_pairs(sources);
    }
    if (sinks)
    {
        remove_rm_pairs(sinks);
    }

    result = ratematch_real_eps(sources, sinks);

    pfree(sources);
    pfree(sinks);
    return result;
    /*If rm_list is NULL then the timer will cancel itself next time it fires */
}

ENDPOINT *stream_rm_get_clk_src_from_pair(ENDPOINT *ep)
{
    RATEMATCH_PAIR *cur_pair = rm_list;
    patch_fn_shared(stream_ratematch);

    if (ep != NULL)
    {
        return NULL;
    }

    /* Search through the short list of RM pairs for those containing the supplied endpoint
     * return the endpoint that determines the rate seen at the boundary of ep */

    while (cur_pair != NULL)
    {
        if (cur_pair->ep1 == ep)
        {
            /* If the provided endpoint performs enacting then the rate at the
             * boundary is that of the other clock in the pair. Otherwise it is
             * the rate of the provided endpoint.
             */
            if (ep == cur_pair->enacting_ep)
            {
                return cur_pair->ep2;
            }
            else
            {
                return ep;
            }
        }
        else if (cur_pair->ep2 == ep)
        {
            /* If the provided endpoint performs enacting then the rate at the
             * boundary is that of the other clock in the pair. Otherwise it is
             * the rate of the provided endpoint.
             */
            if (ep == cur_pair->enacting_ep)
            {
                return cur_pair->ep1;
            }
            else
            {
                return ep;
            }
        }
        cur_pair = cur_pair->next;
    }
    /* Not in a RM pair, so the rate must be the same as that of the endpoint itself */
    return ep;
}

unsigned stream_rm_get_rate_adjustment(ENDPOINT *ep_a,unsigned src_rate,ENDPOINT *ep_b,unsigned sink_rate)
{
    ENDPOINT *clk_a = stream_rm_get_clk_src_from_pair(ep_a);
    ENDPOINT *clk_b = stream_rm_get_clk_src_from_pair(ep_b);
    patch_fn_shared(stream_ratematch);

    /* rates not found in rate matching, return zero - no adjustment */
    if (clk_a == NULL || clk_b == NULL)
    {
        return 0;
    }

    /* Get rates if from associated endpoints */
    if(clk_a != ep_a)
    {
         uint32 value = src_rate;
         if (stream_get_endpoint_config(clk_a, EP_RATEMATCH_RATE,&value))
         {
             src_rate = (unsigned)value;
         }
    }

    if(clk_b != ep_b)
    {
         uint32 value = sink_rate;
         if (stream_get_endpoint_config(clk_b, EP_RATEMATCH_RATE,&value))
         {
             sink_rate = (unsigned)value;
         }
    }

    /* Compute adjustment */
    return calc_diff((int)src_rate, (int)sink_rate); 
}

#ifdef UNIT_TEST_BUILD
extern unsigned get_num_rm_pairs(void);
extern unsigned* get_rm_pairs_dump(void);

unsigned get_num_rm_pairs(void)
{
    unsigned cnt = 0;
    RATEMATCH_PAIR *cur_pair;
    for (cur_pair = rm_list; cur_pair != NULL; cur_pair = cur_pair->next, cnt++);

    return cnt;
}

unsigned* get_rm_pairs_dump(void)
{
    unsigned count = get_num_rm_pairs();
    /* Caller frees */
    unsigned* dump = xzpnewn(count*3+1, unsigned);
    unsigned* p = dump;
    *p++ = count;
    RATEMATCH_PAIR *cur_pair;
    for (cur_pair = rm_list; cur_pair != NULL; cur_pair = cur_pair->next)
    {
        *p++ = cur_pair->ep1->id;
        *p++ = cur_pair->ep2->id;
        *p++ = cur_pair->enacting_ep->id;
    }
    return dump;
}
#endif /* UNIT_TEST_BUILD */
