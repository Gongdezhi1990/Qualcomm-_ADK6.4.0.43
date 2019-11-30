/*
Copyright (c) 2018  Qualcomm Technologies International, Ltd.
*/

#ifndef INPUT_EVENT_MANAGER_H_
#define INPUT_EVENT_MANAGER_H_

#include <message.h>

#define IEM_NUM_PIOS  (96)
#define IEM_NUM_BANKS (IEM_NUM_PIOS / 32)

typedef uint32 input_event_bits_t;

typedef enum
{
    ENTER,
    RELEASE,
    HELD,
    HELD_RELEASE,
    DOUBLE
} InputEventAction_t;

typedef struct
{
    input_event_bits_t    bits;
    input_event_bits_t    mask;
    InputEventAction_t    action;
    uint16              timeout;
    uint16              repeat;  /* Only used for HELD and ENTER actions */
    MessageId           message;
} InputActionMessage_t;

typedef struct
{
    int8 pio_mapping[IEM_NUM_PIOS];
    uint32 pio_config[IEM_NUM_BANKS];
    uint8 debounce_reads;
    uint16 debounce_period;
} InputEventConfig_t;

/*! @brief Initialise the input event manager. */
Task InputEventManagerInit(Task client,
                           const InputActionMessage_t *action_table,
                           uint32 size_action_table,
                           const InputEventConfig_t *config);

/*! @brief Register a task to receive event notifications on the specified pio.
           When the specified pio changes state, the input event manager will
           send the client task a #MESSAGE_PIO_CHANGED.

    It is the caller's responsibility to configure the pio (e.g. as input,
    setting pull-up/pull-down resistors). The library will maintain the complete
    list of pios requiring debouncing by the firmware, and will call
    PioDebounce32Bank().
*/
void InputEventManagerRegisterTask(Task client, uint8 pio);

/*! @brief Unregister a task. */
void InputEventManagerUnregisterTask(Task client, uint8 pio);


#endif /* INPUT_EVENT_MANAGER_H_ */
