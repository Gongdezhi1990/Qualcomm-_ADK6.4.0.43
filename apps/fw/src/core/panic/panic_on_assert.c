/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Panic on invalid assertion (implementation)
 */

#include "panic/panic_private.h"

void panic_on_assert(
    uint16 line_num
)
{
    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}

void panic_on_assert_brief(
    const char *file_path,
    uint16 line_num
)
{
    UNUSED(file_path);

    L0_DBG_MSG2("assert @ %s:%d", file_path, line_num);

    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}

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
                (uint16)(assert_addr >> 16),
                (uint16)assert_addr,
                line_num);

    panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, line_num);
}    
