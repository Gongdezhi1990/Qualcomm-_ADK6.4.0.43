/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       thermistor.c
\brief      Support for thermistor temperature sensing, implementing the API defined
            in av_headset_temperature_sensor.h.
*/

#ifdef INCLUDE_TEMPERATURE
#ifdef HAVE_THERMISTOR
#include <panic.h>
#include <pio.h>
#include <hydra_macros.h>

#include "../av_headset_log.h"
#include "../av_headset_config.h"
#include "../av_headset_temperature_sensor.h"
#include "thermistor.h"

#include CSR_EXPAND_AND_STRINGIFY(THERMISTOR_DATA_FILE)

/*! \brief Returns the PIO bank number.
    \param pio The pio. */
#define PIO2BANK(pio) ((uint16)((pio) / 32))
/*! \brief Returns the PIO bit position mask within a bank.
    \param pio The pio. */
#define PIO2MASK(pio) (1UL << ((pio) % 32))

/*! Clamp a temperature within the limits specified in the THERMISTOR_DATA_FILE */
static int8 temperatureClamp(int8 temperature)
{
    temperature = MIN(temperature, THERMISTOR_TEMP_MAX);
    temperature = MAX(temperature, THERMISTOR_TEMP_MIN);
    return temperature;
}

uint16 appThermistorDegreesCelsiusToMillivolts(int8 temperature)
{
    temperature = temperatureClamp(temperature);
    return thermistor_voltage_lookup_table[temperature - THERMISTOR_TEMP_MIN];
}

/*! \brief Convert a thermistor milli-voltage to temperature in degrees Celsius.
    \param thermistor_mv The latest thermistor milli-voltage measurement.
    \param temperature_prev The previous temperature result.
    \note The temperature lookup table is ordered from low temperature to high
    temperature, which is from high voltage to low voltage. The conversion uses
    the previous temperature measurement as a starting index in the table search.
    Since temperature is expected to change slowly this will speed up the table
    search. The search results in a rounding up in temperature.
*/
static int8 appThermistorMillivoltsToDegreesCelsius(uint16 thermistor_mv, int8 temperature_prev)
{
    uint32 prev_index, i;
    uint16 prev_mv, this_mv;

    temperature_prev = temperatureClamp(temperature_prev);

    prev_index = temperature_prev - THERMISTOR_TEMP_MIN;
    prev_mv = thermistor_voltage_lookup_table[prev_index];

    if (thermistor_mv < prev_mv)
    {
        /* Temperature increase, search forward through lower voltages / higher temperatures */
        for (i = prev_index + 1; i < ARRAY_DIM(thermistor_voltage_lookup_table); i++)
        {
            this_mv = thermistor_voltage_lookup_table[i];
#ifdef THERMISTOR_DEBUG_VERBOSE
            DEBUG_LOGF("appThermistorMillivoltsToDegreesCelsius, search forwards i=%d, thermistor_mv=%d, this_mv=%d",
                       i, thermistor_mv, this_mv);
#endif
            if (thermistor_mv >= this_mv)
            {
                return THERMISTOR_TEMP_MIN + i;
            }
        }
        return THERMISTOR_TEMP_MAX;
    }
    else /* thermistor_mv >= prev_mv */
    {
        /* Temperature decrease, search backwards through higher voltages / lower temperatures */
        for (i = prev_index; i > 0; i--)
        {
            this_mv = prev_mv;
            prev_mv = thermistor_voltage_lookup_table[i-1];
#ifdef THERMISTOR_DEBUG_VERBOSE
            DEBUG_LOGF("appThermistorMillivoltsToDegreesCelsius, search backwards i=%d, thermistor_mv=%d, this_mv=%d, prev_mv=%d",
                       i, thermistor_mv, this_mv, prev_mv);
#endif
            if (thermistor_mv >= this_mv && thermistor_mv < prev_mv)
            {
                return THERMISTOR_TEMP_MIN + i;
            }
        }
        return THERMISTOR_TEMP_MIN;
    }
}

/*! \brief Handle the new result, calculating the new thermistor voltage.
    \param result The new ADC measurement.
    \param temperature [IN,OUT] Input: the last measured temperature. Output: the new
    temperature.
    \return TRUE if a new thermistor voltage was calculated, FALSE otherwise. */
static bool appThermistorHandleMeasurement(const MessageAdcResult* result, int8 *temperature)
{
    /*! The latest vref measurement, which is required to calculate the thermistor voltage.
        vref must be measured before the thermistor ADC voltage. */
    static uint16 vref_raw;

    uint16 reading = result->reading;

    if (adcsel_vref_hq_buff == result->adc_source)
    {
        vref_raw = reading;
    }
    else if (appConfigThermistor()->adc == result->adc_source)
    {
        uint32 vref_const = VmReadVrefConstant();
        uint16 thermistor_mv = (uint16)(vref_const * reading / vref_raw);
        int8 celsius = appThermistorMillivoltsToDegreesCelsius(thermistor_mv, *temperature);
#ifdef THERMISTOR_DEBUG_VERBOSE
        DEBUG_LOGF("appThermistorHandleMeasurement, %u, %u, %u, %d", vref_const, vref_raw, thermistor_mv, celsius);
#endif
        *temperature = celsius;
        return TRUE;
    }
    else
    {
        DEBUG_LOGF("appThermistorHandleMeasurement unexpected source - %d",result->adc_source);
    }

    return FALSE;
}

/*! \brief Enable/disable the thermistor 'on' PIO */
static void appThermistorPIOEnable(bool enable)
{
    uint8 on = appConfigThermistor()->on;
    if (on != THERMISTOR_PIO_UNUSED)
    {
        uint16 bank = PIO2BANK(on);
        uint32 mask = PIO2MASK(on);
        PanicNotZero(PioSet32Bank(bank, mask, enable ? mask : 0));
    }
}

void appTemperatureSensorRequestMeasurement(Task task)
{
    appThermistorPIOEnable(TRUE);
    AdcReadRequest(task, adcsel_vref_hq_buff, 0, 0);
    AdcReadRequest(task, appConfigThermistor()->adc, 0, 0);
}

bool appTemperatureSensorHandleMessage(Task task, MessageId id, Message message, int8 *temperature)
{
    UNUSED(task);
    switch (id)
    {
        case MESSAGE_ADC_RESULT:
            if (appThermistorHandleMeasurement((const MessageAdcResult*)message, temperature))
            {
                appThermistorPIOEnable(FALSE);
                return TRUE;
            }
        break;

        default:
            DEBUG_LOGF("appThermistorHandleMessage unexpected message - %u", id);
            Panic();
        break;
    }
    return FALSE;
}

void appTemperatureSensorInit(void)
{
    uint8 on = appConfigThermistor()->on;

    if (on != THERMISTOR_PIO_UNUSED)
    {
        /* Setup 'on' PIO as output (driving low) */
        uint16 bank = PIO2BANK(on);
        uint32 mask = PIO2MASK(on);
        PanicNotZero(PioSetMapPins32Bank(bank, mask, mask));
        PanicNotZero(PioSetDir32Bank(bank, mask, mask));
        PanicNotZero(PioSet32Bank(bank, mask, 0));
    }
}

#endif /* HAVE_THERMISTOR */
#endif /* INCLUDE_TEMPERATURE */
