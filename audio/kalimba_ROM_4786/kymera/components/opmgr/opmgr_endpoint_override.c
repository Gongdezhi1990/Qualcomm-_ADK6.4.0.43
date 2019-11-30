/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  opmgr_endpoint_override.c
 * \ingroup override
 *
 * Override module from Operator Manager.
 * This file contains the override specific operations for endpoints.
 *
 */
#include "opmgr_private.h"
#include "stream/stream_endpoint.h"
#include "stream/stream_for_override.h"
#include "cbops_mgr/cbops_flags.h"
#include "cbops_mgr/cbops_mgr.h"
#include "opmgr_endpoint_override.h"

/****************************************************************************
Private Type Declarations
*/

/**
* cbops_parameters is the type which is used to encapsulate all the necessary information
* about a cbops chain implemented by a cbops manager. This encapsulated information
* can be accessed by different api functions for endpoints/operators not using cbops_mgr.
*/
struct CBOPS_PARAMETERS
{
    /** cbops operators are represented as a flag in the 'flags' bit-field.
     * Each flag from cbops_flags.h represents a cbops operator in the flags bit-field. */
    uint32 flags_to_add;

    /** cbops operators are represented as a flag in the 'flags' bit-field.
     * Each flag from cbops_flags.h represents a cbops operator in the flags bit-field. */
    uint32 flags_to_remove;

    /** Some cbops operators has additional information (for ex: cbops shift has shift
     * amount). These additional information are saved after the end of this structure.
     * This field has a size of 0 and is only used to get the address of the memory area
     * after this structure. */
    uintptr_t data[];
};

typedef struct
{
    /** The number of bits to left shift by (signed hence -ve = right shift)
     * Same as in cbops_shift_c.h, cbops_dc_remove_c.h, cbops_dither_and_shift_c.h etc*/
    int shift_amount;
}CBOPS_SHIFT_DATA;

#ifdef DESKTOP_TEST_BUILD
/* Used for testing only.
*/
typedef struct
{
    /* This is a signed value representing the difference between the rate data
     * is being produced and consumed. */
    unsigned int rm_adjust_amount;
}CBOPS_RATEADJUST_DATA;

typedef struct
{
    /* The type of dither to use.*/
    unsigned dither_type;
    /* Indicates whether the compressor is used.*/
    bool enable_compressor;
}CBOPS_DITHER_DATA;
#endif
/****************************************************************************
Private Function Definitions
*/

/**
 * \brief  Returns the size of the data needed for the specific cbops operator.
 *
 * \param  cbops_operator_flag - CBOPS operator as described by the flag.
 *
 * \return size of the cbops operator data(bytes).
 */
static unsigned get_size_of_cbops_operator(uint32 cbops_operator_flag)
{
    switch(cbops_operator_flag)
    {
        case CBOPS_SHIFT:
            return sizeof(CBOPS_SHIFT_DATA);
#ifdef DESKTOP_TEST_BUILD
/* Added for testing
*/
        case CBOPS_MIX://DITHER:
            return sizeof(CBOPS_DITHER_DATA);
        case CBOPS_RATEADJUST:
            return sizeof(CBOPS_RATEADJUST_DATA);
#endif
        default:
            /* no data for this cbops operator*/
            return 0;
    }
}

/**
 * \brief  Returns the size of all cbops operator data which is present in the cbops_flags.
 *
 * \param  cbops_flags - cbops operators as described by the flags.
 *
 * \return size of every cbops operators present in the cbops_flags
 */
static unsigned get_size_of_every_cbops_operator_from_the_flag(uint32 cbops_flags)
{
    unsigned total_size = 0, counter = 0;

    for(; cbops_flags > 0 ; cbops_flags >>= 1, counter++)
    {
        /* Check if the cbops operator is present. */
        if (cbops_flags & 1)
        {
            /* get the size of the cbops operator and add to the summ */
            total_size += get_size_of_cbops_operator(1<<counter);
        }
    }

    return total_size;
}

/**
 * \brief  Returns the position of the cbops operator data in the cbops parameters data.
 *
 * \param  parameters - pointer to cbops parameters.
 * \param  cbops_flag - specific cbops operator as described by the flag.
 *
 * \return position of the cbops operator data in cbops parameters.
 */
static unsigned get_position_of_cbops_operator_data(CBOPS_PARAMETERS* parameters, uint32 cbops_flag)
{
    unsigned position = 0, counter = 0;
    uint32 flags = parameters->flags_to_add;
    for(; flags > 0 &&  cbops_flag > 1<<counter ; flags >>= 1, counter++)
    {
        /* Check if the cbops operator is present. */
        if (flags & 1)
        {
            /* cbops flags */
            position += get_size_of_cbops_operator(1<<counter);
        }
    }

    return position;
}


/**
 * \brief  Returns the address of the cbops operator data associated with the cbops flag.
 *
 * \param  parameters - pointer to cbops parameters.
 * \param  cbops_flag - specific cbops operator as described by the flag.
 *
 * \return pointer to the cbops operator data.
 */
static void* get_address_of_cbops_operator_data(CBOPS_PARAMETERS* parameters, uint32 cbops_flag)
{
    return (void *)((uintptr_t)(parameters->data) + (unsigned)get_position_of_cbops_operator_data(parameters, cbops_flag));
}

/****************************************************************************
Public Function Definitions
*/
/* create_cbops_parameters */
CBOPS_PARAMETERS* create_cbops_parameters(uint32 flags_to_add, uint32 flags_to_remove)
{
    CBOPS_PARAMETERS* parameters;
    unsigned data_size = get_size_of_every_cbops_operator_from_the_flag(flags_to_add);

    if (flags_to_add & flags_to_remove)
    {
        /* The two flags overlap. It makes no sense to add and remove a cbops operator
         * in the same time. */
        return NULL;
    }
    parameters = pmalloc(sizeof(CBOPS_PARAMETERS)  + data_size);
    if (!parameters)
    {
        return NULL;
    }
    parameters->flags_to_add = flags_to_add;
    parameters->flags_to_remove = flags_to_remove;

    return parameters;
}

/* free_cbops_parameters */
void free_cbops_parameters(CBOPS_PARAMETERS* parameters)
{
    /* It's a function and not a define to have better memory debug.*/
    pfree(parameters);
}

/* cbops_parameters_set_shift_amount */
bool cbops_parameters_set_shift_amount(CBOPS_PARAMETERS* parameters, int shift_amount)
{
    CBOPS_SHIFT_DATA* shift_data;
    if (parameters->flags_to_add & CBOPS_SHIFT)
    {
        shift_data = (CBOPS_SHIFT_DATA*)(get_address_of_cbops_operator_data(parameters,CBOPS_SHIFT));
        shift_data->shift_amount = shift_amount;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

/* pass_cbops_parameters_to_cbops_mgr */
bool opmgr_override_pass_cbops_parameters(CBOPS_PARAMETERS* parameters, cbops_mgr *cbops_mgr, tCbuffer *source, tCbuffer *sink)
{
    bool success = TRUE;
    /* variable used for checking all the flags in the bit-field. */
    uint32 current_operator_flag = 1;

    /* Go through all cbops operators and check changes for all the cbops flags. */
    for(; (current_operator_flag < CBOPS_LAST_FLAG) && success; current_operator_flag <<= 1)
    {
        if (parameters->flags_to_add & current_operator_flag)
        {
            /* New cbops operator was added.
             * Test if the cbops operator has some additional information by checking
             * the data size needed by the cbops operator.*/
            if (get_size_of_cbops_operator(current_operator_flag) == 0)
            {
                CBOP_VALS vals;
                vals.rate_adjustment_amount = NULL;
                /* Cbops operator without additional data. */
                success = cbops_mgr_append(cbops_mgr, (unsigned)current_operator_flag, 1, &source, &sink, &vals, FALSE);
            }
            else if( current_operator_flag & CBOPS_SHIFT)
            {
                /* cbops shift has additional data. */
                CBOP_VALS vals;
                vals.shift_amount = get_shift_from_cbops_parameters(parameters);
                success = cbops_mgr_append(cbops_mgr, (unsigned)current_operator_flag, 1, &source, &sink, &vals, FALSE);
            }
            else
            {
                /* This cbops operator is not yet supported by the override module. */
                success = FALSE;
            }
            /* TODO: Consider adding additional cbops operators. */
        }
        else if (parameters->flags_to_remove & current_operator_flag)
        {
            /* cbops operator was removed. */
            success = cbops_mgr_remove(cbops_mgr, (unsigned)current_operator_flag, 1, &source, &sink, FALSE);
        }
    }
    return success;
}


/* get_shift_from_cbops_parameters */
int get_shift_from_cbops_parameters(CBOPS_PARAMETERS* parameters)
{
    if(parameters->flags_to_add & CBOPS_SHIFT)
    {
        return ((CBOPS_SHIFT_DATA*)get_address_of_cbops_operator_data(parameters,CBOPS_SHIFT))->shift_amount;
    }
    else
    {
        return 0;
    }
}

/* get_dc_remove_from_cbops_parameters */
bool get_dc_remove_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters)
{
    return cbops_parameters->flags_to_add & CBOPS_DC_REMOVE;
}

/* get_u_law_from_cbops_parameters */
bool get_u_law_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters)
{
    return cbops_parameters->flags_to_add & CBOPS_U_LAW;
}

/* get_a_law_from_cbops_parameters */
bool get_a_law_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters)
{
    return cbops_parameters->flags_to_add & CBOPS_A_LAW;
}

/* get_override_ep_sample_rate */
bool get_override_ep_sample_rate(OVERRIDE_EP_HANDLE ep_hdl, uint32* value)
{
    ENDPOINT_GET_CONFIG_RESULT result;
    result.u.value = 0;
    bool success = stream_get_connected_to_endpoint_config((ENDPOINT*)ep_hdl, STREAM_INFO_KEY_AUDIO_SAMPLE_RATE, &result);
    *value = result.u.value;
    return success;
}

/* set_override_ep_gain */
bool set_override_ep_gain(OVERRIDE_EP_HANDLE ep_hdl, uint32 value)
{
    ENDPOINT *ep = (ENDPOINT*)ep_hdl;
    /* Set the gain for the overridden endpoint  */
    if (ep->direction == SINK) /* connected to SOURCE */
    {
        return stream_configure_connected_to_endpoint(ep, EP_SET_INPUT_GAIN, value);
    }
    else
    {
        return stream_configure_connected_to_endpoint(ep, EP_SET_OUTPUT_GAIN, value);
    }
}

/* get_override_ep_ratematch_enacting */
bool set_override_ep_ratematch_enacting(OVERRIDE_EP_HANDLE ep_hdl, uint32 value)
{
    return stream_configure_connected_to_endpoint((ENDPOINT*)ep_hdl, EP_RATEMATCH_ENACTING, value);
}

/* get_override_ep_ratematch_ability */
bool get_override_ep_ratematch_ability(OVERRIDE_EP_HANDLE ep_hdl, uint32* value)
{
    ENDPOINT_GET_CONFIG_RESULT result;
    result.u.value = RATEMATCHING_SUPPORT_NONE;
    bool success = stream_get_connected_to_endpoint_config((ENDPOINT*)ep_hdl, EP_RATEMATCH_ABILITY, &result);
    *value = result.u.value;
    return success;
}

/* get_override_ep_ratematch_rate */
bool get_override_ep_ratematch_rate(OVERRIDE_EP_HANDLE ep_hdl, uint32* value)
{
    ENDPOINT_GET_CONFIG_RESULT result;
    result.u.value = (1 << STREAM_RATEMATCHING_FIX_POINT_SHIFT);
    bool success = stream_get_connected_to_endpoint_config((ENDPOINT*)ep_hdl, EP_RATEMATCH_RATE, &result);
    *value = result.u.value;
    return success;
}

/* is_override_ep_locally_clocked */
bool is_override_ep_locally_clocked(OVERRIDE_EP_HANDLE ep_hdl)
{
    return stream_connected_to_endpoint_is_locally_clocked((ENDPOINT*)ep_hdl);
}

/* override_endpoints_have_same_clock_source */
bool override_endpoints_have_same_clock_source(OVERRIDE_EP_HANDLE ep_hdl1,OVERRIDE_EP_HANDLE ep_hdl2)
{
    return stream_connected_to_endpoints_have_same_clock_source((ENDPOINT*)ep_hdl1,(ENDPOINT*)ep_hdl2);
}

/* get_override_ep_ratematch_adjustment */
bool set_override_ep_ratematch_adjustment(OVERRIDE_EP_HANDLE ep_hdl, uint32 value)
{
    return stream_configure_connected_to_endpoint((ENDPOINT*)ep_hdl, EP_RATEMATCH_ADJUSTMENT, value);
}

/* get_overriden_endpoint*/
OVERRIDE_EP_HANDLE get_overriden_endpoint(unsigned int op_id, unsigned int terminal_id)
{
   return (OVERRIDE_EP_HANDLE)stream_endpoint_from_extern_id(get_opidep_from_opid_and_terminalid(op_id,terminal_id));
}

/* get_clk_src_of_endpoint */
OVERRIDE_EP_HANDLE override_get_clk_src_of_endpoint(OVERRIDE_EP_HANDLE ep_hdl)
{
    return (OVERRIDE_EP_HANDLE)stream_rm_get_clk_src_from_pair((ENDPOINT *)ep_hdl);
}

/* override_get_endpoint_rate */
unsigned override_get_rate_adjustment(OVERRIDE_EP_HANDLE ep_src,unsigned src_rate,OVERRIDE_EP_HANDLE ep_sink,unsigned sink_rate)
{
   return stream_rm_get_rate_adjustment((ENDPOINT *)ep_src,src_rate,(ENDPOINT *)ep_sink,sink_rate);
}

