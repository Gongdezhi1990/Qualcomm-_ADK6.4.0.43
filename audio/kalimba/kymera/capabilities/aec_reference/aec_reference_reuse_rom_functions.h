/****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/*
 * Each macro enables reusing the rom function instead of repeating
 * that in downloadable. This is only suitable for functions that
 * haven't changed at all compared to ROM build. If the function
 * changes later, its macro needs to be removed from this list.
 */
#ifndef AEC_REFERENCE_REUSE_ROM_FUNCTIONS_H
#define AEC_REFERENCE_REUSE_ROM_FUNCTIONS_H
#define AEC_REFERENCE_LATENCY_OP_USE_ROM
#define AECREF_CALC_REF_RATE_USE_ROM
#define AEC_REF_PURGE_MICS_USE_ROM
#define AEC_REFERENCE_GET_SCHED_INFO_USE_ROM
#define AEC_REFERENCE_GET_DATA_FORMAT_USE_ROM
#define AEC_REFERENCE_OPMSG_OBPM_SET_CONTROL_USE_ROM
#define AEC_REFERENCE_OPMSG_OBPM_GET_PARAMS_USE_ROM
#define AEC_REFERENCE_OPMSG_OBPM_GET_DEFAULTS_USE_ROM
#define AEC_REFERENCE_OPMSG_OBPM_SET_PARAMS_USE_ROM
#define AEC_REFERENCE_OPMSG_OBPM_GET_STATUS_USE_ROM
#define UPS_PARAMS_AEC_USE_ROM
#define AEC_REFERENCE_OPMSG_SET_UCID_USE_ROM
#define AEC_REFERENCE_SPKR_TTP_DISCARD_SAMPLES_USE_ROM
#define CBOPS_AEC_REF_SPKR_OP_PRE_MAIN_USE_ROM
#endif
