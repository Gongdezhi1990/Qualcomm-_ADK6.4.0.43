#ifndef __PIO_H__
#define __PIO_H__
#include <app/pio/pio_if.h>

/*! file  @brief Access BlueCore I/O lines 
** 
** 
BlueCore variants from BlueCore2 onwards have twelve Programmable Input/Output(PIO) 
pins and a further three or four pins (dependant on BlueCore variant) may
be available if they are not being used as AIO pins.
**
Some BlueCore variants from BlueCore5-MM onwards have 16 PIO pins and 
can also control some of the chips UART/PCM lines by mapping them in
as PIO pins 16 and greater. Mapping a UART/PCM line as a PIO line stops 
the line performing its original function, so these lines should only be 
used as PIOs when these interfaces are not required. 
**
Attempts to configure the upper 16 PIOs will be rejected if these
pins have not been mapped first. 
**
Persistent Store keys may be set to hand control of certain PIO pins over 
to the BlueCore firmware. Examples include PSKEY_USB_PIO_WAKEUP, PSKEY_USB_PIO_DETACH,
PSKEY_USB_PIO_PULLUP and PSKEY_USB_PIO_VBUS.
In such configurations the VM should not attempt to use these pins.
**
Various other hardware and software configurations make use of specific PIO pins.
Examples include
- Pio[1:0] Used to control external hardware on Class 1 modules.
- Pio[7:6] Used for I2C.
*/

#if TRAPSET_CORE

/**
 *  \brief Configure PIO monitoring.
 *   This function configures a simple debounce engine for PIO input
 *   pins. It can detect transitions on PIOs and return stabilised
 *   readings for them, filtering out glitches.
 *   When the engine detects a change in state of one or more of the
 *   monitored pins, it repeatedly reads the pin state. If the state of
 *   all the monitored pins remains unchanged for \e count successive
 *   samples, it is considered stable and a \#MESSAGE_PIO_CHANGED message
 *   is sent to the task registered with MessagePioTask().
 *   Calling PioDebounce has the same behaviour as calling PioDebounce32Bank using
 *   bank 0 or as calling PioDebounceGroup32Bank using group 0 and bank 0.
 *   \note
 *   It is not possible to configure different debounce settings for
 *   different pins or groups of pins, or to debounce pins independently
 *   of each other.
 *   Examples:
 *   To enable events on PIO2 and PIO3, with 4 reads 2ms apart (6ms
 *   total):
 *   \code
 *   PioDebounce32(1u<<2 | 1u<<3, 4, 2);
 *   \endcode
 *   To enable events on any transition on PIO5 with no debouncing:
 *   \code
 *   PioDebounce32(1u<<5, 1, 0);
 *   \endcode
 *  \param mask Bitmask indicating which pins to monitor. Setting this to zero disables PIO
 *  monitoring. 
 *  \param count How many times the monitored pins' state must be observed to be consistent
 *  before it is considered stable. Zero and one have special significance. If \e
 *  count is 1, then no debouncing is performed - any perceived change of the pins
 *  will raise the event. If \e count is 0, any transition raises the event, even
 *  if no change in pin state can be detected; in this case, the "state" members
 *  of \#MessagePioChanged may show no change in state. 
 *  \param period The delay in milliseconds between successive reads of the pins. 
 *  \return A 32 bit mask. If any bit in this mask is high then monitoring could not be set
 *  up for that PIO; no action will have been taken on any PIOs (any previously
 *  set up monitoring will remain in force). The most likely reasons for a
 *  non-zero return are attempting to debounce nonexistent PIOs, or attempting to
 *  debounce unmapped PIOs. See the PioSetMapPins32() documentation for
 *  information on which PIOs can be mapped.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioDebounce32(uint32 mask, uint16 count, uint16 period);

/**
 *  \brief Configure PIO monitoring.
 *   This function configures a simple debounce engine for PIO input
 *   pins. It can detect transitions on PIOs and return stabilised
 *   readings for them, filtering out glitches.
 *   When the engine detects a change in state of one or more of the
 *   monitored pins, it repeatedly reads the pin state. If the state of
 *   all the monitored pins remains unchanged for \e count successive
 *   samples, it is considered stable and a \#MESSAGE_PIO_CHANGED message
 *   is sent to the task registered with MessagePioTask().
 *   Calling PioDebounce32Bank has the same behaviour as calling
 *   PioDebounceGroup32Bank using  group 0. 
 *   \note
 *   It is not possible to configure different debounce settings for
 *   different pins or groups of pins, or to debounce pins independently
 *   of each other.
 *   Examples:
 *   To enable events on PIO2 and PIO3, with 4 reads 2ms apart (6ms
 *   total):
 *   \code
 *   PioDebounce32Bank(1u<<2 | 1u<<3, 4, 2);
 *   \endcode
 *   To enable events on any transition on PIO5 with no debouncing:
 *   \code
 *   PioDebounce32Bank(1u<<5, 1, 0);
 *   \endcode
 *  \param bank PIO bank number.
 *  \param mask Bitmask indicating which pins to monitor. Setting this to zero disables PIO
 *  monitoring. 
 *  \param count How many times the monitored pins' state must be observed to be consistent
 *  before it is considered stable. Zero and one have special significance. If \e
 *  count is 1, then no debouncing is performed - any perceived change of the pins
 *  will raise the event. If \e count is 0, any transition raises the event, even
 *  if no change in pin state can be detected; in this case, the "state" members
 *  of \#MessagePioChanged may show no change in state. 
 *  \param period The delay in milliseconds between successive reads of the pins. 
 *  \return A 32 bit mask. If any bit in this mask is high then monitoring could not be set
 *  up for that PIO; no action will have been taken on any PIOs (any previously
 *  set up monitoring will remain in force). The most likely reasons for a
 *  non-zero return are attempting to debounce nonexistent PIOs, or attempting to
 *  debounce unmapped PIOs. See the PioSetMapPins32() documentation for
 *  information on which PIOs can be mapped.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioDebounce32Bank(uint16 bank, uint32 mask, uint16 count, uint16 period);

/**
 *  \brief Configure PIO monitoring.
 *   This function configures a simple debounce engine for PIO input
 *   pins. It can detect transitions on PIOs and return stabilised
 *   readings for them, filtering out glitches. It supports two distinct groups of
 *   PIOs. The settings apply to all the selected PIOs in a group. A PIO can belong
 *   in both groups at the same time and will fulfill both functions.
 *   When the engine detects a change in state of one or more of the
 *   monitored pins, it repeatedly reads the pin state. If the state of
 *   all the monitored pins remains unchanged for \e count successive
 *   samples, it is considered stable and a \#MESSAGE_PIO_CHANGED message
 *   is sent to the task registered with MessagePioTask().
 *   \note
 *   It is not possible to configure different debounce settings for different pins
 *   within the same group. If a pin is used in more than one group it will
 *   generate one message for each group after each group's conditions have been
 *   met.
 *   Examples:
 *   On group 0, enable events on PIO2 and PIO3, with 4 reads 2ms apart (6ms
 *   total):
 *   \code
 *   PioDebounceGroup32Bank(0, 0, 1u<<2 | 1u<<3, 4, 2);
 *   \endcode
 *   On group 1, enable events on any transition on PIO5 with no debouncing:
 *   \code
 *   PioDebounceGroup32Bank(1, 0, 1u<<5, 1, 0);
 *   \endcode
 *  \param group PIO group number, 0 or 1.
 *  \param bank PIO bank number.
 *  \param mask Bitmask indicating which pins to monitor. Setting this to zero disables PIO
 *  monitoring. 
 *  \param count How many times the monitored pins' state must be observed to be consistent
 *  before it is considered stable. Zero and one have special significance. If \e
 *  count is 1, then no debouncing is performed - any perceived change of the pins
 *  will raise the event. If \e count is 0, any transition raises the event, even
 *  if no change in pin state can be detected; in this case, the "state" members
 *  of \#MessagePioChanged may show no change in state. 
 *  \param period The delay in milliseconds between successive reads of the pins. 
 *  \return A 32 bit mask. If any bit in this mask is high then monitoring could not be set
 *  up for that PIO; no action will have been taken on any PIOs (any previously
 *  set up monitoring will remain in force). The most likely reasons for a
 *  non-zero return are attempting to debounce nonexistent PIOs, or attempting to
 *  debounce unmapped PIOs. See the PioSetMapPins32() documentation for
 *  information on which PIOs can be mapped. If the selected group is not
 *  supported then the full mask is returned.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioDebounceGroup32Bank(uint16 group, uint16 bank, uint32 mask, uint16 count, uint16 period);

/**
 *  \brief Returns the contents of the PIO data input register. For PIOs set as outputs,
 *  this function will return the value last written using PioSet32(). 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGet32(void );

/**
 *  \brief Returns the contents of one of the PIO data input registers. For PIOs set as
 *  outputs, this function will return the value last written using
 *  PioSet32Bank(). 
 *  \param bank PIO bank number.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGet32Bank(uint16 bank);

/**
 *  \brief Modifies the contents of the PIO data output register. PIO pins must be set to
 *  outputs via PioSetDir32() before they can be driven high or low through this
 *  trap. This trap also sets pull direction for PIOs used as inputs.
 *   
 *   Note that all PIOs must be mapped in before they can be used.
 *   See the PioSetMapPins32() documentation for information on valid PIO
 *  directions
 *   and PIO mapping.  
 *   BlueCore has internal resistors which can be configured to either pull-up or 
 *   pull-down the pins used for input. This is controlled by the value 
 *   written to the output register using PioSet32().
 *   The resistors pull-down if the value is zero, and pull-up otherwise, so the 
 *   following fragment sets pins 1 and 2 to inputs with pin 1 configured to 
 *   pull-up and pin 2 configured to pull-down.
 *   \code
 *   PioSet32(2|4, 2);
 *   PioSetDir32(2|4, 0);
 *   \endcode
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified.
 *  \param bits Each bit in the "bits" value corresponds to a PIO line. Bits set to 1 in this
 *  value will result in that PIO line being driven high. Bits set to 0 in this
 *  value will result in that PIO line being driven low.
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be
 *  driven to the level specified; note that no action will have been taken on any
 *  PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSet32(uint32 mask, uint32 bits);

/**
 *  \brief Modifies the contents of the PIO data output register. PIO pins must be set to
 *  outputs via PioSetDir32Bank() before they can be driven high or low through
 *  this trap. This trap also sets pull direction for PIOs used as inputs.
 *   
 *   Note that all PIOs must be mapped in before they can be used.
 *   See the PioSetMapPins32Bank() documentation for information on valid PIO
 *  directions
 *   and PIO mapping.  
 *   BlueCore has internal resistors which can be configured to either pull-up or 
 *   pull-down the pins used for input. This is controlled by the value 
 *   written to the output register using PioSet32Bank().
 *   The resistors pull-down if the value is zero, and pull-up otherwise, so the 
 *   following fragment sets pins 1 and 2 to inputs with pin 1 configured to 
 *   pull-up and pin 2 configured to pull-down.
 *   \code
 *   PioSet32Bank(0, 2|4, 2);
 *   PioSetDir32Bank(0, 2|4, 0);
 *   \endcode
 *  \param bank PIO bank number.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified.
 *  \param bits Each bit in the "bits" value corresponds to a PIO line. Bits set to 1 in this
 *  value will result in that PIO line being driven high. Bits set to 0 in this
 *  value will result in that PIO line being driven low.
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be
 *  driven to the level specified; note that no action will have been taken on any
 *  PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSet32Bank(uint16 bank, uint32 mask, uint32 bits);

/**
 *  \brief Read whether PIOs are set as inputs or outputs. 
 *  \return Each bit in the return value corresponds to a PIO line. Bits set to 1 mean that
 *  PIO line is configured as an output. Bits set to 0 mean it is configured as an
 *  input.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetDir32(void );

/**
 *  \brief Read whether PIOs are set as inputs or outputs. 
 *  \param bank PIO bank number.
 *  \return Each bit in the return value corresponds to a PIO line. Bits set to 1 mean that
 *  PIO line is configured as an output. Bits set to 0 mean it is configured as an
 *  input.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetDir32Bank(uint16 bank);

/**
 *  \brief Set PIOs as inputs or outputs.
 *   
 *   Note that all PIOs must be mapped in before they can be used.
 *   See the PioSetMapPins32() documentation for information on valid PIO
 *  directions
 *   and PIO mapping.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified.
 *  \param dir Each bit in the "dir" value corresponds to a PIO line. Bits set to 1 in this
 *  value will result in that PIO line being configured as an output. Bits set to
 *  0 in this value will result in that PIO line being configured as an input.
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be set
 *  to the direction specified; note that no action will have been taken on any
 *  PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetDir32(uint32 mask, uint32 dir);

/**
 *  \brief Set PIOs as inputs or outputs.
 *   
 *   Note that all PIOs must be mapped in before they can be used.
 *   See the PioSetMapPins32Bank() documentation for information on valid PIO
 *  directions
 *   and PIO mapping.
 *  \param bank PIO bank number.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified.
 *  \param dir Each bit in the "dir" value corresponds to a PIO line. Bits set to 1 in this
 *  value will result in that PIO line being configured as an output. Bits set to
 *  0 in this value will result in that PIO line being configured as an input.
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be set
 *  to the direction specified; note that no action will have been taken on any
 *  PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetDir32Bank(uint16 bank, uint32 mask, uint32 dir);

/**
 *  \brief Read whether PIOs are set to use strong or weak pull.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetStrongBias32(void );

/**
 *  \brief Read whether PIOs are set to use strong or weak pull.
 *  \param bank PIO bank number.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetStrongBias32Bank(uint16 bank);

/**
 *  \brief Set PIOs to use strong or weak pull when used as inputs. 
 *   
 *   \code
 *    PioSetDir32((1<<4)|(1<<5), 0);
 *    PioSet32((1<<4)|(1<<5), (1<<4));
 *    PioSetStrongBias32((1<<4)|(1<<5), (1<<5));
 *   \endcode
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified. 
 *  \param bits Each bit in the "bits" value corresponds to a PIO line. Bits set to 1 in this
 *  value will result in that PIO line being configured as having strong bias.
 *  Bits set to 0 in this value will result in that PIO line being configured as
 *  not having strong bias. 
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be set
 *  to use strong bias; note that no action will have been taken on any PIOs.
 *  BlueCore includes weak internal pull-ups or pull-downs on pins which are being
 *  used as inputs (see PioSet32()). This function allows the pull-up or pull-down
 *  to be made stronger on a per-pin basis. So to set pin 4 and 5 as inputs, 4
 *  pulled up weakly, 5 pulled down strongly
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetStrongBias32(uint32 mask, uint32 bits);

/**
 *  \brief Set PIOs to use strong or weak pull when used as inputs. 
 *   
 *   \code
 *    PioSetDir32((1<<4)|(1<<5), 0);
 *    PioSet32((1<<4)|(1<<5), (1<<4));
 *    PioSetStrongBias32((1<<4)|(1<<5), (1<<5));
 *   \endcode
 *  \param bank PIO bank number.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified. 
 *  \param bits Each bit in the "bits" value corresponds to a PIO line. Bits set to 1 in this
 *  value will result in that PIO line being configured as having strong bias.
 *  Bits set to 0 in this value will result in that PIO line being configured as
 *  not having strong bias. 
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be set
 *  to use strong bias; note that no action will have been taken on any PIOs.
 *  BlueCore includes weak internal pull-ups or pull-downs on pins which are being
 *  used as inputs (see PioSet32()). This function allows the pull-up or pull-down
 *  to be made stronger on a per-pin basis. So to set pin 4 and 5 as inputs, 4
 *  pulled up weakly, 5 pulled down strongly
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetStrongBias32Bank(uint16 bank, uint32 mask, uint32 bits);

/**
 *  \brief Returns a 32 bit value showing which PIO lines have been mapped to chip pins
 *  (see documentation for PioSetMapPins32() for more detail).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetMapPins32(void );

/**
 *  \brief Returns a 32 bit value showing which PIO lines have been mapped to chip pins
 *  (see documentation for PioSetMapPins32() for more detail).
 *  \param bank PIO bank number.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetMapPins32Bank(uint16 bank);

/**
 *  \brief Maps PIOs as software or hardware controlled.
 *   Before using a PIO as a software controlled digital IO a call to this
 *   function is required. Not mapping the PIO may cause other functions in the
 *   PIO trap API to return errors and not produce the required behaviour.
 *   To put a PIO under HW control the app needs to call this function first and
 *   then PioSetFunction to select the HW functionality needed.
 *   Please note that there is no default state, all PIOs should be considered
 *   unmapped and unusable until they are configured by a call to this function.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified. 
 *  \param bits Each bit corresponds to a PIO line. A bit set to 1 will cause a PIO to be
 *  behave as a software controlled pin. A bit set to 0 will result in the pio
 *  being marked as controlled by a hardware peripheral.
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be
 *  mapped or unmapped; note that no action will have been taken on any PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetMapPins32(uint32 mask, uint32 bits);

/**
 *  \brief Maps PIOs as software or hardware controlled.
 *   Before using a PIO as a software controlled digital IO a call to this
 *   function is required. Not mapping the PIO may cause other functions in the
 *   PIO trap API to return errors and not produce the required behaviour.
 *   To put a PIO under HW control the app needs to call this function first and
 *   then PioSetFunction to select the HW functionality needed.
 *   Please note that there is no default state, all PIOs should be considered
 *   unmapped and unusable until they are configured by a call to this function.
 *  \param bank PIO bank number.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified. 
 *  \param bits Each bit corresponds to a PIO line. A bit set to 1 will cause a PIO to be
 *  behave as a software controlled pin. A bit set to 0 will result in the pio
 *  being marked as controlled by a hardware peripheral.
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be
 *  mapped or unmapped; note that no action will have been taken on any PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetMapPins32Bank(uint16 bank, uint32 mask, uint32 bits);

/**
 *  \brief Sets a supported function for particular pio pin 
 *   if a pin can be mapped as LED, UART, BITSERIAL, ANALOGUE etc, then this
 *   function can be used to set the pin for one of the supported functions.
 *   Consult the device's data sheet to understand what functions are supported
 *   for each PIO pins.
 *   A pin can support only few functions. Trying to set a function which is NOT
 *   supported by the pin will return FALSE without affecting/modifying the
 *   existing pin function.
 *   Before this trap can be used, it is necessary to use PioSetMapPins32Bank to
 *   put the PIO in HW mode.
 *   For functions corresponding to other susbystems the OTHER function ID must be
 *   used. This reverts the PIOs to the initial unmapped state. Any further use by
 *   the app needs remapping.
 *   PioSetFunction() Usage Example:
 *   if PIO[20] can be mapped to UART/BITSERIAL/LED/PIO and currently PIO[20] is
 *   mapped as PIO, then App should call the PioSetMapPins32Bank() to unmap it
 *   from PIO and then call PioSetFunction() to map to (UART/BITSERIAL/LED/PIO)
 *   function.
 *   Unmap PIO[20] so that it can be mapped to a function
 *   PioSetMapPins32Bank(0, 1<<20, 0<<20);
 *   This will map PIO[20] line as UART_RX
 *   PioSetFunction(20, UART_RX);  
 *   To map back the PIO[20] as a PIO, PioSetMapPins32Bank() should be used.
 *   PioSetMapPins32Bank(0, 1<<20, 1<<20);
 *  \param pin  Pin that requires a function change; the pin value ranges from 0 to 95. 
 *  \param function  Supported function that needs to be set for the specified pin. Refer
 *  \#pin_function_id
 *  \return TRUE if successful, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
bool PioSetFunction(uint16 pin, pin_function_id function);

/**
 *  \brief Sets the drive strength for given PIOs 
 * Depending on the PIO in question there may be several options for the drive
 *  strength.
 * Please consult the datasheet to see which pins support which drive strength
 *  values.
 *  \param bank PIO bank number.
 *  \param mask Each bit in the mask corresponds to a PIO line. Bits set to 1 in this mask will
 *  be modified. Bits set to 0 in this mask will not be modified.
 *  \param drive_strength Supported drive strength that needs to be set for the specified PIOs. Refer to
 *  \#pin_drive_strength_id .
 *  \return A 32 bit mask. If any bit in this mask is high then that PIO could not be set
 *  to use the given drive strength ID; note that no action will have been taken
 *  on any PIOs.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioSetDriveStrength32Bank(uint16 bank, uint32 mask, pin_drive_strength_id drive_strength);

/**
 *  \brief Read the drive strength setting of a particular PIO pin.
 *  \param pin  - Pin that requires a drive strength read.
 *  \return  - Supported drive strength configured for the specified pin.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
pin_drive_strength_id PioGetDriveStrength(uint16 pin);

/**
 *  \brief Determine which physically existing PIOs are currently not in use by firmware
 *  or VM apps. 
 *  \return Those PIOs which are available for use. b31 = PIO31 thru b0 = PIO0. A '1'
 *  indicates available.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetUnusedPins32(void );

/**
 *  \brief Determine which physically existing PIOs are currently not in use by firmware
 *  or VM apps. 
 *  \param bank PIO bank number.
 *  \return Those PIOs which are available for use. b31 = PIO31 thru b0 = PIO0. A '1'
 *  indicates available.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint32 PioGetUnusedPins32Bank(uint16 bank);

/**
 *  \brief Controls the state for each PIO line that will wake 
 * the chip from deep sleep. The PIO lines must already have 
 * been configured to wake the chip. 
 *  \param bank  The bank index of the PIO lines. 
 *  \param mask  Bitmask to select the PIO lines to update within the given bank. 
 *  \param value  Each bit corresponds to a PIO line in the selected bank. 
 * Setting a bit high will result in the chip waking when that 
 * line goes high, setting the bit low will cause the chip to 
 * wake when the line goes low. Default is wakeup when line goes high.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void PioSetWakeupStateBank(uint16 bank, uint32 mask, uint32 value);

/**
 *  \brief Allows the chip to deep sleep irrespective of the level of the PIO 
 * lines. 
 *  \param bank  The bank index of the PIO lines. 
 *  \param mask  Bitmask to select the PIO lines to update within the given bank. 
 *  \param value  Each bit corresponds to a PIO line in the selected bank. 
 * Traditionally BlueCore chips will stay out of deep sleep when 
 * any PIO line configured as an input is high. This behaviour can be 
 * modified with PioSetWakeupStateBank()
 * Setting a bit high will result in the chip waking when that 
 * line goes high, setting the bit low will cause the chip to 
 * wake when the line goes low. 
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void PioSetDeepSleepEitherLevelBank(uint16 bank, uint32 mask, uint32 value);
#endif /* TRAPSET_CORE */
#if TRAPSET_KALIMBA

/**
 *  \brief Find out which pins are under kalimba control. Note: Only some BlueCore
 *  variants have access to more than 16 PIO lines.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint32 PioGetKalimbaControl32(void );

/**
 *  \brief Modify which pins are under control of the Kalimba DSP. 
 *   Aspects the DSP has control over include the direction (input or output)
 *   of a PIO, and the level driven when used as an output.
 *   
 *   
 *  \param mask The bit mask to use. 
 *  \param value The pins to set.
 *  \return A 32 bit mask. If any bit in that mask is high then control of that PIO could
 *  not be given to Kalimba and the trap call failed. Note: Only some BlueCore
 *  variants have access to more than 16 PIO lines.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_kalimba
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint32 PioSetKalimbaControl32(uint32 mask, uint32 value);
#endif /* TRAPSET_KALIMBA */

/**
 *  \brief Grabs PIOs for use by the LCD block. 
 *  \param mask These are the required LCD segments. b0-PIO0, b31=PIO31. 
 *  \param pins These should be set to one. 
 *  \param common_pin The PIO that will be connected to the LCD common. 
 *  \return zero if successful, otherwise bad bits returned and nothing done.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset___special_inline
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint32 PioSetLcdPins(uint32 mask, uint32 pins, uint16 common_pin);
#endif
