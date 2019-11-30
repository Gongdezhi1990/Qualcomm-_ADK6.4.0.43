/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      Header file for thermistor
*/

#ifndef THERMISTOR_H
#define THERMISTOR_H

#ifdef HAVE_THERMISTOR

#include <types.h>
#include <adc.h>

/*! Used to indicate a PIO in the config structure is unused */
#define THERMISTOR_PIO_UNUSED 255

/*! The thermistor configuration */
typedef struct
{
    /*! PIO used to drive the high side of the thermistor potential divider
    or THERMISTOR_PIO_UNUSED */
    uint8 on;

    /*! On chip ADC source from which the thermistor voltage is read */
    vm_adc_source_type adc;

} thermistorConfig;

/*! \brief Returns the expected thermistor voltage at a specified temperature.
    \param temperature The specified temperature in degrees Celsius.
    \return The equivalent milli-voltage.
    \note For test purposes only.
*/
uint16 appThermistorDegreesCelsiusToMillivolts(int8 temperature);

#endif /* HAVE_THERMISTOR */
#endif /* THERMISTOR_H */
