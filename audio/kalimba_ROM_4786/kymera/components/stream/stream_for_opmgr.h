/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup stream Stream Subsystem
 *
 * \file  stream_for_ompgr.h
 *
 * stream header file. <br>
 * This file contains stream functions that are only publicly available to opmgr. <br>
 */

#ifndef STREAM_FOR_OPMGR_H
#define STREAM_FOR_OPMGR_H

/****************************************************************************
Include Files
*/

#ifdef INSTALL_UNINTERRUPTABLE_ANC

#include "stream.h"
#include "hal_audio_anc.h"
#include "accmd_prim.h"

#endif /* INSTALL_UNINTERRUPTABLE_ANC */

/**
 * \brief Kicks an endpoint from outside of streams.
 *
 * \param ep The endpoint to call the kick function of.
 * \param kick_dir The direction that the kick has come from.
 *
 */
void stream_if_kick_ep(ENDPOINT *ep, ENDPOINT_KICK_DIRECTION kick_dir);

/**
 * \brief Given an endpoint id returns the endpoint that is connected to it.
 *
 * \param ep_id The id of the endpoint to work from.
 *
 * \return The endpoint which is connected to that represented by ep_id. If
 * there is no connection then NULL is returned.
 */
ENDPOINT *stream_get_connected_ep_from_id(unsigned ep_id);

/**
 * \brief Accessor for getting the ID of an endpoint.
 *
 * \param ep The endpoint to get the ID of.
 *
 * \return The ID of the ep.
 */
unsigned stream_ep_id_from_ep(ENDPOINT *ep);

/**
 * \brief Generic function used to ask an endpoint not to bother kicking
 * the endpoint it is connected to.
 *
 * \note Not every endpoint will take notice of this request.
 *
 * \param The endpoint that to ask to stop propagating kicks.
 */
extern void stream_disable_kicks_from_endpoint(ENDPOINT *ep);

/**
 * Cancel the kick at the end of the in place chain.
 *
 * \param ep - Pointer to the endpoint  to start the traverse wiht.
 */
extern void in_place_cancel_tail_kick(ENDPOINT *ep);

#ifdef INSTALL_UNINTERRUPTABLE_ANC
/**
 * \brief Configure the ANC DC filters and small low-pass filters.
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param shift Shift used to configure filter.
 *
 */
extern void stream_anc_set_anc_filter_shift(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 shift);

/**
 * \brief Configure the ANC path gains.
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param gain Gain applied to filter (8 bit linear gain value).
 *
 */
extern void stream_anc_set_anc_gain(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 gain);

/**
 * \brief Configure the ANC path gain shift (gain exponent).
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param shift Gain shift applied to filter (4 bit shift value).
 *
 */
extern void stream_anc_set_anc_gain_shift(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 shift);

/**
 * \brief Configure the ANC gain enable (used in Adaptive ANC)
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param enable Enable/disable adaptive operation.
 *
 */
extern void stream_anc_set_anc_gain_enable(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 enable);

/**
 * \brief Configure ANC miscellaneous hardware options
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param en_and_mask Combined bit field of enables and selection mask for ANC hardware control options.
 *
 */
extern void stream_anc_set_anc_control(ACCMD_ANC_INSTANCE anc_instance, uint32 en_and_mask);

/**
 * \brief Configure an ANC IIR filter (sets the coefficients)
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param num_coeffs Number of coefficients.
 * \param coeffs Pointer to an array of IIR coefficients.
 *
 */
extern void stream_anc_set_anc_iir_coeffs(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 num_coeffs, const uint16 *coeffs);

/**
 * \brief Select the currently active IIR coefficient set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param coeff_set coefficient set 0: Foreground, 1: background.
 *
 * \note  Coefficients for the FFA, FFB and FB IIR filters are banked
 *        (LPF shift coefficients are not banked)
 */
extern void stream_anc_select_active_iir_coeffs(ACCMD_ANC_INSTANCE anc_instance, hal_anc_bank_select_type coeff_set);

/**
 * \brief Copy the foreground coefficient set to the background coefficient set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 *
 * \note  Coefficients for the FFA, FFB and FB IIR filters are banked
 *        (LPF shift coefficients are not banked)
 */
extern void stream_anc_update_background_iir_coeffs(ACCMD_ANC_INSTANCE anc_instance);

/**
 * \brief Select the currently active gain set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param gain_set gain set 0: Foreground, 1: background.
 *
 * \note: Gains for the FFA, FFB, and FB LPF are shadowed
 *        (but gain shifts are not)
 */
extern void stream_anc_select_active_gains(ACCMD_ANC_INSTANCE anc_instance, hal_anc_bank_select_type gain_set);

/**
 * \brief Copy the foreground gain set to the background gain set
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 *
 * \note  Gains for the FFA, FFB, and FB LPF are shadowed
 *        (but gain shifts are not)
 */
extern void stream_anc_update_background_gains(ACCMD_ANC_INSTANCE anc_instance);

/**
 * \brief Configure an ANC LPF filter (sets the LPF coefficients)
 *
 * \param anc_instance ANC instance ID (e.g. ANC0, ANC1).
 * \param path_id ANC input path ID (e.g. FFA, FFB, FB).
 * \param shift1 Coefficient 1 expressed as a shift.
 * \param shift2 Coefficient 2 expressed as a shift.
 *
 */
extern void stream_anc_set_anc_lpf_coeffs(ACCMD_ANC_INSTANCE anc_instance, ACCMD_ANC_PATH path_id, uint16 shift1, uint16 shift2);

/**
 * \brief Place-holder function for patching purposes.
 *
 * \param ptr Pointer to function parameters (to be used as required)
 *
 */
extern void stream_anc_user1(void *ptr);

/**
 * \brief Place-holder function for patching purposes.
 *
 * \param ptr Pointer to function parameters (to be used as required)
 *
 */
extern void stream_anc_user2(void *ptr);

#endif /* INSTALL_UNINTERRUPTABLE_ANC */

#endif /* STREAM_FOR_OPMGR_H */

