#ifndef __ADC_H__
#define __ADC_H__
#include <message_.h>
#include <app/vm/vm_if.h>
#include <app/adc/adc_if.h>

/*! file  @brief Access to ADC hardware.
** 
**  
BlueCore has an analogue-to-digital converter (ADC) which can be
used to read the  voltage at a number of locations, such as:
- At AIO pins \#adcsel_aio0 to \#adcsel_aio3.
- From \#adcsel_vref - the internal voltage in the chip.
- From \#adcsel_vdd_bat - the battery.
.
(Note that this hardware is distinct from the ADCs that form part
of the analogue audio codec.)
**    
Note that not all sources are available on all BlueCores.
**    
The voltage (in mV) from a source is given by:
**
(reading) * VrefConstant / (vref_reading) 
**
\a reading comes from the \#MESSAGE_ADC_RESULT for the source in
question. \a vref_reading is a corresponding reading for
\#adcsel_vref that has been taken relatively recently.
VmReadVrefConstant() should be used to determine \a VrefConstant.
This calculation determines how many mV each bit of \a reading 
corresponds to, correcting for natural variation and systematic
errors in the ADC reading process.
**
For example, on CSR8670 VmReadVrefConstant() returns 700 (which is in
mV, and therefore represents 0.7 V). A reading of \#adcsel_vref might 
return 531 (this number will vary slightly with process and 
temperature). In which case, 1 bit of ADC reading corresponds 
to a real value of 700/531 mV. Hence, if a reading of 100 is 
obtained from \#adcsel_aio0, this corresponds to a true voltage of
**
100 * 700/531 = 132 mV on AIO0.
**
The underlying hardware has 8 bits of resolution on older BlueCores,
and 10 bits of resolution from BlueCore 5 onwards. The raw reading
may be conditioned by the firmware before being returned to the
application.
*/

#if TRAPSET_CORE

/**
 *  \brief Send a request to the ADC to make a reading from adc_source. When the reading
 *  is made, a \#MESSAGE_ADC_RESULT message will be sent to the task passed. See
 *  adc.h for how to interpret the result.
 *     Note that this trap replaces trap AdcRequest(). This is because it has to
 *  support queuing multiple request
 * calls with different task, ADC source, extra flag and delay parameters.
 *     So far the extra_flag parameter only supports enabling/disabling the 10uA
 *  internal current source from HQ_REF
 * during the reading.
 *     Adc readings for various analogue sources may require a delay to let
 *  external circuitry settle and not impact
 * the reading (for example external capacitors need time to charge when the
 *  internal current source is enabled).
 *     This delay needs to be provided by customer where they see appropriate.
 *     PIO setup is not handled by this trap but can be done at any point. XIO
 *  pads need to be configured as analogue pins.
 * This is done by using PioSetMapPins32Bank to put the PIO under HW control and
 *  then PioSetFunction to set the function to
 * XIO_ANALOGUE. LED pads are handled internally by the Curator so we just need to
 *  make sure those pins are not used by
 * something else. This is done by using PioSetMapPins32Bank to put the PIO under
 *  HW control and then PioSetFunction to set
 * the function to OTHER.
 *     Application can't schedule more than \#MAX_ADC_READ_REQUESTS concurrent
 *  requests.
 *     If this limit is reached, the trap will start returning FALSE until some of
 *  the
 *     pending requests complete. Application shall monitor number of
 *     \#MESSAGE_ADC_RESULT messages received and balance number of submitted
 *     requests against it.
 *  \param task The task which will receive the ADC result message. 
 *  \param adc_source The source (\#vm_adc_source_type) to take readings from. 
 *  \param extra_flag The flags (\#vm_adc_extra_flag) to be used for readings
 *  \param delay Delay in millisconds to use for internal current source
 *  \return TRUE if request is accepted, else FALSE.
 *           Request is not accepted if either \#adc_source or \#extra_flag
 *           is not correct or if there are \#MAX_ADC_READ_REQUESTS pending
 *           requests already.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
bool AdcReadRequest(Task task, vm_adc_source_type adc_source, vm_adc_extra_flag extra_flag, uint16 delay);
#endif /* TRAPSET_CORE */
#endif
