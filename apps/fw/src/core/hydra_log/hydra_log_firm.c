/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra log (embedded firmware variant implementation).
 *
 * The log key (format string address) and arguments are written to a
 * circular buffer to be picked up via SPI and decoded by the debugger.
 * (E.g. log() in xap2emu.py).
 */

/*****************************************************************************
 * Implementation dependencies.
 ****************************************************************************/

#include "hydra_log/hydra_log.h"
#include "hydra/hydra_macros.h"
/*
 * Only compile this implementation variant if the header tells us it's an
 * embedded target
 */
#if defined(HYDRA_LOG_FIRM)

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

#include "hydra/hydra_trb.h"
#define IO_DEFS_MODULE_BUS_PROC_STREAM

#include "io/io.h"
#include "int/int.h"
/*****************************************************************************
 * Private Defines
 ****************************************************************************/

/**
 * Number of elements in circular log buffer.
 *
 * Must be a power of 2
 */
#ifndef HYDRA_LOG_BUF_SIZE
#define HYDRA_LOG_BUF_SIZE       512
#endif

#define HYDRA_LOG_BUF_MASK       (HYDRA_LOG_BUF_SIZE - 1)

/**
 * Number of characters in circular log character buffer.
 * Must be a power of 2.
 */
#ifndef HYDRA_LOG_CHAR_BUF_SIZE
#define HYDRA_LOG_CHAR_BUF_SIZE     4096
#endif

#define HYDRA_LOG_CHAR_BUF_MASK     (HYDRA_LOG_CHAR_BUF_SIZE - 1)

/**
 * Initial log level.
 */
#ifndef HYDRA_LOG_INITIAL_LEVEL
#define HYDRA_LOG_INITIAL_LEVEL HYDRA_LOG_NORMAL
#endif

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
/*lint -esym(551, debugBuffer) Not accessed, read via debugger only */
static hydra_log_elem debugBuffer[HYDRA_LOG_BUF_SIZE];

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
uint16 debugBufferSizeMask = HYDRA_LOG_BUF_MASK;

#ifdef FIRMWARE_PUTCHAR_LOG
char debugCharBuffer[HYDRA_LOG_CHAR_BUF_SIZE];
uint16 debugCharBufferPos;
uint16 debugCharStrLen;
#endif /* FIRMWARE_PUTCHAR_LOG */

struct {
    /** Set to enable the STREAM transactions to be sent for firmware debug
     * messages.
     */
    bool enable;
    /**
     * Tag value to use in the stream transactions. The debugger may
     * change this field for routing purposes
     */
    uint8 tag;
} hydra_log_trb_cfg = { FALSE,
        HYDRA_TRB_DEFAULT_STREAM_TAG};

enum {
    TRB_LOG_FLAG_SOP = 0x80,
    TRB_LOG_FLAG_EOP = 0x40
};


/*****************************************************************************
 * Public Data
 ****************************************************************************/

#if defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS)

HYDRA_LOG hydra_log =
{
    /* Init Per-module levels */
    {
        #define HYDRA_LOG_INIT(module) HYDRA_LOG_INITIAL_LEVEL,
        /*lint -e(123) : HYDRA_LOG_INIT defined with arguments  */
        HYDRA_LOG_FIRM_MODULES_X(HYDRA_LOG_INIT)
        #undef HYDRA_LOG_INIT
    }
};

#else /* !defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */

HYDRA_LOG hydra_log =
{
    /* Init global level */
    HYDRA_LOG_INITIAL_LEVEL
};

#endif /* !defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */

/*****************************************************************************
 * Private Functions
 ****************************************************************************/
static void hydra_log_trb(uint8 flags, uint32 dword1, uint32 dword2)
{
#ifdef CHIP_HAS_BUS_PROC_STREAM_MODULE
    block_interrupts();
    if(hal_get_reg_bus_proc_stream_config())
    {
        /* P0 or the debugger have set up the destination. */
        hal_set_reg_bus_proc_stream_payload1(dword1);
        hal_set_reg_bus_proc_stream_payload0(dword2);
        hal_set_reg_bus_proc_stream_final(flags |
                                (hydra_log_trb_cfg.tag << 8) |
                                (STREAM_SRC_BLOCK_ID_FIRMWARE_LOG_CPU_1 << 12));
    }
    unblock_interrupts();
#else
    UNUSED(flags);
    UNUSED(dword1);
    UNUSED(dword2);
    UNUSED(hydra_log_trb_cfg.tag);
#endif /* PROCESSOR_P1 && CHIP_HAS_BUS_PROC_STREAM_MODULE */
}

#ifdef FIRMWARE_PUTCHAR_LOG
static void hydra_log_trb_char(char c)
{
#ifdef CHIP_HAS_BUS_PROC_STREAM_MODULE
    block_interrupts();
    if(hal_get_reg_bus_proc_stream_config())
    {
        /* P0 or the debugger have set up the destination. */
        hal_set_reg_bus_proc_stream_payload1(0);
        hal_set_reg_bus_proc_stream_payload0(0);
        hal_set_reg_bus_proc_stream_final(c |
                                (hydra_log_trb_cfg.tag << 8) |
                                (STREAM_SRC_BLOCK_ID_PUT_CHAR << 12));
    }
    unblock_interrupts();
#else
    UNUSED(c);
#endif /* PROCESSOR_P1 && CHIP_HAS_BUS_PROC_STREAM_MODULE */
}
#endif /* FIRMWARE_PUTCHAR_LOG */

/**
 * Log a variadic debug event to circular buffer.
 *
 * Common implementation for hydra_log_firm_N functions.
 */
/*lint -e(960) -e(1916) Violates MISRA Rule 16.1, but debug/log only */
void hydra_log_firm_variadic
(
    /**
     * Event key.
     *
     * This is formed by casting the address of the format string
     * (declared in unlinked segment using DECLARE_DEBUG_STRING)
     * to a hydra_log_elem later.
     */
    const char *event_key,

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
     * All args must be of hydra_log_elem type.
     */
    ...
)
{
    va_list va_argp;
    size_t pos;

    va_start(va_argp, n_args);

    if (hydra_log_trb_cfg.enable)
    {
        hydra_log_elem trb_args[2];
        va_list va_argp_trb;
        size_t n_args_trb = n_args;

        /* First time round always send event_key */
        uint8 trb_flags = TRB_LOG_FLAG_SOP;
        int arg_i = 1;
        uint8 trb_size = 4;
        trb_args[0] = (hydra_log_elem)event_key;

        /* Get copy of va_list */
        va_copy(va_argp_trb, va_argp);

        /* Send arguments over TRB */
        do
        {
            /* Read up to 2 args each time round */
            while (arg_i < 2)
            {
                /* Get argument if any left */
                if (n_args_trb)
                {
                    trb_args[arg_i] = va_arg(va_argp_trb, hydra_log_elem);
                    trb_size += 4;
                    n_args_trb--;
                }
                else
                    trb_args[arg_i] = 0;

                arg_i++;
            }

            /* Set EOP flag if no more arguments */
            if (n_args_trb == 0)
                trb_flags |= TRB_LOG_FLAG_EOP;

            /* Send 64 bits over TRB */
            hydra_log_trb((uint8)(trb_flags | trb_size), trb_args[0], trb_args[1]);

            /* Clear SOP for subsequent transfers */
            trb_flags &= ~TRB_LOG_FLAG_SOP;

            /* Reset index and size */
            arg_i = 0;
            trb_size = 0;
        } while (n_args_trb);
    }

    block_interrupts();

#ifndef __KALIMBA__
    /* Create a constant in the symbol table for the buffer size.
     *
     * Note that this doesn't produce any executable code in this
     * function.
     *
     * \todo Investigate having the log script get the buffer's size direct
     * from the elf to avoid the need for this (Debuggers manage it) .
     */
    __asm__(".set debugBufferSize,    " CSR_EXPAND_AND_STRINGIFY(HYDRA_LOG_BUF_SIZE));
#endif
    pos = debugBufferPos;

    /* Push the event key (aka format string's address).
     */
    debugBuffer[pos] = (hydra_log_elem) event_key;
    ++pos;
    pos &= debugBufferSizeMask;

    /* Push any and all args to log.
     *
     * FWIW: This loop was 14 XAP instructions long when I last looked.
     * So it should fit in XAP cache.
     */

    while(n_args--)
    {
        debugBuffer[pos] = va_arg(va_argp, hydra_log_elem);
        ++pos;
        pos &= debugBufferSizeMask;
    }
    /*lint -e(10) Stronger magic */va_end(va_argp);

    debugBufferPos = pos;

    unblock_interrupts();        
}


void set_p1_firmware_log_trb_parameters(void)
{
}


/*****************************************************************************
 * Public Functions
 *
 * One function call for each number of params to avoid passing nr params
 * from every call site (code-space optimisation).
 ****************************************************************************/

void hydra_log_firm_0(const char *event_key)
{
    hydra_log_firm_variadic(event_key, 0);
}

void hydra_log_firm_1(const char *event_key, hydra_log_elem p1)
{
    hydra_log_firm_variadic(event_key, 1, p1);
}

void hydra_log_firm_2(const char *event_key, hydra_log_elem p1, \
        hydra_log_elem p2)
{
    hydra_log_firm_variadic(event_key, 2, p1, p2);
}

void hydra_log_firm_3(const char *event_key, hydra_log_elem p1, \
        hydra_log_elem p2, hydra_log_elem p3)
{
    hydra_log_firm_variadic(event_key, 3, p1, p2, p3);
}

void hydra_log_firm_4(const char *event_key, hydra_log_elem p1, \
        hydra_log_elem p2, hydra_log_elem p3, hydra_log_elem p4)
{
    hydra_log_firm_variadic(event_key, 4, p1, p2, p3, p4);
}

#ifdef FIRMWARE_PUTCHAR_LOG
void hydra_log_firm_putchar(char c)
{
    debugCharBuffer[debugCharBufferPos] = c;
    UNUSED(debugCharBuffer[debugCharBufferPos]); /* to keep lint quiet */
    ++debugCharStrLen;
    if(c == '\n')
    {
        HYDRA_LOG_STRING(log_fmt, "putchar string 0x%x");
        hydra_log_firm_1(log_fmt,
                        ((uint32)debugCharStrLen << 16) | debugCharBufferPos);
        debugCharStrLen = 0;
    }
    debugCharBufferPos = (debugCharBufferPos +1) & HYDRA_LOG_CHAR_BUF_MASK;

    if(hydra_log_trb_cfg.enable)
    {
        hydra_log_trb_char(c);
    }
}
#endif /* FIRMWARE_PUTCHAR_LOG */

void hydra_log_enable_trb_logging(bool enable)
{
    hydra_log_trb_cfg.enable = enable;
}

#endif /* defined(HYDRA_LOG_FIRM) */
