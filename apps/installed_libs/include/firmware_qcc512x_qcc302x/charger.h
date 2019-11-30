#ifndef __CHARGER_H__
#define __CHARGER_H__
#include <app/charger/charger_if.h>

/*! file  @brief Configuration of the onchip battery charger present on some BlueCore variants */

#if TRAPSET_CHARGER2

/**
 *  \brief Returns the state of the battery recorded at boot time.
 *     
 *  \return the status of the battery at boot, as described in \#charger_battery_status
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_charger2
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
charger_battery_status ChargerGetBatteryStatusAtBoot(void );
#endif /* TRAPSET_CHARGER2 */
#if TRAPSET_CHARGER

/**
 *  \brief Configuration of the onchip battery charger present on some chip variants 
 *         Charger parameters can be changed at any time but normally
 *         will be applied only with the charger enable command.
 *         Application shall ensure the charger is disabled,
 *         configure all parameters and then enable the charger by setting
 *         CHARGER_ENABLE key to "1".
 *         
 *         The only exception are the following keys: CHARGER_TRICKLE_CURRENT,
 *         CHARGER_PRE_CURRENT, CHARGER_FAST_CURRENT. These can be set even
 *         when the charger is already enabled and are applied immediately.
 *         Note: enabling the charger when there is no input power increases
 *         power consumption and is not recommended. Instead, application
 *         should wait for MESSAGE_CHARGER_DETECTED and enable or disable
 *         the charger depending on the charger type detected.
 *         
 *         Consult the datasheet for your chip variant and the documentation for
 *  the
 *         \#charger_config_key type for more information.
 *  \param key Which aspect of the charger hardware to configure 
 *  \param value Which value to use
 *  \return         TRUE if the key parameter is valid, else FALSE. 
 *         If parameter value is wrong, application will receive
 *         MESSAGE_CHARGER_STATUS with an error when parameter is applied. 
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_charger
 */
bool ChargerConfigure(charger_config_key key, uint16 value);

/**
 *  \brief Get the status of the charging hardware.
 *   Reports information on the state of the on chip battery charger.
 *   This is only supported on chip variants with charger hardware.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_charger
 */
charger_status ChargerStatus(void );
#endif /* TRAPSET_CHARGER */
#if TRAPSET_CHARGERMESSAGE

/**
 *  \brief Enables charger events and configures their debouncing. 
 *     A \#MESSAGE_CHARGER_CHANGED message will be delivered to the task setup by
 *  MessageChargerTask()
 *     when the debounced state of the charger pins has changed.
 *     For example calling ChargerDebounce((CHARGER_VREG_EVENT |
 *  CHARGER_CONNECT_EVENT), 4, 1000)
 *     will enable the VREGEN_H and the charger attach/detach events. The
 *  application will
 *     receive a \#MESSAGE_CHARGER_CHANGED message after the charger pins value has
 *  changed and then 
 *     remained stable for 4 consecutive reads 1000 milliseconds apart. 
 *     For CSR8670 or CSR8670-like chips this same call would also enable a
 *  \#MESSAGE_CHARGER_CHANGED
 *     message to be received when VREG_EN has been detected pressed for four
 *  consecutive readings,
 *     each 1000mS apart.
 *     
 *     In ADK6 firmware, CHARGER_CONNECT_EVENT Debounce configuration through this
 *  trap will not be supported.
 *     \#MESSAGE_CHARGER_CHANGED message will be received when charger has been
 *  detected 
 *     using the ChargerInsertionDebounceTime and ChargerRemovalDebounceTime. 
 *  \param events_to_enable Each bit position enables a charger event. See the \#charger_events enum for
 *  details of the events. 
 *  \param count The number of times to read from the charger pins before notifying the
 *  application 
 *  \param period The delay in milliseconds between reads.
 *  \return A charger_events bit mask where any bit set high indicates an invalid event bit
 *  passed in the "events_to_enable" parameter. Any invalid event will result in
 *  the ChargerDebounce() request being rejected. Returns 0 on success.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_chargermessage
 */
charger_events ChargerDebounce(charger_events events_to_enable, uint16 count, uint16 period);
#endif /* TRAPSET_CHARGERMESSAGE */
#endif
