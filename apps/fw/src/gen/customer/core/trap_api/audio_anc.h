#ifndef __AUDIO_ANC_H__
#define __AUDIO_ANC_H__
/** \file */
#if TRAPSET_AUDIO_ANC

/**
 *  \brief This command controls the enabling/disabling of ANC.
 * Note that the DSP software/operator framework must be loaded before
 *  enabling/disabling ANC.
 * Check OperatorFrameworkEnable documentation for more details.
 *         
 *  \param anc_enable_0 The anc_enable_0 parameter is a bit field that enables the ANC input and output
 *  paths of the ANC0 instance.
 *  \param anc_enable_1 The anc_enable_1 parameter is a bit field that enables the ANC input and output
 *  paths of the ANC1 instance.
 *  \return TRUE if the ANC stream was enabled, FALSE if the function failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_anc
 */
bool AudioAncStreamEnable(uint16 anc_enable_0, uint16 anc_enable_1);

/**
 *  \brief This command allows configuration of the ANC LPF filter for each ANC channel
 *  instance.
 * Note that the DSP software/operator framework must be loaded before configuring
 *  ANC.
 * Check OperatorFrameworkEnable documentation for more details.
 *         
 *  \param anc_instance The anc_instance parameter is an ID that identifies the ANC channel instance to
 *  which the configuration is to be applied (ANC0/ANC1).
 *  \param path_id The path_id parameter is an ID that specifies the ANC path and hence the LPF
 *  filter instance to be configured.
 *  \param shift1 The shift1 parameter is used to derive the first LPF coefficient.
 *  \param shift2 The shift2 parameter is used to derive the second LPF coefficient.
 *  \return TRUE if the ANC LPF filter was set, FALSE if the function failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_anc
 */
bool AudioAncFilterLpfSet(audio_anc_instance anc_instance, audio_anc_path_id path_id, uint16 shift1, uint16 shift2);

/**
 *  \brief This command allows configuration of the ANC IIR filter for each ANC channel
 *  instance.
 * Note that the DSP software/operator framework must be loaded before configuring
 *  ANC.
 * Check OperatorFrameworkEnable documentation for more details.
 *         
 *  \param anc_instance The anc_instance parameter is an ID that identifies the ANC channel instance to
 *  which the configuration is to be applied (ANC0/ANC1).
 *  \param path_id The path_id parameter is an ID that specifies the ANC path and hence the IIR
 *  filter instance to be configured.
 *  \param num_coeffs The num_coeffs parameter specifies the number of IIR coefficients to be
 *  configured.
 *  \param coeffs An array of filter coefficients; length is given by the num_coeffs parameter.
 *  \return TRUE if the ANC LPF filter was set, FALSE if the function failed.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_audio_anc
 */
bool AudioAncFilterIirSet(audio_anc_instance anc_instance, audio_anc_path_id path_id, uint16 num_coeffs, const uint16 * coeffs);
#endif /* TRAPSET_AUDIO_ANC */
#endif
