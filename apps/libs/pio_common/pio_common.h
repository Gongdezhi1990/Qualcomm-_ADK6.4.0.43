/*************************************************************************
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
 

FILE : 
    pio_common.h

CONTAINS:
    Common PIO operations used by libraries and applications

**************************************************************************/

#ifndef PIO_COMMON_H_
#define PIO_COMMON_H_

#define PIN_INVALID 0xFF

typedef enum
{
    pio_pull = 0,
    pio_input = 0,
    pio_drive
} pio_common_dir;

typedef enum
{
    pin_pcma_in     = (1 <<  0),
    pin_pcma_out    = (1 <<  1),
    pin_pcma_sync   = (1 <<  2),
    pin_pcma_clk    = (1 <<  3),
    pin_pcmb_in     = (1 <<  4),
    pin_pcmb_out    = (1 <<  5),
    pin_pcmb_sync   = (1 <<  6),
    pin_pcmb_clk    = (1 <<  7),
    pin_spdifa_rx   = (1 <<  8),
    pin_spdifa_tx   = (1 <<  9),
    pin_spdifb_rx   = (1 << 10),
    pin_spdifb_tx   = (1 << 11)
} pio_common_pin_t;

typedef enum
{
    /* PIO functions provided by pio_if.h. */
    pin_function_uart_rx,
    pin_function_uart_tx,
    pin_function_uart_rts,
    pin_function_uart_cts,
    pin_function_pcm_in,
    pin_function_pcm_out,
    pin_function_pcm_sync,
    pin_function_pcm_clk,
    pin_function_sqif,
    pin_function_led,
    pin_function_lcd_segment,
    pin_function_lcd_common,
    pin_function_pio,
    pin_function_spdif_rx,
    pin_function_spdif_tx,
    pin_function_bitserial_0_clock_in,
    pin_function_bitserial_0_clock_out,
    pin_function_bitserial_0_data_in,
    pin_function_bitserial_0_data_out,
    pin_function_bitserial_0_sel_in,
    pin_function_bitserial_0_sel_out,
    pin_function_bitserial_1_clock_in,
    pin_function_bitserial_1_clock_out,
    pin_function_bitserial_1_data_in,
    pin_function_bitserial_1_data_out,
    pin_function_bitserial_1_sel_in,
    pin_function_bitserial_1_sel_out,
    pin_function_analogue,
    pin_function_other,
    /* Other PIO functions defined by ADK config rather than pio_if.h. */
    pin_function_interrupt_active_low = 255
} pio_common_pin_function_id;

#define MAX_NUMBER_OF_PIO_BANKS    3

/*!
 * @brief Structure used to contains mask or values from all PIOs.
 *
 * A structure is provided for this as some supported devices have more than 
 * 32 PIOs.
 */
typedef struct
{
        /*! Actual mask or value. The accessor functions for this type should
         *  be used to access specific bits. */
    uint32 mask[MAX_NUMBER_OF_PIO_BANKS];
} pio_common_allbits;

/*!
    @brief pioCommonCheckValid, check if the pio is valid

    @param pio, the PIO to be configured

    @returns TRUE if valid, FALSE otherwise
*/

bool pioCommonCheckValid(uint8 pio);

/*!
    @brief PioCommonSetPio, will drive/pull a PIO to the specified level

    @param pio, the PIO to be configured
    @param direction, the direction of the PIO, pio_drive for output,
           pio_pull as input
    @param level, the level to drive the pio to, 1 being high, 0 low
    
    @returns TRUE if successful, FALSE otherwise
*/

bool PioCommonSetPio(uint8 pio, pio_common_dir direction, bool level);

/*!
    @brief PioCommonGetPio, will configure a pin as input and attempt to
           read it.

    @param pio, the PIO to be set to an input and read

    @returns TRUE if the pin is high, FALSE if the pin is low or could
             not be configured.
*/
bool PioCommonGetPio(uint8 pio);

/*!
    @brief PioCommonDebounce, configures the requested pin_mask as
    inputs and attempts to debounce them.

    @param[in,out] pio_mask, Pointer to pio_common_allbits structure
                   with complete mask of PIOs
    @param count, number of successive reads required
    @param period, time between reads

    @returns TRUE If all pins requested debounced
*/

bool PioCommonDebounce(pio_common_allbits *pio_mask, uint16 count, uint16 period);

/*!
    @brief Configures a debounce group.
    
    @param[in] group               Debounce group ID.
    @param[in] debounceMask        Mask.
    @param[in] num_checks          Number of times a bit is checked.
    @param[in] time_between_checks Time between each check.
    
    @returns TRUE if all pins debounced, otherwise FALSE.
*/

bool PioCommonDebounceGroup(uint16             group,
                            pio_common_allbits *pio_mask,
                            uint16             count,
                            uint16             period);

/*! 
    @brief PioCommonSetMap, specifies pins to be used as PIOs rather than
           their normal function.

    @note The PIOs that may be successfully used as PIOs may be restricted on
    the device.
    
    @param mask, the mask of PIOs to be updated.
    @param bits, the setting for the specific bits

    @returns TRUE if requested changes were successful. FALSE, and a non-zero result, 
             in case of failure
*/ 
bool PioCommonSetMap(pio_common_allbits *mask, pio_common_allbits *bits);

/*!
    @brief PioCommonBitsInit, initialise a pio_common_allbits structure to zero.

    @param[in,out] mask, pointer to pio_common_allbits structure to be set to zero
*/
void PioCommonBitsInit(pio_common_allbits *mask);

/*!
    @brief PioCommonSetDir, this function sets the PIO direction.

    @param[in] mask pio_common_allbits structure to be modified.
    @param[in] direction pio_common_allbits structure containing direction.
*/
void PioCommonSetDir(pio_common_allbits mask, pio_common_allbits direction);

/*!
    @brief PioCommonSet, this function sets the output level or input pull sense (up or down).

    @param[in] mask pio_common_allbits structure to be modified.
    @param[in] bits pio_common_allbits structure containing level/direction.
*/
void PioCommonSet(pio_common_allbits mask, pio_common_allbits bits);

/*!
    @brief PioCommonSetStrongBias, this function selects use of strong or weak pull.

    @param[in] mask pio_common_allbits structure to be modified.
    @param[in] bits pio_common_allbits structure containing the required drive strength.
*/
void PioCommonSetStrongBias(pio_common_allbits mask, pio_common_allbits bits);

/*!
    @brief PioCommonBitsSetBit, set a bit representing the requested pio a mask.

    @param[in,out] mask, pointer to pio_common_allbits structure to be modified
    @param[in] pio, PIO to be set in the mask
*/
void PioCommonBitsSetBit(pio_common_allbits *mask, uint8 pio);

/*!
    @brief PioCommonBitsBitIsSet, check if a specified PIO is enabled in the
           supplied mask.

    @param mask, pointer to a pio_common_allbits structure to be checked
    @param pio, the PIO to be checked.

    @returns TRUE if the bit is set, FALSE if not set or the mask pointer is NULL.
*/
bool PioCommonBitsBitIsSet(pio_common_allbits *mask, uint8 pio);

/*!
    @brief PioCommonBitsIsEmpty, check if any PIOs are enabled in the supplied mask.

    @param mask, pointer to a pio_common_allbits structure to be checked

    @returns TRUE if no bits are set.
*/
bool PioCommonBitsIsEmpty(pio_common_allbits *mask);

/*!
    @brief PioCommonBitsOR, this function combines two pio masks by a
           logical OR

    @note Either of the mask1 or mask2 parameters can be the same as result.

    @param[in,out] result, pointer to pio_common_allbits structure to be modified
    @param[in] mask1, pointer to pio_common_allbits structure to combined
    @param[in] mask2, pointer to pio_common_allbits structure to combined

    @returns TRUE if any bits in the result are set false otherwise

*/
bool PioCommonBitsOR(pio_common_allbits *result,
                     pio_common_allbits *mask1,
                     pio_common_allbits *mask2);

/*!
    @brief PioCommonBitsXOR, checks for changes between two PIO masks

    @note The masks are checked for differences, in either direction, with a bit being set
          in the result if the bits are different.
          Either of the mask or old_mask parameter can be the same as result

    @param[in,out] result, pointer to pio_common_allbits structure for the result
    @param[in] mask1, pointer to pio_common_allbits structure.
    @param[in] mask2, pointer to pio_common_allbits structure.

    @return TRUE if any bits change changed, false otherwise
*/
bool PioCommonBitsXOR(pio_common_allbits *result,
                      pio_common_allbits *mask1,
                      pio_common_allbits *mask2);

/*!

    @brief PioCommonBitsAND, this function combines two pio masks by a
           logical AND

    @note Either of the mask1 or mask2 parameters can be the same as result.

    @param[in,out] result, pointer to pio_common_allbits structure to be modified
    @param[in] mask1 Pointer to pio_common_allbits structure to combined
    @param[in] mask2 Pointer to pio_common_allbits structure to combined

    @returns TRUE if any bits in the result are set false otherwise
*/
bool PioCommonBitsAND(pio_common_allbits *result,
                      pio_common_allbits *mask1,
                      pio_common_allbits *mask2);

/*!
    @brief PioCommonBitsRead, read all PIOs into the supplied pio_common_allbits
           structure.

    @param[in,out] pioState, pointer to the structure to hold the read PIO state
*/
void PioCommonBitsRead(pio_common_allbits *pioState);

/*!
    @brief PioCommonXORWithInvertMask, invert PIOs based on invert mask.

    @note Any bit specified in the pioInvertMask will be inverted in the supplied
          pioState structure. Bits not affected by the pioInvertMask will be
          unchanged.

    @param[in,out] pioState, Pointer to the structure to update
*/
void PioCommonXORWithInvertMask(pio_common_allbits *pioState);

/*!
    @brief PioCommonStoreInvertMask, stores the supplied invert mask.

    @param[in] pioInvertMask Pointer to the invert mask to be stored
*/
void PioCommonStoreInvertMask(pio_common_allbits *pioInvertMask);

/*!
    @brief PioCommonEnableFunctionPins, enables special functions of device pins.

    @param[in] pins, bitmap of pins to be enabled
*/
bool PioCommonEnableFunctionPins(pio_common_pin_t pins);

/*!
    Enables SPDIF input functionality in a device pin.

    @param[in] pin to be assigned as SPDIF input
*/
void PioCommonConfigureSpdifInput(uint8 spdif_input_pin);

/*!
    Enables SPDIF output functionality in a device pin.

    @param[in] pin to be assigned as SPDIF output
*/
void PioCommonConfigureSpdifOutput(uint8 spdif_output_pin);

/*!
    Sets function on a device pin.

    @param[in] pin to be assigned
    @param[in] function to assign to pin

    @returns TRUE if pin function set, FALSE otherwise
*/
bool PioCommonSetFunction (uint16 pin, pio_common_pin_function_id function);

#endif /*PIO_COMMON_H_*/
