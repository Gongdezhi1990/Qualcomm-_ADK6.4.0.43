// Copyright (c) 2016 Qualcomm Technologies International, Ltd.
//   %%version
/*
 * \file
 * \brief SLT fingerprint and table pointer
 *
 */

#include "slt/apps_fingerprint.h"

    .section CONST_SLT, "a"

$_slt_fingerprint:
    .word APPS_SLT_FINGERPRINT

$_slt_pointer:
    .word $_slt_table_start
