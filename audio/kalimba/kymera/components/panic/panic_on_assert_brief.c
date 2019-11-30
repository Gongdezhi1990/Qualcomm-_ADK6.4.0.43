/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic_on_assert_brief.c
 * \ingroup panic
 * Panic on invalid assertion (implementation)
 */

#include "panic/panic_private.h"

void panic_on_assert_brief(
    const char *file_path,
    uint16 line_num
)
{
    UNUSED(file_path);

    L0_DBG_MSG2("assert @ %s:%d", file_path, line_num);

    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}
