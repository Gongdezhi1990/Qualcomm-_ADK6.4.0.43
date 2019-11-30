/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_log_firm.c
 * \ingroup AUDIO_LOG
 *
 * Audio log (embedded firmware variant implementation).
 *
 * The log key (format string address) and arguments are written to a
 * circular buffer to be picked up via SPI and decoded by the debugger.
 * (E.g. log() in xap2emu.py).
 *
 */

/*****************************************************************************
 * Implementation dependencies.
 ****************************************************************************/

#include "audio_log.h"
#include "platform/pl_intrinsics.h"
#if defined (CHIP_BASE_HYDRA) && !defined(DESKTOP_TEST_BUILD) && !defined(UNIT_TEST_BUILD)
#include "mib/mib.h"
#endif
#include "hal/hal_dm_sections.h"

/*
 * Only compile this implementation variant if the header tells us it's an
 * embedded target
 */
#if defined(AUDIO_LOG_FIRM)

/*
 * Lint chokes badly on stdarg.h.
 *
 * The easiest fix seems to be to hide it (from lint) and fake the macros!
 */
/*lint -D_STDARG_H */
/*lint +dva_list=void* */
/*lint +dva_start(x,y)=((x)=__pclint_va_arg_init) */
/*lint +"dva_arg(x,y)=((x) = (char *) (x) + sizeof(y), ((y *) (x))[-1])" */
/*lint +dva_end(x)=((x) = (void *)0) */
#include <stdarg.h>

#include "hydra_macros.h"
#include "pbc.h"
#include "hal_interrupt.h"
#include "pmalloc/pl_malloc.h"

/*****************************************************************************
 * Private Defines
 ****************************************************************************/

/**
 * Number of elements in circular log buffer.
 *
 * Must be a power of 2
 */
#if !defined(AUDIO_LOG_BUF_SIZE)
#define AUDIO_LOG_BUF_SIZE       1024
#endif

#define AUDIO_LOG_BUF_MASK       (AUDIO_LOG_BUF_SIZE - 1)

/*****************************************************************************
 * Private Data
 ****************************************************************************/

/**
 * Circular debug log buffer.
 *
 * Each event record starts with a key element, and is followed by none or more
 * argument elements. The reader (script) must sync by recognising element
 * values that match debug string addresses!
 *
 * \note Do not change its name unless you are prepared to enhance debug
 * scripts that look for it in elf file.
 */
/*lint -esym(551, debugBuffer) Not accessed, read via SPI only */
static DM2_DEBUG audio_log_elem debugBuffer[AUDIO_LOG_BUF_SIZE];

#ifdef AUDIO_SECOND_CORE
/* Debug buffer is too big to fit in private RAM, so we have to have two
 * differently-named allocations in shared RAM.
 * FIXME: neither Pydbg nor ACAT is prepared to cope with looking up
 * different symbols for different processors yet. */
static DM1_DEBUG audio_log_elem debugBuffer1[AUDIO_LOG_BUF_SIZE];
#endif // AUDIO_SECOND_CORE

/**
 * Next debug log write offset.
 *
 * \note Do not change its name unless you are prepared to enhance debug
 * scripts that look for it in elf file.
 */
static size_t debugBufferPos;

/**
 * Indirection so the size of the buffer can be easily patched. That allows
 * some or nearly all the space to be used for patch code if required.
 */
uint16 debugBufferSizeMask = AUDIO_LOG_BUF_MASK;

/*****************************************************************************
 * Public Data
 ****************************************************************************/

/*volatile*/ audio_log_level debugLogLevel = AUDIO_LOG_DEFAULT;

/*****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * Log a variadic debug event to circular buffer.
 *
 * Common implementation for audio_log_firm_N functions.
 */
/*lint -e(960) -e(1916) Violates MISRA Rule 16.1, but debug/log only */
static void audio_log_firm_variadic
(
    /**
     * Event key.
     *
     * This is formed by casting the address of the format string
     * (declared in unlinked segment using DECLARE_DEBUG_STRING)
     * to a audio_log_elem. The log macros do this cast.
     */
    audio_log_elem event_key,

    /**
     * Number of event args.
     *
     * Can't afford to link, let alone parse the format strings so nargs
     * must be specified.
     */
    size_t n_args,

    /**
     * None or more event args.
     *
     * All args must be of audio_log_elem type.
     */
    ...
)
{
    va_list va_argp;
    audio_log_elem *ourDebugBuffer = debugBuffer;

#ifdef AUDIO_SECOND_CORE
    if( hal_get_reg_processor_id() != 0 )
    {
        ourDebugBuffer = debugBuffer1;
    }
#endif //AUDIO_SECOND_CORE

    interrupt_block();

    /* Push the event key (aka format string's address).
     */
    ourDebugBuffer[debugBufferPos] = event_key;
    ++debugBufferPos;
    debugBufferPos &= debugBufferSizeMask;

    /* Push any and all args to log.
     *
     * FWIW: This loop was 14 XAP instructions long when I last looked.
     * So it should fit in XAP cache.
     */
    va_start(va_argp, n_args);
    while(n_args--)
    {
        ourDebugBuffer[debugBufferPos] = va_arg(va_argp, audio_log_elem);
        ++debugBufferPos;
        debugBufferPos &= debugBufferSizeMask;
    }
    /*lint -e(10) Stronger magic */va_end(va_argp);

    interrupt_unblock();
}

/*****************************************************************************
 * Public Functions
 *
 * One function call for each number of params to avoid passing nr params
 * from every call site (code-space optimisation).
 ****************************************************************************/

void audio_log_firm_0(audio_log_elem event_key)
{
    audio_log_firm_variadic(event_key, 0);
}

void audio_log_firm_1(audio_log_elem event_key, audio_log_elem p1)
{
    audio_log_firm_variadic(event_key, 1, p1);
}

void audio_log_firm_2(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2)
{
    audio_log_firm_variadic(event_key, 2, p1, p2);
}

void audio_log_firm_3(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2, audio_log_elem p3)
{
    audio_log_firm_variadic(event_key, 3, p1, p2, p3);
}

void audio_log_firm_4(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2, audio_log_elem p3, audio_log_elem p4)
{
    audio_log_firm_variadic(event_key, 4, p1, p2, p3, p4);
}

void audio_log_firm_5(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2, audio_log_elem p3, audio_log_elem p4,
        audio_log_elem p5)
{
    audio_log_firm_variadic(event_key, 5, p1, p2, p3, p4, p5);
}

/* Important note:
 * Downloadable builds use different logging to differentiate from the normal one.
 * The differences are:
 *
 * - First of all the debug messages are saved in different memory area 0x15500000 -
 *   0x15300000 (and not 0x13500000 - 0x13500000 see the linker script). This is
 *   necessary otherwise the messages will overlap between Kymera and the downloadable
 *   capability builds. In this manner, downloadable capability builds will still have o
 *   verlapping debug messages between them but not with the Kymera build.
 *
 * - To select which downloadable capability build should be used to read the debug
 *   message the return address of the debug function is saved as a parameter. That
 *   return address is used to select which downloaded capability was calling the
 *   logging. Knowing the capability the appropriate (downloadable capability) build is
 *   selected to read the debug message. */
void audio_log_firm_download_0(audio_log_elem event_key)
{
    unsigned return_addr = pl_get_return_addr();
    audio_log_firm_variadic(event_key, 1, return_addr);
}

void audio_log_firm_download_1(audio_log_elem event_key, audio_log_elem p1)
{
    unsigned return_addr = pl_get_return_addr();
    audio_log_firm_variadic(event_key, 2, p1, return_addr);
}

void audio_log_firm_download_2(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2)
{
    unsigned return_addr = pl_get_return_addr();
    audio_log_firm_variadic(event_key, 3, p1, p2, return_addr);
}

void audio_log_firm_download_3(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2, audio_log_elem p3)
{
    unsigned return_addr = pl_get_return_addr();
    audio_log_firm_variadic(event_key, 4, p1, p2, p3, return_addr);
}

void audio_log_firm_download_4(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2, audio_log_elem p3, audio_log_elem p4)
{
    unsigned return_addr = pl_get_return_addr();
    audio_log_firm_variadic(event_key, 5, p1, p2, p3, p4, return_addr);
}

void audio_log_firm_download_5(audio_log_elem event_key, audio_log_elem p1, \
        audio_log_elem p2, audio_log_elem p3, audio_log_elem p4,
        audio_log_elem p5)
{
    unsigned return_addr = pl_get_return_addr();
    audio_log_firm_variadic(event_key, 6, p1, p2, p3, p4, p5, return_addr);
}

void audio_log_firm_init(void)
{
#if defined (CHIP_BASE_HYDRA) && !defined(DESKTOP_TEST_BUILD) && !defined(UNIT_TEST_BUILD)
    audio_log_level_set(mibgetrequ16(DEFAULTLOGLEVEL));
#endif
}

#endif /* defined(AUDIO_LOG_FIRM) */
