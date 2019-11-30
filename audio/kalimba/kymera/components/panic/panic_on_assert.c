/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic_on_assert.c
 * \ingroup panic
 * Panic on invalid assertion (implementation)
 */

#include "panic/panic_private.h"

void panic_on_assert(
    uint16 line_num
)
{
    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}
