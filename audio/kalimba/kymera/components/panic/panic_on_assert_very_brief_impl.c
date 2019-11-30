/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file panic_on_assert_very_brief_impl.c
 * \ingroup panic
 *
 * Panic on invalid assertion (implementation)
 */

#include "panic/panic_private.h"

/**
 * This function is called from assembly code that captures the callers PC
 * This keeps the conditional logging in "C" land.
 */
/*lint -function(exit, panic_on_assert_very_brief_impl) i.e. NORETURN */
extern void panic_on_assert_very_brief_impl(
    uint32 assert_addr,
    uint16 line_num
) NORETURN;

void panic_on_assert_very_brief_impl(
    uint32 assert_addr,
    uint16 line_num
)
{
    UNUSED(assert_addr);

    L0_DBG_MSG3("assert @ 0x%04x%04x (line %d)",
            (uint16)(assert_addr >> 16), (uint16)assert_addr, line_num
    );

    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}
