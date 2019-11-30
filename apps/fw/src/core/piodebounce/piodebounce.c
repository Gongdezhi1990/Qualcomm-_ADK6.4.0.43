/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Debounce input bits on the pio port.
 */

#include "piodebounce/piodebounce_private.h"


/** Stores the piodebounce database. */
static piodebounce_db piodb[PIODEBOUNCE_NUMBER_OF_GROUPS];

/** Stores a mask keeping track which goups raised the background interrupt */
static uint32 pio_debounce_bg_group_mask = 0;
/** Stores a mask keeping track which goups raised the timer scheduling
    background interrupt */
static uint32 pio_debounce_bg_sched_group_mask = 0;

/* Forward references. */
static void debounce_it_handler(uint16 fniarg, void *fnvarg);
static void sched_debounce(uint16 group);
static void update_output(uint16 group, uint16 bank);
static void register_kick(uint16 group);
static void piodebounce_kick_0(void);
static void piodebounce_kick_1(void);

void init_piodebounce(void)
{
    uint16 i, j;

    for(i = 0; i < PIODEBOUNCE_NUMBER_OF_GROUPS; i++)
    {
        for(j = 0; j < NUMBER_OF_PIO_BANKS; j++)
        {
            piodb[i].mask[j] = 0x00000000;
            piodb[i].output[j] = 0x00000000;
        }
        piodb[i].bank_mask = 0x0000;
        piodb[i].itid = NO_TID;
        register_kick(i);
    }
}

void piodebounce_config(uint16 group,
                        uint16 bank,
                        pio_size_bits mask,
                        uint16 nreads,
                        uint16 period,
                        piodebounce_cb callback)
{
    assert(group < PIODEBOUNCE_NUMBER_OF_GROUPS);

    L4_DBG_MSG4("Piodebounce: Registering callback 0x%08x on bank %d "
                "mask 0x%08x for group %d", callback, bank, mask, group);

    piodb[group].mask[bank] = mask;
    piodb[group].period = period;
    piodb[group].nreads = nreads;
    piodb[group].callback = callback;

    /* Set an initial output value. If piodb[group].mask is zero this will
    return zero. */
    piodb[group].output[bank] = pio_get_levels_all(bank) & mask;

    register_kick(group);
}
static void piodebounce_kick_0(void)
{
    piodebounce_kick(0);
}

static void piodebounce_kick_1(void)
{
    piodebounce_kick(1);
}

void piodebounce_kick(uint16 group)
{
    uint16 i;

    assert(group < PIODEBOUNCE_NUMBER_OF_GROUPS);

    L4_DBG_MSG1("Piodebounce: Received kick for group %d", group); 
    for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
    {
        piodb[group].tmp[i] = pio_get_levels_mask(i, piodb[group].mask[i]);
    }

    switch (piodb[group].nreads)
    {
        case 0:
            /* Special case 1: if piodb[group].nreads is 0 then always accept
               the input pins current value and always raise an event. */
            update_output(group, 0);
            break;
        case 1:
            /* Special case 2: if piodb[group].nreads is 1 then always accept
               the input pins current value and raise an event if it differs
               from the last debounced value. */
            for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
            {
                if(piodb[group].output[i] != piodb[group].tmp[i])
                {
                    update_output(group, i);
                    break;
                }
            }
            break;
        default:
            /* The normal case: start a sequence of reads. */
            piodb[group].nleft = (uint16)(piodb[group].nreads - 1);
            sched_debounce(group);
            break;
    }
}

pio_size_bits piodebounce_get(uint16 group, uint16 bank)
{
    assert(group < PIODEBOUNCE_NUMBER_OF_GROUPS);

    return piodb[group].output[bank];
}

static void sched_debounce(uint16 group)
{
    ATOMIC_BLOCK_START {
        pio_debounce_bg_sched_group_mask |= 1 << group;
    } ATOMIC_BLOCK_END;
    GEN_BG_INT(piodebounce_sched_debounce);
}

void piodebounce_sched_debounce_bg_int_handler(void)
{
    uint32 group_mask;
    uint16 i;

    ATOMIC_BLOCK_START {
        group_mask = pio_debounce_bg_sched_group_mask;
        pio_debounce_bg_sched_group_mask = 0;
    } ATOMIC_BLOCK_END;

    for (i = 0; i < PIODEBOUNCE_NUMBER_OF_GROUPS; i++)
    {
        if (group_mask & (1 << i))
        {
            /* Cancel any pending scheduled call of debounce(). */
            (void)cancel_timed_event(piodb[i].itid, NULL, NULL);

            /* Call debounce_it_handler() after piodb[i].period milliseconds at
               int level 1. */
            piodb[i].itid = timed_event_in(piodb[i].period * MILLISECOND,
                                           debounce_it_handler,
                                           0, (void *)(uint32)i);
        }
    }
}

static void debounce_it_handler(uint16 fniarg, void *fnvarg)
{
    uint32 current[NUMBER_OF_PIO_BANKS];
    uint16 i;
    uint16 group = (uint16)(uint32)fnvarg;

    UNUSED(fniarg);

    L4_DBG_MSG1("Piodebounce: Polling PIOs in group %d", group);
    for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
    {
        current[i] = pio_get_levels_mask(i, piodb[group].mask[i]);
    }

    /* Restart the sequence of reads if the current port value does not match
       the candidate new debounce value. */
    for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
    {
        if(piodb[group].tmp[i] != current[i])
        {
            L4_DBG_MSG1("Piodebounce: PIOs in group %d changed, kicking",
                        group); 
            piodebounce_kick(group);
            return;
        }
    }

    if(piodb[group].nleft)
    {
        /* Go round again if we've not read the pins enough times. */
        --piodb[group].nleft;
        sched_debounce(group);
    }
    else
    {
        /* Accept candidate value as stable and signal the change. */
        L4_DBG_MSG1("Piodebounce: PIOs in group %d are stable", group); 
        for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
        {
            if(piodb[group].output[i] != piodb[group].tmp[i])
            {
                update_output(group, i);
            }
        }
    }
}

static void register_kick(uint16 group)
{
    uint16 i;

    for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
    {
        pioint_configure(i, piodb[group].mask[i], group?piodebounce_kick_1:piodebounce_kick_0);
    }
}

static void update_output(uint16 group, uint16 bank)
{
    ATOMIC_BLOCK_START {
        piodb[group].bank_mask |= 1 << bank;
        piodb[group].output[bank] = piodb[group].tmp[bank];
        pio_debounce_bg_group_mask |= 1 << group;
    } ATOMIC_BLOCK_END;
    /* Drop to background and call the handler. */
    GEN_BG_INT(piodebounce);
}   

void piodebounce_bg_int_handler(void)
{
    uint32 group_mask, bank_masks[PIODEBOUNCE_NUMBER_OF_GROUPS] = {0, 0};
    uint16 bank;
    uint16 i;

    ATOMIC_BLOCK_START {
        group_mask = pio_debounce_bg_group_mask;
        pio_debounce_bg_group_mask = 0;
        for (i = 0; i < PIODEBOUNCE_NUMBER_OF_GROUPS; i++)
        {
            if (group_mask & (1 << i))
            {
                bank_masks[i] = piodb[i].bank_mask;
                piodb[i].bank_mask = 0;
            }
        }
    } ATOMIC_BLOCK_END;

    for (i = 0; i < PIODEBOUNCE_NUMBER_OF_GROUPS; i++)
    {
        if (group_mask & (1 << i))
        {
            bank = 0;
            while (bank_masks[i])
            {
                if (bank_masks[i] & 0x01)
                {
                    L4_DBG_MSG2("Piodebounce: PIOs in group %d, bank %d "
                                "changed, calling callback", i, bank);
                    if (piodb[i].callback != NULL)
                    {
                        piodb[i].callback(i, bank);
                    }
                }
                bank_masks[i] >>= 1;
                bank++;
            }
        }
    }
}

