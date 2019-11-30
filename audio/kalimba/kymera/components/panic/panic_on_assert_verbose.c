/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic_on_assert_verbose.c
 * \ingroup panic
 * Panic on invalid assertion (implementation)
 */

#include "panic/panic_private.h"

void panic_on_assert_verbose(
    const char *file_path,
    uint16 line_num,
    const char *assertion_text
)
{
    UNUSED(file_path);
    UNUSED(assertion_text);

    L0_DBG_MSG3("!(%s) @ %s:%d", assertion_text, file_path, line_num);
    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}
