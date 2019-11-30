/* Copyright (c) 2016, 2019 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        adc_if.h  -  ADC interface
 
CONTAINS
        The GLOBAL definitions for the ADC subsystem from the VM
 
DESCRIPTION
        This file is seen by the stack, and VM applications, and
        contains things that are common between them.

*/

#ifndef __APP_ADC_IF_H__
#define __APP_ADC_IF_H__

/** Maximum number of concurrent requests that can be scheduled from
 * the application with AdcReadRequest() trap. */
#define MAX_ADC_READ_REQUESTS 30

/*! 
   @brief Current source for AdcReadRequest
*/
typedef enum
{
    /*! Standard measurement, no current source enabled. */
    ADC_STANDARD = 0,
    /*! Before measuring, enable the internal current source on the same pin we
        need to measure from. */
    ADC_INTERNAL_CURRENT_SOURCE_EN = 1,
    /*! Before measuring, enable the internal current source on LED0. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED0 = 2,
    /*! Before measuring, enable the internal current source on LED1. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED1 = 3,
    /*! Before measuring, enable the internal current source on LED2. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED2 = 4,
    /*! Before measuring, enable the internal current source on LED3. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED3 = 5,
    /*! Before measuring, enable the internal current source on LED4. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED4 = 6,
    /*! Before measuring, enable the internal current source on LED5. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED5 = 7,
    /*! Before measuring, enable the internal current source on LED6. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_LED6 = 8,
    /*! Before measuring, enable the internal current source on XIO0. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO0 = 9,
    /*! Before measuring, enable the internal current source on XIO1. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO1 = 10,
    /*! Before measuring, enable the internal current source on XIO2. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO2 = 11,
    /*! Before measuring, enable the internal current source on XIO3. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO3 = 12,
    /*! Before measuring, enable the internal current source on XIO4. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO4 = 13,
    /*! Before measuring, enable the internal current source on XIO5. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO5 = 14,
    /*! Before measuring, enable the internal current source on XIO6. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO6 = 15,
    /*! Before measuring, enable the internal current source on XIO7. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO7 = 16,
    /*! Before measuring, enable the internal current source on XIO8. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO8 = 17,
    /*! Before measuring, enable the internal current source on XIO9. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO9 = 18,
    /*! Before measuring, enable the internal current source on XIO10. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO10 = 19,
    /*! Before measuring, enable the internal current source on XIO11. */
    ADC_INTERNAL_CURRENT_SOURCE_EN_XIO11 = 20
} vm_adc_extra_flag;

/** Helper macro to convert a VM ADC extra flag into a VM ACD extra flag index */
#define vm_extra_flag_to_idx(x) \
    ((uint16)(x) - (uint16)ADC_STANDARD)
/** Helper macro to convert a VM ACD extra flag index into a VM ADC extra flag */
#define vm_extra_idx_to_flag(x) \
    (vm_adc_extra_flag)((x) + (uint16)ADC_STANDARD)

enum adcsel_enum {
        /*! Test pin AIO0.  Read with Apps and BCCMD. 
            Not available in QCCxxxx devices */
        adcsel_aio0,

        /*! Test pin AIO1.  Read with Apps and BCCMD. 
            Not available in QCCxxxx devices */
        adcsel_aio1,

        /*! Test pin AIO2.  Read with Apps and BCCMD.
            Not available on BC7 or BC7-derived chips, e.g.
            CSR8670, QCCxxxx */
        adcsel_aio2,

        /*! Test pin AIO3.  Read with Apps and BCCMD.
            Not available on BC7 or BC7-derived chips, e.g.
            CSR8670, QCCxxxx */
        adcsel_aio3,

        /*! The internal reference voltage in the chip.
            Read with Apps and BCCMD.
            Not available in QCCxxxx devices */
        adcsel_vref,

        /*! Battery voltage at output of the charger (CHG_VBAT).
            Read with Apps and BCCMD. 
            Not available in QCCxxxx devices */
        adcsel_vdd_bat,

        /*! Input to bypass regulator (VCHG).
            Read with Apps and BCCMD. 
            Not available in QCCxxxx devices */
        adcsel_byp_vregin,

        /*! Battery voltage sense (CHG_VBAT_SENSE).
            Read with Apps and BCCMD. 
            Not available in QCCxxxx devices */
        adcsel_vdd_sense,

        /*! VREG_ENABLE voltage. Read with Apps and BCCMD. 
            Not available in QCCxxxx devices */
        adcsel_vregen,

        /*! USB charger detection sensor. This is available only in
            CSRA681xx device, Please consult the device datasheet */
        adcsel_usb_hst_datp,

        /*! USB charger detection sensor. This is available only in
            CSRA681xx device, Please consult the device datasheet */
        adcsel_usb_hst_datm,

        /*! Boot Ref power supply, Max input voltage ~7V (VCharger), scaled by 6 */
        adcsel_boot_ref,

        /*! Charger current monitor */
        adcsel_chg_mon,

        /*! Charger voltage monitor */
        adcsel_chg_mon_vref,

        /*! PMU VBat Sense, Max input voltage ~4.8V (Li battery) scaled by 4*/
        adcsel_pmu_vbat_sns,

        /*! PMU VCharge Sense, Max input voltage ~7V, scaled by 6*/
        adcsel_pmu_vchg_sns,

        /*! Charger voltage monitor, Provides better accuracy than adcsel_chg_mon_vref */
        adcsel_vref_hq_buff,

        /*! LED pads used by Apps to measure the battery 
            temperature and/or provide constant current */ 
        adcsel_led0,
        adcsel_led1,
        adcsel_led2,
        adcsel_led3,
        adcsel_led4,
        adcsel_led5,
        /*! adcsel_led6 cannot be used in some devices. Please
            consult the device datasheet */
        adcsel_led6,

        /*! XIOs used by Apps to measure the battery temperature
            and/or provide constant current. Furthermore,
            capacitive sensing is only possible on XIO pads.
            XIOs not used for any of the above can be used by
            customers to do arbitrary ADC measurements. */
        adcsel_xio0,
        adcsel_xio1,
        adcsel_xio2,
        adcsel_xio3,
        adcsel_xio4,
        adcsel_xio5,
        adcsel_xio6,
        adcsel_xio7,
        adcsel_xio8,
        adcsel_xio9,
        adcsel_xio10,
        adcsel_xio11,
        /*! XIO12 is reserved for BIST ADC DAC output. */
        adcsel_xio12

};
typedef enum adcsel_enum vm_adc_source_type;

/** Helper macro to convert a VM ADC internal source into a VM ACD internal source index */
#define vm_int_src_to_idx(x) \
    ((uint16)(x) - (uint16)adcsel_usb_hst_datp)
/** Helper macro to convert a VM ACD internal source index into a VM ADC internal source */
#define vm_int_idx_to_src(x) \
    (vm_adc_source_type)((x) + (uint16)adcsel_usb_hst_datp)

/** Helper macro to convert a VM ADC LED source into a VM ACD LED source index */
#define vm_led_src_to_idx(x) \
    ((uint16)(x) - (uint16)adcsel_led0)
/** Helper macro to convert a VM ACD LED source index into a VM ADC LED source */
#define vm_led_idx_to_src(x) \
    (vm_adc_source_type)((x) + (uint16)adcsel_led0)

/** Helper macro to convert a VM ADC XIO source into a VM ACD XIO source index */
#define vm_xio_src_to_idx(x) \
    ((uint16)(x) - (uint16)adcsel_xio0)
/** Helper macro to convert a VM ACD XIO source index into a VM ADC XIO source */
#define vm_xio_idx_to_src(x) \
    (vm_adc_source_type)((x) + (uint16)adcsel_xio0)

#endif /* ifndef __APP_ADC_IF_H__ */
