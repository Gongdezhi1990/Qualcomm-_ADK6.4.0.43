#ifndef __CAPACITIVESENSOR_H__
#define __CAPACITIVESENSOR_H__
#include <app/capacitive_sensor/capacitive_sensor_if.h>



#if TRAPSET_CAPACITIVE_SENSOR

/**
 *  \brief Get the current status of the capacitive touch pads. 
 *     The current state of each of the pads will be written to the
 *     corresponding element of the results array.
 *  \param count The number of pad states to read. 
 *  \param pads count values each specifying a pad. 
 *  \param results count values to be written to. 
 *  \return TRUE if successful, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_capacitive_sensor
 */
bool CapacitiveSensorPadQuery(uint16 count, const uint16* pads, capacitive_sensor_pad_state * results);

/**
 *  \brief Configure a set of CapacitiveSensor channels
 *         This call enables the common parameters of a set of CapacitiveSensor
 *  channels 
 *         (perhaps all of them) to be set in the hardware.
 *        
 *  \param count Number of pads to be configured
 *  \param pads List of \c count pads to be configured
 *  \param common_config Pointer to a single \c CAPACITIVE_SENSOR_PAD_CONFIG instance
 *  \return Returns TRUE if configuration was successful
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_capacitive_sensor
 */
bool CapacitiveSensorConfigureMultiPad(uint16 count, const uint16 * pads, const CAPACITIVE_SENSOR_PAD_CONFIG * common_config);

/**
 *  \brief Enable the supplied set of CapacitiveSensor pads
 *        To enable a channel, it is necessary to supply a "preload" value, which
 *        should be as close as possible to the "natural" capacitance of the pad,
 *        plus on- and off-trigger levels, which contain the change in the 
 *        measured capacitance (i.e. current level - background level) to indicate 
 *        respectively a touch-on when there is currently no detected touch, and
 *        a touch-off when there is currently a detected touch.
 *        PIO setup is not handled by this trap but can be done at any point.
 *        XIO pads need to be configured as analogue pins. This is done by using
 *        PioSetMapPins32Bank to put the PIO under HW control and then
 *        PioSetFunction to set the function to XIO_ANALOGUE.
 *        
 *  \param count Number of pads to be enabled
 *  \param pads List of \c count pads to be enabled
 *  \param pad_levels Array of \c count \c CAPACITIVE_SENSOR_PAD_LEVELS, containing the
 *          preload and trigger levels for each pad to be enabled
 *  \return Returns TRUE if enabling was successful
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_capacitive_sensor
 */
bool CapacitiveSensorEnablePads(uint16 count, const uint16 * pads, const CAPACITIVE_SENSOR_PAD_LEVELS * pad_levels);

/**
 *  \brief Configure the touch offset for a CapacitiveSensor channel.
 *         This call sets the touch offset for a specific CapacitiveSensor
 *  channel. This
 *         is meant to be used by the APP to take out the capacitance added by a
 *         touch to avoid the situation where a long touch eventually becomes part
 *         of the background from the point of view of the background capacitance
 *         tracker.
 *         Please note that this value is subtracted from the reading.
 *        
 *  \param pad Pad to be configured
 *  \param touch_offset Touch offset value.
 *  \return Returns TRUE if configuration was successful
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_capacitive_sensor
 */
bool CapacitiveSensorConfigureTouchOffset(uint16 pad, uint32 touch_offset);

/**
 *  \brief Enable/Disable CapacitiveSensor shield on XIO11.
 *         Enabling the shield causes the capacitance seen on XIO11 to be
 *         subtracted from the capacitance measured on all the other XIOs. This is
 *         done in hardware prior to sampling thus preserving ADC range. The
 *         purpose for this is to remove any stray capacitance from the other XIOs.
 *         Please note that XIO11 should not be used for any other operations when
 *         the shield is enabled. Also, XIO11 should be set up in analogue mode
 *         prior to enabling the shield.
 *        
 *  \param enable Enable or disable the shield.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_capacitive_sensor
 */
void CapacitiveSensorShieldEnable(bool enable);
#endif /* TRAPSET_CAPACITIVE_SENSOR */
#endif
