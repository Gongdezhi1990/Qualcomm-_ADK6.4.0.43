/*
Copyright (c) 2018  Qualcomm Technologies International, Ltd.
*/
#include <csrtypes.h>
#include <vmtypes.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <pio.h>

#include "input_event_manager.h"
#include "input_event_manager_private.h"

static InputEventState_t input_event_manager_state = { {0} };

static void enterAction(InputEventState_t *state,
                        const InputActionMessage_t *input_action,
                        input_event_bits_t input_event_bits)
{
    /* If all the bits, for the msg, are 'on', and at least one of those bits,
     * was just turned on, then ...
     */
    if (input_action->bits == (input_event_bits & input_action->mask))
    {
        /* A new enter action cancels any existing repeat timer */
        (void) MessageCancelAll(&state->task,
                                IEM_INTERNAL_REPEAT_TIMER);
        MessageSend(state->client, input_action->message, 0);

        /* if there is a repeat on this action, start the repeat timer */
        if (input_action->repeat)
        {
            state->repeat = input_action;
            MessageSendLater(&state->task, IEM_INTERNAL_REPEAT_TIMER,
                             0, input_action->repeat);
        }
        else
            state->repeat = 0;

    }
    /* if any of the bits are turned off and there is a repeat timer,
     * cancel it and clear the stored input_action
     */
    else if (input_action->repeat &&
             state->repeat == input_action &&
             input_action->bits == (state->input_event_bits & input_action->mask) &&
             input_action->bits != (input_event_bits & input_action->mask))
    {
        (void) MessageCancelAll(&state->task, IEM_INTERNAL_REPEAT_TIMER);
        state->repeat = 0;
    }
}

/* There can be 1+ held action/messages on the same PIO */
static void heldAction(InputEventState_t *state,
                       const InputActionMessage_t *input_action,
                       input_event_bits_t input_event_bits)
{
    /* If all the PIO, for the msg, are 'on'... */
    if (input_action->bits == (input_event_bits & input_action->mask))
    {
        /* Send a pointer to this input_action as part of the timer message so that it
         * can be handled when the timeout expired
         */
        const InputActionMessage_t **m = PanicUnlessNew(const InputActionMessage_t *);
        *m = input_action;

        MessageSendLater(&state->task, IEM_INTERNAL_HELD_TIMER,
                         m, input_action->timeout);
    }
    /* If any of the bits are turned off...
     */
    else if (input_action->bits == (state->input_event_bits & input_action->mask) &&
             input_action->bits != (input_event_bits & input_action->mask))
    {
        /* Cancel any active held or repeat timers. */
        if (!MessageCancelAll(&state->task, IEM_INTERNAL_HELD_TIMER))
            (void)MessageCancelAll(&state->task, IEM_INTERNAL_REPEAT_TIMER);
    }
}

static void heldReleaseAction(InputEventState_t *state,
                              const InputActionMessage_t *input_action,
                              input_event_bits_t input_event_bits)
{
    /* If all the bits, for the msg, are 'on' then ...
     */
    if (input_action->bits == (input_event_bits & input_action->mask))
    {
        const InputActionMessage_t **m = PanicUnlessNew(const InputActionMessage_t *);
        *m = input_action;

        MessageSendLater(&state->task,
                    IEM_INTERNAL_HELD_RELEASE_TIMER,
                    (void*)m,
                    input_action->timeout
                    );

        state->held_release = 0;
    }
    /* Otherwise, if the PIO were on but now changed to off...
     */
    else if (input_action->bits == (state->input_event_bits & input_action->mask) &&
             input_action->bits != (input_event_bits & input_action->mask))
    {
        if (!state->held_release)
            /* If no action message was registered for held_release yet,
            * it means the pio was released before the held_release timer
            * was expired.*/
        {
            /* Cancel any active held_release timers. */
            (void) MessageCancelAll(&state->task,
                                    IEM_INTERNAL_HELD_RELEASE_TIMER);
        }
        else if (state->held_release == input_action)
            /* If an action message was registered, it means that the
            * held_release timer has expired and hence send the
            * message */
        {
            state->held_release = 0;
            MessageSend(state->client, input_action->message, 0);

            /* Now that held_release message is issued, supress
            * future release messages */
            state->input_event_release_disabled |= input_action->bits;
        }
    }
}

static void doubleAction(InputEventState_t *state,
                         const InputActionMessage_t *input_action,
                         input_event_bits_t input_event_bits)
{
    /* if the Pio has changed to On... */
    if (input_action->bits == (input_event_bits & input_action->mask))
    {
        /* If this input_action is the same as the stored input_action then this must be
         * the second button tap.
         */
        if (state->double_tap == input_action)
        {
            /* if the double tap timer is running then send the message
             * (the condition check will cancel the timer).
             */
            if (MessageCancelAll(&state->task, IEM_INTERNAL_DOUBLE_TIMER))
                MessageSend(state->client, input_action->message, 0);

            /* Regardless of if the timer was running, clear the stored input_action */
            state->double_tap = 0;
        }
        /* Otherwise, this must be the first tap so store the input_action and start
         * the Double Tap timer.
         */
        else
        {
            state->double_tap = input_action;
            MessageSendLater(&state->task, IEM_INTERNAL_DOUBLE_TIMER,
                             NULL, input_action->timeout);
        }
    }
}

static void inputEventsChanged(InputEventState_t *state, input_event_bits_t input_event_bits)
{
    input_event_bits_t changed_bits = state->input_event_bits ^ input_event_bits;
    const InputActionMessage_t *input_action = state->action_table;
    const uint8 size = state->num_action_messages;

    IEM_DEBUG(("IEM: Updated input events %08x\n", input_event_bits));

    /* Go through the action table to determine what action to do and
       what message may need to be sent. */
    for (;input_action != &(state->action_table[size]); input_action++)
    {
        if (changed_bits & input_action->mask)
        {
            switch (input_action->action)
            {
                case ENTER:
                    enterAction(state, input_action, input_event_bits);
                    break;

                /* Only a release if the PIO were previously on and now
                    have been turned off, and a release message was not
                    to be supressed */
                case RELEASE:
                {
                    bool prev_bit_state_is_high = (input_action->bits == (state->input_event_bits & input_action->mask));
                    bool bit_state_is_low = (input_action->bits != (input_event_bits & input_action->mask));
                    bool bit_release_msg_is_active = (input_action->bits != (state->input_event_release_disabled & input_action->mask ));
                    if (prev_bit_state_is_high && bit_state_is_low && bit_release_msg_is_active)
                        MessageSend(state->client, input_action->message, 0);

                    /* Re-enable Release messages for the next release action */
                    state->input_event_release_disabled &= ~(input_action->bits);
                }
                break;

                case HELD:
                    heldAction(state, input_action, input_event_bits);
                    break;

                case HELD_RELEASE:
                    heldReleaseAction(state, input_action, input_event_bits);
                    break;

                case DOUBLE:
                    doubleAction(state, input_action, input_event_bits);
                    break;

                default:
                    break;
            }
        }
    }

    /* Store the bits previously reported */
    state->input_event_bits = input_event_bits;
}

static uint32 calculateInputEvents(InputEventState_t *state)
{
    int pio, bank;
    uint32 input_event_bits = 0;
    for (bank = 0; bank < IEM_NUM_BANKS; bank++)
    {
        const uint8 pio_base = bank * 32;
        const uint32 pio_state = state->pio_state[bank];
        for (pio = 0; pio < 32; pio++)
        {
            const uint32 pio_mask = 1UL << pio;
            if (pio_state & pio_mask)
                input_event_bits |= (1UL << state->config->pio_mapping[pio_base + pio]);
        }
    }
    return input_event_bits;
}

/*! Generate PIO events */
static void handleMessagePioChangedEvents(InputEventState_t *state, const MessagePioChanged *mpc)
{
    /* Mask out PIOs we're not interested in */
    const uint32 pio_state = (mpc->state) + ((uint32)mpc->state16to31 << 16);
    const uint32 pio_state_masked = pio_state & state->config->pio_config[mpc->bank];

    if (state->pio_state[mpc->bank] != pio_state_masked)
    {
        /* Update our copy of the PIO state */
        state->pio_state[mpc->bank] = pio_state_masked;

        /* Calculate input events from PIO state and handle them */
        inputEventsChanged(state, calculateInputEvents(state));
    }
}

/*! Check if any task registered is interested in this PIO change message */
static void handleMessagePioChangedClients(InputEventState_t *state, const MessagePioChanged *mpc)
{
    InputEventClient_t *client = NULL;
    const uint32 pio_state = (mpc->state) + ((uint32)mpc->state16to31 << 16);

    for (client = state->client_list; client != NULL; client = client->next)
    {
        const int client_bank = client->pio / 32;
        if (client_bank == mpc->bank)
        {
            const int pio_in_bank = client->pio - (32 * client_bank);
            const uint32 pio_mask = 1UL << pio_in_bank;

            /* Check if latest PIO state doesn't match the stored state */
            if (((pio_state & pio_mask) >> pio_in_bank) != client->state)
            {
                /* Update stored state */
                client->state ^= 1;

                IEM_DEBUG(("IEM: Sending MESSAGE_PIO_CHANGED to task %p for PIO %u\n", client->task, client->pio));

                /* Send MESSAGE_PIO_CHANGED to task */
                void *mpc_copy = PanicUnlessNew(MessagePioChanged);
                memcpy(mpc_copy, mpc, sizeof(MessagePioChanged));
                MessageSend(client->task, MESSAGE_PIO_CHANGED, mpc_copy);
            }
        }
    }
}

static void pioHandler(Task task, MessageId id, Message message)
{
    InputEventState_t *state = (InputEventState_t *)task;

    switch (id)
    {
        case MESSAGE_PIO_CHANGED:
        {
            const MessagePioChanged *mpc = (const MessagePioChanged *)message;
            IEM_DEBUG(("IEM: PIO state %04x%04x for bank %u\n", mpc->state16to31, mpc->state, mpc->bank));
            handleMessagePioChangedEvents(state, mpc);
            handleMessagePioChangedClients(state, mpc);
        }
        break;

        /* If a pio has been HELD for the timeout required, then send the message stored */
        case IEM_INTERNAL_HELD_TIMER:
        {
            const InputActionMessage_t **m = (const InputActionMessage_t **)message;
            const InputActionMessage_t *input_action = *m;

            MessageSend(state->client, input_action->message, NULL);

            /* Cancel any existing repeat timer that may be running */
            (void)MessageCancelAll(&state->task, IEM_INTERNAL_REPEAT_TIMER);

            /* Now that a held message has been issued, suppress future
               release messages */
            state->input_event_release_disabled |= input_action->bits;

            /* If there is a repeat action start the repeat on this message
               and store the input_action */
            if (input_action->repeat)
            {
                MessageSendLater(&state->task,
                                 IEM_INTERNAL_REPEAT_TIMER, 0,
                                 input_action->repeat);

                state->repeat = input_action;
            }
        }
        break;

        case IEM_INTERNAL_REPEAT_TIMER:
        {
            if (state->repeat)
            {
                MessageSend(state->client, (state->repeat)->message, NULL);

                /* Start the repeat timer again */
                MessageSendLater(&state->task, IEM_INTERNAL_REPEAT_TIMER,
                                 NULL, (state->repeat)->repeat);
            }
        }
        break;

        /* Store the input_action so that when the PIO for the message are released
           it can be validated and the message sent */
        case IEM_INTERNAL_HELD_RELEASE_TIMER:
        {
            const InputActionMessage_t **m = (const InputActionMessage_t **)message;
            state->held_release = *m;
        }
        break;

        case IEM_INTERNAL_DOUBLE_TIMER:
            /* Clear the stored input_action */
            state->double_tap = 0;
            break;

        default:
            break;
    }
}

static void configurePioHardware(Task task, const InputEventConfig_t *config)
{
    uint8 bank;

    /* Configure PIOs:
       1.  Map as PIOs
       2.  Allow deep sleep on either level
       3.  Set as inputs
       4.  Enable debouncing */
    for (bank = 0; bank < IEM_NUM_BANKS; bank++)
    {
        IEM_DEBUG(("IEM: Configuring bank %d, mask %08x\n", bank, config->pio_config[bank]));

        if (PioSetMapPins32Bank(bank, config->pio_config[bank], config->pio_config[bank]))
            Panic();
        PioSetDeepSleepEitherLevelBank(bank, config->pio_config[bank], config->pio_config[bank]);
        PioSetDir32Bank(bank, config->pio_config[bank], 0);
        PioDebounce32Bank(bank, config->pio_config[bank], config->debounce_reads, config->debounce_period);  
    }

    (void)MessagePioTask(task);
}

static void setStartupStates(InputEventState_t *state)
{
    uint8 bank;
    UNUSED(state);

    /* Send initial PIO messages */
    for (bank = 0; bank < IEM_NUM_BANKS; bank++)
    {
        MessagePioChanged message;
        uint32 pio_state = PioGet32Bank(bank);

        message.state       = (pio_state >>  0) & 0xFFFF;
        message.state16to31 = (pio_state >> 16) & 0xFFFF;
        message.time = 0;
        message.bank = bank;

        handleMessagePioChangedEvents(state, &message);
    }
}

/*! Add a client to the list of clients */
static bool addClient(Task task, uint8 pio)
{
    InputEventClient_t *client = calloc(1, sizeof(*client));
    if (client)
    {
        const uint32 bank = pio / 32;
        const uint32 bank_pio = pio % 32;
        const uint32 bank_pio_mask = 1UL << bank_pio;
        const uint32 bank_state = PioGet32Bank(bank);

        client->pio = pio;
        client->task = task;
        client->next = input_event_manager_state.client_list;
        client->state = (bank_state & bank_pio_mask) ? 1 : 0;
        input_event_manager_state.client_list = client;


        IEM_DEBUG(("IEM: addClient pio %d, state %d\n", pio, client->state));

        return TRUE;
    }

    return FALSE;
}

/*! Remove a client from the list of clients */
static void removeClient(Task task, uint8 pio)
{
    InputEventClient_t **head;
    for (head = &input_event_manager_state.client_list; *head != NULL; head = &(*head)->next)
    {
        if (((*head)->task == task) && ((*head)->pio == pio))
        {
            InputEventClient_t *to_remove = *head;
            *head = (*head)->next;
            free(to_remove);
            IEM_DEBUG(("IEM: removeClient pio %d\n", pio));
            break;
        }
    }
}

/*! Given a PIO (which has been added/removed from interest) update the debounced PIOs */
static void updateDebounced(const InputEventConfig_t *config, uint8 pio)
{
    InputEventClient_t *client;
    const uint32 pio_bank = pio / 32;
    uint32 pio_bank_mask = 0;

    for (client = input_event_manager_state.client_list;
         client != NULL;
         client = client->next)
    {
        const uint32 client_bank = client->pio / 32;
        if (client_bank == pio_bank)
        {
            const uint32 client_bank_pio = client->pio - (32 * client_bank);
            const uint32 client_bank_pio_mask = 1UL << client_bank_pio;
            pio_bank_mask |= client_bank_pio_mask;
        }
    }
    IEM_DEBUG(("IEM: updateDebounced pio_bank %d, mask %08x\n", pio_bank, pio_bank_mask));
    PioDebounce32Bank(pio_bank, pio_bank_mask | config->pio_config[pio_bank],
                      config->debounce_reads, config->debounce_period);
}


Task InputEventManagerInit(Task client,
                           const InputActionMessage_t *action_table, uint32 size_action_table,
                           const InputEventConfig_t *config)
{
    memset(&input_event_manager_state, 0, sizeof(InputEventState_t));

    input_event_manager_state.task.handler = pioHandler;
    input_event_manager_state.client = client;
    input_event_manager_state.action_table = action_table;
    input_event_manager_state.num_action_messages = size_action_table / sizeof(InputActionMessage_t);
    input_event_manager_state.config = config;

    configurePioHardware(&input_event_manager_state.task, input_event_manager_state.config);
    setStartupStates(&input_event_manager_state);

    return &input_event_manager_state.task;
}

void InputEventManagerRegisterTask(Task client, uint8 pio)
{
    addClient(client, pio);
    updateDebounced(input_event_manager_state.config, pio);
}

void InputEventManagerUnregisterTask(Task client, uint8 pio)
{
    removeClient(client, pio);
    updateDebounced(input_event_manager_state.config, pio);
}
