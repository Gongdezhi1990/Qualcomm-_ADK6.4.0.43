#ifndef __VOLTSENSE_H__
#define __VOLTSENSE_H__
/** \file */
#if TRAPSET_XIO

/**
 *  \brief API to measure voltage on an XIO. 
 *     PIO setup is not handled by this trap but can be done at any point. XIO
 *  pads need to be configured as analogue pins.
 * This is done by using PioSetMapPins32Bank to put the PIO under HW control and
 *  then PioSetFunction to set the function to
 * XIO_ANALOGUE.
 *         
 *  \param task The task to which the message containing the voltage reading will be delivered
 *  \param first_xio xio or first xio in difference pair to use for input
 *  \param second_xio xio or second xio in difference pair to use for input
 *  \param voltage_gain VGA value, 0.85, 1, 2, 4, 8
 *  \return result, success, failure, failure with reason
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_xio
 */
trap_voltsense_result XioMeasureVoltage(Task task, xio_source first_xio, xio_source second_xio, vga_gain voltage_gain);

/**
 *  \brief Enables or disables the DAC
 *         
 *  \param enable TRUE if setting up the DAC or FALSE if releasing the DAC. 
 *  \return True if 'enable' or 'disable' has occurred.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_xio
 */
bool DacEnable(bool enable);

/**
 *  \brief Sets the DAC buffer gain to the value supplied. 
 *                      Note the DAC buffer gain will be either 1 or 0.9.
 *         
 *     PIO setup is not handled by this trap but can be done at any point. XIO 12
 *  needs to be configured as analogue pin.
 * This is done by using PioSetMapPins32Bank to put the PIO under HW control and
 *  then PioSetFunction to set the function to
 * XIO_ANALOGUE.
 *         
 *  \param gain gain, 0.9, 1
 *  \return result, success, or failure with reason
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_xio
 */
trap_voltsense_result DacSetGain(dac_buffer_gain gain);

/**
 *  \brief Sets the output level on the DAC to the value supplied. 
 *                      Note that the auxiliary DAC level drops to 0V when
 *  BlueCore enters
 *                      deep sleep for power-saving.
 *         
 *  \param level Required Level (10 bit digital value)
 *  \return result, success, or failure with reason
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_xio
 */
trap_voltsense_result DacSetLevel(uint16 level);
#endif /* TRAPSET_XIO */
#endif
