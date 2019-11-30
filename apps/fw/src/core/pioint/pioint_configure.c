/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Manage interrupts on PIOs.
 */

#include "pioint/pioint_private.h"


/** Pointer to the start of the database. */
static pioint_config_entry *head;

/**
 * Handler that crt calls for PIO interrupt event.
 */
static void pioint_handler(void)
{
    pioint_config_entry *cur;
    pio_size_bits raised[NUMBER_OF_PIO_BANKS];
    uint32 i;

    /* Record the events which triggered the interrupt. */
    block_interrupts();
    for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
    {
       raised[i] = pio_get_event_cause(i);
       pio_clear_event_cause(i, raised[i]);
    }
    unblock_interrupts();

    /* Now loop over all registered entries. We would only call the handlers
       for ones interested in the PIOs that have changed. */
    for(cur = head; cur != NULL; )
    {
        pioint_config_entry *tmp = cur->next;
        if(cur->mask & raised[cur->bank])
        {
            cur->handler();
        }
        cur = tmp;
    }
}

void pioint_configure(uint16 bank, pio_size_bits mask, void (*handler)(void))
{
    pioint_config_entry *tmp, *entry, **prev_next;
    bool updated = FALSE, en_flag = FALSE;
    pio_size_bits master_mask[NUMBER_OF_PIO_BANKS];
    uint32 i;
    static bool enabled = FALSE;

    assert(bank < NUMBER_OF_PIO_BANKS);

    if(handler != NULL)
    {
        memset(master_mask, 0U, sizeof(master_mask));
        /* Loop through the database and search for an existing handler-bank
           pair. */
        block_interrupts();
        for(prev_next = &head, entry = head; entry != NULL; )
        {
           if((entry->handler == handler) && (entry->bank == bank))
           {
               /* Matching handler-bank pair found */
               if(!mask)
               {
                   /* A mask with only 0s means we want to deregister.
                      Remove the entry from the list. */
                   *prev_next = entry->next;
               }
               /* Update the mask. This will be 0 for deregister calls. */
               entry->mask = mask;
               /* Signal update completed */
               updated = TRUE;
           }
           /* Record the enabled interrupt sources */
           master_mask[entry->bank] |= entry->mask;
           if (!entry->mask)
           {
               /* This is only executed for deregister calls. entry->mask is
                  never 0 unless this is a deregister call. */
               tmp = entry->next;
               /* Free the removed entry */
               pfree(entry);
               /* prev_next does not need updating. It already points to the
                  next entry. */
               entry = tmp;
           }
           else
           {
               prev_next = &(entry->next);
               entry = entry->next;
           }
        }
        if((!updated) && mask)
        {
            /* This is a new handler/bank pair, add it in front of the list */
            entry = pnew(pioint_config_entry);
            entry->bank = bank;
            entry->mask = mask;
            entry->handler = handler;
            entry->next = head;
            head = entry;
            /* Record the enabled interrupt sources */
            master_mask[entry->bank] |= entry->mask;
        }
        for(i = 0; i < NUMBER_OF_PIO_BANKS; i++)
        {
            uint32 prev_triggers;

            prev_triggers = pio_get_rising_int_triggers(i);
            /* Make sure triggers to be enabled are not already raised */
            pio_clear_event_cause(i, (~prev_triggers) & master_mask[i]);
            /* Update rising triggers */
            pio_set_rising_int_triggers(i, master_mask[i]);
            /* Make sure disabled triggers are not left raised */
            pio_clear_event_cause(i, prev_triggers & (~master_mask[i]));

            prev_triggers = pio_get_falling_int_triggers(i);
            /* Make sure triggers to be enabled are not already raised */
            pio_clear_event_cause(i, (~prev_triggers) & master_mask[i]);
            /* Update falling triggers */
            pio_set_falling_int_triggers(i, master_mask[i]);
            /* Make sure disabled triggers are not left raised */
            pio_clear_event_cause(i, prev_triggers & (~master_mask[i]));

            /* If any bank has at least one enabled trigger then we set the
               flag to indicate that we need to register for a PIO interrupt */
            en_flag = master_mask[i]?TRUE:en_flag;
        }
        if((int)enabled != (int)en_flag)
        {
            /* Enable interrupts if interrupts are disabled and the flag wants
               them enabled. Similarly, disable interrupts if the interrupts
               are enabled and the flag wants them disabled. */
            enabled = en_flag;
            configure_interrupt(PIOINT_SOURCE, PIOINT_PRIORITY, en_flag?pioint_handler:NULL);
        }
        unblock_interrupts();
    }
}

