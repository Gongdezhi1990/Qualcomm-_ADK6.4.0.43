/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_pin.c
DESCRIPTION
    Various functions to handle pin code entry.
*/

#include <stdio.h>
#include "hid.h"
#include "hid_private.h"

/****************************************************************************
NAME 
    HidPinInit
DESCRIPTION
    Initialises a HID PIN structure, must be called before another PIN
    functions.
RETURNS
    void
*/
void HidPinInit(hid_pin *pin)
{
    pin->pin_code_length = 0;
    pin->key_down = 0;
}

/****************************************************************************
NAME 
    HidPinCodeHandleReport
DESCRIPTION
    Called to handle an HID input report, parse report and builds up a pin
    code string.
RETURNS
    hid_pin_status - Current status of pin code entered.
*/
hid_pin_status HidPinCodeHandleReport(hid_pin *pin, const uint8 *data, int length)
{
    int index;
    uint8 key_down;

    /* Before processing, we need to check that this is a boot keyboard report.
       Should be at least 10 bytes with Input Report header and report ID of 1 */
    if ((length < 10) || (data[0] != 0xA1) || (data[1] != 0x01))
        return hid_pin_error;

    /* Check that only one key is pressed by making sure that keycodes 2-6 are zero */
    for (index = 5; index < 10; index++)
    {	if (data[index] != 0x00)
            return hid_pin_error;
    }
    
    /* We now check the value of keycode 1 */
    key_down = data[4];

    /* If a key is down and there was one down last time then something has gone wrong,
       we expect zero reports when a key is released */
    if (key_down && pin->key_down)
        return hid_pin_error;

    /* Need to special case the release of the enter key.  this is because we don't want 
       to send the pin until the enter key goes back up again */
    if	(!key_down)
        if ((pin->key_down == 0x28) || (pin->key_down == 0x58))
             return hid_pin_complete;

    /* Store key down so we can make sure its released before processing the next one */
    pin->key_down = key_down;

    /* Process key */
    switch (key_down)
    {
        default:
        {
            uint8 ascii = 0;

            /* Convert usage ID to ASCII */
            if ((key_down >= 0x04) && (key_down <= 0x1d))        /* Keyboard letters */
                ascii = 'A' + (key_down - 0x04);
            else if ((key_down >= 0x1e) && (key_down <= 0x26))   /* Keyboard numbers */
                ascii = '1' + (key_down - 0x1e);
            else if (key_down == 0x27)                           /* Keyboard 0 */
                ascii = '0';
            else if ((key_down >= 0x59) && (key_down <= 0x61))   /* Keypad numbers */
                ascii = '1' + (key_down - 0x59);
            else if (key_down == 0x62) /* keypad 0 */
                ascii = '0';

            /* Add valid ASCII value to PIN */
            if (ascii)
            {
                HID_PRINT(("Pin: %c\n", ascii));

                /* Max PIN length is 16 chars. We could send the pin the instant we get 16
                   characters, but this doesn't allow the user to delete and retype the
                   last char. */
                if (pin->pin_code_length < HID_PIN_MAX_LENGTH)
                {
                    /* Add to PIN */
                    pin->pin_code[pin->pin_code_length++] = ascii;
                    return hid_pin_add;
                }
                else
                {
                    return hid_pin_full;
                }
            }
        }
        break;

        case 0x28: /* Keyboard enter */
        case 0x58: /* Keypad enter */
          break;
        
        case 0x2a: /* Keyboard backspace */
        case 0x4c: /* Keyboard delete */
        case 0x63: /* Keypad del (on .) */
        {
            if (pin->pin_code_length > 0)
            {
                /* Delete last character */
                pin->pin_code_length--;
                return hid_pin_delete;
            }	
            else
                return hid_pin_cleared;		
        }
    }
    
    /* Indicate we parsed input report OK */
    return hid_pin_ok;
}

/****************************************************************************
NAME 
    HidPinCodeLength
DESCRIPTION
    Returns length of entered pin code.
RETURNS
    int - Current length of pin code, 0 if no pin code.
*/
int HidPinCodeLength(hid_pin *pin)
{
    return pin->pin_code_length;
}

/****************************************************************************
NAME 
    HidPinCodeData
DESCRIPTION
    Returns pointer to entered pin code.
RETURNS
    const uint8 * - Pointer to pin code data, or NULL if no pin code.
*/
const uint8 *HidPinCodeData(hid_pin *pin)
{
    if (pin->pin_code_length)
        return pin->pin_code;
    else
        return 0;
}
