/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2018

@file

    Functions that provide access to the buttons on the
    development board.

    The support is hard coded to support SW1 to SW8, calling the
    function supplied by the application for each button that is
    pressed.
    - Multiple button presses are ignored.
    - There is no facility for handling types of press such as
    long, double
*/

#include "dev_board_buttons.h"
#include <message.h>
#include <panic.h>

#define NUM_SWITCHES    (8+1)

#if defined(H13672v2)
    #define HAVE_BUTTONS      1
    /* The dev board has a mapping of SW1=74...SW8=PIO81 */
    #define SWITCH_BANK       2
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(74), SW_PIO_MASK(75), SW_PIO_MASK(76), SW_PIO_MASK(77),
        SW_PIO_MASK(78), SW_PIO_MASK(79), SW_PIO_MASK(80), SW_PIO_MASK(81)
    };
#elif defined(CE446)
    #define HAVE_BUTTONS      1
    #define SWITCH_BANK       2
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(79), SW_PIO_MASK(78), SW_PIO_MASK(84), SW_PIO_MASK(85),
        SW_PIO_MASK(81), SW_PIO_MASK(80), SW_PIO_MASK(76), SW_PIO_MASK(77)
    };
#elif defined(CF376_CE692H3)
    #define HAVE_BUTTONS      1
    #define SWITCH_BANK       2
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(79), SW_PIO_MASK(78), SW_PIO_MASK(84), SW_PIO_MASK(85),
        SW_PIO_MASK(76), SW_PIO_MASK(77), SW_PIO_MASK(81), SW_PIO_MASK(80)
    };
#elif defined(CF376_CE826)
    #define HAVE_BUTTONS      1
    #define SWITCH_BANK       0
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(15), SW_PIO_MASK(21), SW_PIO_MASK(23), SW_PIO_MASK(29),
        SW_PIO_MASK(30), SW_PIO_MASK(31),               0,               0
    };
#elif defined(CF376_CF212)
    #define HAVE_BUTTONS      1
    #define SWITCH_BANK       1
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(34), SW_PIO_MASK(35), SW_PIO_MASK(36), SW_PIO_MASK(37),
        SW_PIO_MASK(38), SW_PIO_MASK(39), SW_PIO_MASK(40), SW_PIO_MASK(41)
    };
#elif defined(CF376_CF429)
    #define HAVE_BUTTONS      0
#elif defined(CF376_CF440)
    #define HAVE_BUTTONS      0
#elif defined(CF133)
    #define HAVE_BUTTONS      0
#elif defined(QCC5127_AA_DEV_BRD_R2_AA)
	#define HAVE_BUTTONS      0
#elif defined(CG437)
    #define HAVE_BUTTONS      0
#elif defined(CF376_CG724)
    #define HAVE_BUTTONS      1
    #define SWITCH_BANK       0
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(2), SW_PIO_MASK(3), SW_PIO_MASK(4), SW_PIO_MASK(5),
        SW_PIO_MASK(23), SW_PIO_MASK(24), SW_PIO_MASK(25), SW_PIO_MASK(21)
    };
#elif defined(QCC5127_AB_DEV_BRD_R2_AA)
    #define HAVE_BUTTONS      1
    #define SWITCH_BANK       1
    #define SW_PIO_MASK(pio)  (1ul << ((pio)-(SWITCH_BANK*PIOS_PER_BANK)))
    /* Set up a table to identify which switch matches a bitmask */
    uint32 switch_mask_table[NUM_SWITCHES] = { 0,
        SW_PIO_MASK(52), SW_PIO_MASK(53), SW_PIO_MASK(54), SW_PIO_MASK(55),
        SW_PIO_MASK(56), SW_PIO_MASK(57), SW_PIO_MASK(40), 0
    };
#else
    #error HW Variant not supported
#endif

#if HAVE_BUTTONS

/* Define a task for handling Pio messages */
static void button_message_handler ( Task pTask, MessageId pId, Message pMessage );
static TaskData button_message_task = {button_message_handler};

/* Application callback function */
static dev_board_button_callback app_button_callback = (dev_board_button_callback)NULL;

/*! Structure to hold information about the buttons.
 * Only hold the last state of the PIOs at present, in
 * buttonState variable
 * */
static struct
{
    uint32      buttonState;
} buttonData;

/*!
 * \brief Compute combined bitmask for a set of switches.
 *
 * \param pTable    The switch mask table
 * \param pSize     The number of elements in the table
 * \result the bitwise or of all the table entries
 */
static uint32 get_mask_for_all_switches ( uint32 *pTable, size_t pSize )
{
    uint32 result = 0;
    for ( size_t i = 0 ; i < pSize ; i++ ) {
        result |= pTable[i];
    }
    return result;
}

/*!
 * \brief Handler for messages from the system about PIO changes
 *
 * This handler only handles the MESSAGE_PIO_CHANGED event from
 * the system.
 *
 * We assume that we will only receive events that we have
 * requested, so do not filter out any events that may apply to a
 * different bank of PIOs.
 *
 * If a single button is pressed we call the application callback
 * with the number of the button (1-8)
 *
 * \param pTask Task data associated with events
 * \param pId The ID of the message we have received
 * \param pMessage Pointer to the message content (if any)
 */
static void button_message_handler ( Task pTask, MessageId pId, Message pMessage )
{
    const MessagePioChanged * lMessage;
    unsigned button;

    UNUSED(pTask);

    switch (pId)
    {
    case MESSAGE_PIO_CHANGED:
        lMessage = ( const MessagePioChanged * ) (pMessage ) ;

        uint32 newPioState = (uint32)lMessage->state | ((uint32)lMessage->state16to31)<<16;
        uint32 change = buttonData.buttonState ^ newPioState;

        /* Only action when changed, with 1 bit set and only 1 bit changed */
        if (   change
            && !(change & (change-1))
            && (change & newPioState))
        {
            for (button = 1; button < sizeof(switch_mask_table)/sizeof(switch_mask_table[0]); button++)
            {
                if (change == switch_mask_table[button])
                {
                    (*app_button_callback)((uint8)button);
                }
            }
        }

        /* save the state regardless */
        buttonData.buttonState = newPioState;

        break;

    default:
        /* Other messages are silently ignored */
        break;
    }
}


/*!
 * \brief Initialise button handling for the dev board.
 *
 * \param cb  The application call back function

 * \internal See header file for fuller description
 */
void dev_board_initialise_buttons(dev_board_button_callback cb)
{
    if (!cb)
        Panic();
    if (app_button_callback)
        Panic();

    MessagePioTask(&button_message_task);

    /* Enable the PIOs with connected switches */
    uint32 allSwitchesMask = get_mask_for_all_switches(switch_mask_table,NUM_SWITCHES);
    PioSetMapPins32Bank(SWITCH_BANK, allSwitchesMask, allSwitchesMask);
    PioSetDir32Bank(SWITCH_BANK, allSwitchesMask, 0);
    PioSet32Bank(SWITCH_BANK, allSwitchesMask, 0);
    PioSetStrongBias32Bank(SWITCH_BANK, allSwitchesMask, allSwitchesMask);

    /* Ensure wake from deep sleep when button is pressed */
    PioSetDeepSleepEitherLevelBank(SWITCH_BANK, allSwitchesMask, allSwitchesMask);

    /* Request monitoring of the button PIOs with suitable
     * parameters for. Must be pressed for 4 * 5 ms intervals
     */
    PioDebounce32Bank(SWITCH_BANK,allSwitchesMask,4,5);

    app_button_callback = cb;
}

#else

/* NOP function when there is no hardware support for buttons */
void dev_board_initialise_buttons(dev_board_button_callback cb)
{
    UNUSED(cb);
}

#endif
