/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup override Override module
 * \ingroup opmgr
 *
 * \file  opmgr_endpoint_override.h
 * \ingroup override
 *
 * \brief
 * Operator Manager, Override module public header file.
 * This file contains public override functions and types.
 */

#include "cbops_mgr/cbops_mgr.h"

/****************************************************************************
Public Type Declarations
*/
typedef struct CBOPS_PARAMETERS CBOPS_PARAMETERS;

typedef void* OVERRIDE_EP_HANDLE;

/****************************************************************************
Public Macro Declarations
*/
/** Empty flag. */
#define EMPTY_FLAG (0)

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief  Function is used for allocating the necessary memory for the cbops parameters
 * containing cbops operators indicated by the cbops_flags.
 *
 * \param  flags_to_add - flags indicating which cbops operators will be added.
 * \param  flags_to_remove - flags indicating which cbops operators will be removed.
 *
 * \return Pointer to cbops parameters.
 */
extern CBOPS_PARAMETERS* create_cbops_parameters(uint32 flags_to_add, uint32 flags_to_remove);

/**
 * \brief  Function to free memory allocated for cbops parameters.
 *
 * \param parameters - pointer to cbops parameters.
 */
extern void free_cbops_parameters(CBOPS_PARAMETERS* parameters);

/**
 *  Function for passing all the cbops parameters to a cbops manager.
 *
 * \param parameters - pointer to cbops parameters.
 * \param cbops_mgr - pointer to the cbops manager who will receive the cbops parameters
 * \param source - needed for cbops manager.
 * \param sink - needed for cbops manager.
 *
 * \return boolean indicating if the operation was successful.
 */
extern bool opmgr_override_pass_cbops_parameters(CBOPS_PARAMETERS* parameters, cbops_mgr *cbops_mgr, tCbuffer *source, tCbuffer *sink);

/**
 * \brief  Function to set the shift amount for the cbops parameters.
 *
 * \param  parameters - pointer to cbops parameters.
 * \param  shift_amount - shift amount in integers.
 *
 * \return Indicates if the operation was successful.
 */
extern bool cbops_parameters_set_shift_amount(CBOPS_PARAMETERS* parameters, int shift_amount);

/**
 * \brief  Function to get the shift amount from the cbops parameters.
 *         The sign of the value should be decided by the operator.
 *
 * \param  cbops_parameters - pointer to cbops parameters.
 *
 * \return Amount of shift.
 */
extern int get_shift_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters);

/**
 * \brief  Function to check if dc remove is in operation.
 *
 * \param  cbops_parameters - pointer to cbops parameters
 *
 * \return returns True if DC remove is present False otherwise.
 */
extern bool get_dc_remove_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters);

/**
 * \brief  Function to check if U law algorithm is in use
 *
 * \param  cbops_parameters - pointer to cbops parameters
 *
 * \return returns true if the U law algorithm is present false otherwise.
 */
extern bool get_u_law_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters);

/**
 * \brief  Function to check if A law algorithm is in use
 *
 * \param  cbops_parameters - pointer to cbops parameters
 *
 * \return returns true if the A law algorithm is present false otherwise.
 */
extern bool get_a_law_from_cbops_parameters(CBOPS_PARAMETERS* cbops_parameters);

/**
 * \brief  Function to get the sampling rate of the overridden endpoint
 *
 * \param  ep_hdl - Endpoint handle
 * \param  *value - Pointer which contain the value of the sampling rate.
 *
 * \return Success or failure.
 */
extern bool get_override_ep_sample_rate(OVERRIDE_EP_HANDLE ep_hdl, uint32* value);

/**
 * \brief  Function to set the ADC gain the overridden endpoint. Returns false if the
 *         operation was not successful. Only codec endoints can set the gain.
 *
 * \param  ep_hdl - Endpoint handle
 * \param  value - The new value of the ADC gain of a codec.
 *
 * \return Success or failure.
 */
extern bool set_override_ep_gain(OVERRIDE_EP_HANDLE ep_hdl, uint32 value);

/**
 * \brief  Function to set the ratemach enacting for the overridden endpoint.
 *         Only supported when the ratematching ability of the overriden endpoint is HW.
 *
 * \param  ep_hdl - Endpoint handle
 * \param  value - The new value of the ratematch enacting.
 *
 * \return Success or failure.
 */
extern bool set_override_ep_ratematch_enacting(OVERRIDE_EP_HANDLE ep_hdl, uint32 value);

/**
 * \brief  Function to get the ratematching ability for the overridden endpoint.
 *
 * \param  ep_hdl - Endpoint handle
 * \param  value - Pointer which contain the value of the ratematching ability.
 *
 * \return Success or failure.
 */
extern bool get_override_ep_ratematch_ability(OVERRIDE_EP_HANDLE ep_hdl, uint32* value);


/**
 * \brief  Function to get the ratematching rate for the overridden endpoint.
 *
 * \param  ep_hdl - Endpoint handle
 * \param  value - Pointer which contain the value of the ratematching ability.
 *
 * \return Success or failure.
 */
extern bool get_override_ep_ratematch_rate(OVERRIDE_EP_HANDLE ep_hdl, uint32* value);

/**
 * \brief  Function to check if the overridden endpoint is clocked locally or remotely.
 *
 * \param  ep_hdl - Endpoint handle
 *
 * \return TRUE if the overridden endpoint is locally clocked, FALSE otherwise.
 */
extern bool is_override_ep_locally_clocked(OVERRIDE_EP_HANDLE ep_hdl);


/**
 * \brief  Function to tests whether two overridden endpoints use the same clock source.
 *
 * \param  ep_hdl1 - The handle for the operator endpoint which overrides a real endpoint.
 * \param  ep_hdl2 - The second handle for the operator endpoint which overrides another real endpoint.
 *
 * \return TRUE if the two overridden endpoints have same clock source, FALSE otherwise.
 */
extern bool override_endpoints_have_same_clock_source(OVERRIDE_EP_HANDLE ep_hdl1,OVERRIDE_EP_HANDLE ep_hdl2);

/**
 * \brief  Function to set the ratemaching adjustment for the overridden endpoint.
 *         Only supported when the ratematching ability of the overriden endpoint is HW.
 *
 * \param  ep_hdl - Endpoint handle
 * \param  value - The new value of the rate adjustment.
 *
 * \return Success or failure.
 */
extern bool set_override_ep_ratematch_adjustment(OVERRIDE_EP_HANDLE ep_hdl, uint32 value);


/**
 * \brief  Function to set the endpoint handle of the endpoint associated with an operator terminal
 *
 * \param  op_id - The operator id.
 * \param  terminal_id - The terminal ID of the operator which is connected to the operator endpoint
           which overrides the real endpoint.
 *
 * \return endpoint handle of the operator terminal's endpoint
 */
extern OVERRIDE_EP_HANDLE get_overriden_endpoint(unsigned int op_id, unsigned int terminal_id);

/**
 * \brief Finds the endpoint whose clock source is seen at the kymera side
 * boundary of an endpoint.
 *
 * \param ep_hdl  The handle of the endpoint whose boundary the clock source is requested of
 *
 * \return The handle of the endpoint whose clock source is present at the boundary. This may
 * be ep_hdl.
 */
extern OVERRIDE_EP_HANDLE override_get_clk_src_of_endpoint(OVERRIDE_EP_HANDLE ep_hdl);

/**
 * \brief computes the rate adjustment between endpoints
 *
 * \param ep_src Handle of the source endpoint
 * \param src_rate Rate of source if not enacting endpoint
 * \param ep_sink Handle of sink the endpoint
 * \param sink_rate Rate of sink if not enacting endpoint
 *
 * \return The rate adjustment, If endpoints invalid then zero.
 */
extern unsigned override_get_rate_adjustment(OVERRIDE_EP_HANDLE ep_src,unsigned src_rate,OVERRIDE_EP_HANDLE ep_sink,unsigned sink_rate);


