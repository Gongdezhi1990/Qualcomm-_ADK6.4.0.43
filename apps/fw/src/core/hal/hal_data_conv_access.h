/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Header file for the data_conv register access workaround.
 *
 * \section hal_data_conv_access_introduction Introduction
 * These functions will hijack any register access made by P0 or P1 for the
 * purpose of adding a delay. This delay is needed to avoid sending a
 * transaction before any previous one has completed. Not doing so will result
 * in a locked transaction bus and an inaccessible Apps subsystem. This is a
 * workaround for a hardware bug.
 */

#ifndef _HAL_DATA_CONV_ACCESS_H_
#define _HAL_DATA_CONV_ACCESS_H_

/* Prevent empty compilation units in cov builds. */
#include "hydra/hydra_types.h"


#endif /* _HAL_DATA_CONV_ACCESS_H_ */
