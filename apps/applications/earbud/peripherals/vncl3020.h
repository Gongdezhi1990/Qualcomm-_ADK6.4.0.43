/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       vncl3020.h
\brief      Header file for vncl3020 Accelerometer
*/

#ifndef VNCL3020_PRIVATE_H
#define VNCL3020_PRIVATE_H

#ifdef HAVE_VNCL3020

#include <types.h>

/*! I2C communication addresses */
#define I2C_ADDRESS                (0x13)

/*! @name VNCL3020 I2C registers. */
//!@{
#define VNCL3020_COMMAND_REG                    (0x80)
#define VNCL3020_PRODCUT_ID_REVISION_REG        (0x81)
#define VNCL3020_PROXIMITY_RATE_REG             (0x82)
#define VNCL3020_IR_LED_CURRENT_REG             (0x83)
#define VNCL3020_PROXIMITY_RESULT_HI_REG        (0x87)
#define VNCL3020_PROXIMITY_RESULT_LO_REG        (0x88)
#define VNCL3020_INTERRUPT_CONTORL_REG          (0x89)
#define VNCL3020_LOW_THRESHOLD_HI_REG           (0x8a)
#define VNCL3020_LOW_THRESHOLD_LO_REG           (0x8b)
#define VNCL3020_HIGH_THRESHOLD_HI_REG          (0x8c)
#define VNCL3020_HIGH_THRESHOLD_LO_REG          (0x8d)
#define VNCL3020_INTERRUPT_STATUS_REG           (0x8e)
#define VNCL3020_PROXIMITY_MOD_TIMING_ADJ_REG   (0x8f)
//!@}

/*! Important note, this code assumes the compiler fills bitfields from the
    least significant bit, as KCC does, such that the following register
    bitfield definitions correctly map onto the physical bits */
#ifndef __KCC__
#warning "Not compiling with KCC. The bitfield register definitions may be incorrect."
#endif

/*! This register is for starting proximity measurements */
typedef union vncl3020_command_register
{
    /*! The register bit fields */
    struct
    {
        /*! R/W bit. Enables state machine and LP oscillator for self timed
            measurements; no measurement is performed until the corresponding bit
            is set */
        unsigned selftimed_en : 1;
        /*! R/W bit. Enables periodic proximity measurement */
        unsigned prox_en : 1;
        /*! Function of this bit is not defined */
        unsigned _unused : 1;
        /*! R/W bit. Starts a single on-demand measurement for proximity.
            Result is available at the end of conversion for reading in the
            registers #7(HB) and #8(LB). */
        unsigned prox_od : 1;
        /*! Function of this bit is not defined */
        unsigned __unused : 1;
        /*! Read only bit. Value = 1 when proximity measurement data is available
            in the result registers. This bit will be reset when one of the
            corresponding result registers is read. */
        unsigned prox_data_rdy : 1;
        /*! Function of this bit is not defined */
        unsigned ___unused : 1;
        /*! Read only bit. Value = 1 */
        unsigned config_lock : 1;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} vncl3020_command_register_t;

/*! This register contains information about product ID and product revision */
typedef union vncl3020_revision_register
{
    /*! The register bit fields */
    struct
    {
        /*! Read only bits. Value = 1 */
        unsigned revision_id : 4;
        /*! Read only bits. Value = 2 */
        unsigned product_id : 4;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} vncl3020_revision_register_t;

/*! Proximity rate control */
enum vncl3020_proximity_rates
{
    vncl3020_proximity_rate_1p95_per_second = 0,
    vncl3020_proximity_rate_3p90625_per_second,
    vncl3020_proximity_rate_7p8125_per_second,
    vncl3020_proximity_rate_16p625_per_second,
    vncl3020_proximity_rate_31p25_per_second,
    vncl3020_proximity_rate_62p5_per_second,
    vncl3020_proximity_rate_125_per_second,
    vncl3020_proximity_rate_250_per_second,
};

/*! This register controls the proximity rate */
typedef union vncl3020_proximity_rate_register
{
    /*! The register bit fields */
    struct
    {
        /*! The measurement rate */
        enum vncl3020_proximity_rates rate : 3;
        /*! Function of this bit is not defined */
        unsigned _unused : 5;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} vncl3020_proximity_rate_register_t;

/*! This register controls the IR LED current */
typedef union vncl3020_ir_led_current_register
{
    /*! The register bit fields */
    struct
    {
        /*! The IR led current = current x 10mA (2=2mA=default) */
        unsigned current : 6;
        /*! Read only bits. Information about fuse program revision used for
            initial setup/calibration of the device. */
        unsigned fuse_prog_id : 2;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} vncl3020_ir_led_current_register_t;

/*! The number of times the threshold is exceeded before generating an interrupt */
enum vncl_threshold_counts
{
    vncl3020_threshold_count_1 = 0,
    vncl3020_threshold_count_2,
    vncl3020_threshold_count_4,
    vncl3020_threshold_count_8,
    vncl3020_threshold_count_16,
    vncl3020_threshold_count_32,
    vncl3020_threshold_count_64,
    vncl3020_threshold_count_128,
};

/*! This register controls the interrupt */
typedef union vncl3020_interrupt_control_register
{
    /*! The register bit fields */
    struct
    {
        /*! R/W bit. The threshold to applied to proximity interrupt */
        unsigned threshold_sel : 1;
        /*! R/W bit. Enables interrupt generation when high or low threshold is
            exceeded */
        unsigned threshold_en : 1;
        /*! Function of this bit is not defined */
        unsigned _unused : 1;
        /*! R/W bit. Enables interrupt generation at proximity data ready */
        unsigned proximity_ready_en : 1;
        /*! Function of this bit is not defined */
        unsigned __unused : 1;
        /*! R/W bits. These bits contain the number of consecutive measurements
            needed above/below the threshold. */
        enum vncl_threshold_counts count_exceeded : 3;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} vncl3020_interrupt_control_register_t;

/*! The interrupt status register. Once an interrupt is generated the
    corresponding status bit goes to 1 and stays there unless it is cleared by
    writing a 1 in the corresponding bit. The int pad will be pulled down while
    at least one of the status bit is 1 */
typedef union vncl3020_interrupt_status_register
{
    /*! The register bit fields */
    struct
    {
        /*! R/W bit. Indicates the high threshold is exceeded */
        unsigned threshold_high : 1;
        /*! R/W bit. Indicates the low threshold is exceeded */
        unsigned threshold_low : 1;
        /*! Function of this bit is not defined */
        unsigned _unused : 1;
        /*! R/W bit. Indicates a generated interrupt for proximity */
        unsigned prox_ready : 1;
        /*! Function of this bit is not defined */
        unsigned __unused : 4;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} vncl3020_interrupt_status_register_t;

/*! The 'on' PIO will not be controlled when __proximity_config.pio.on is set to
    this value */
#define VNCL3020_ON_PIO_UNUSED 255

/*! The high level configuration for taking measurement */
struct __proximity_config
{
    /*! Measurements higher than this value will result in the sensor considering
        it is in-proximity of an object */
    uint16 threshold_high;
    /*! Measurements lower than this value will result in the sensor considering
        it is not in-proximity of an object */
    uint16 threshold_low;
    /*! The I2C clock frequency */
    uint16 i2c_clock_khz;
    /*! The number of measurements above/below the threshold before the sensor
        generates an interrupt */
    enum vncl_threshold_counts threshold_counts;
    /*! The number of measurements per second */
    enum vncl3020_proximity_rates rate;
    /*! The PIOs used to control/communicate with the sensor */
    struct
    {
        /*! PIO used to power-on the sensor, or #VNCL3020_ON_PIO_UNUSED */
        uint8 on;
        /*! Interrupt PIO driven by the sensor */
        uint8 interrupt;
        /*! I2C serial data PIO */
        uint8 i2c_sda;
        /*! I2C serial clock PIO */
        uint8 i2c_scl;
    } pios;
};

/*! Internal representation of proximity state */
enum proximity_states
{
    proximity_state_unknown,
    proximity_state_in_proximity,
    proximity_state_not_in_proximity
};

/*! Trivial state for storing in-proximity state */
struct __proximity_state
{
    /*! The sensor proximity state */
    enum proximity_states proximity;
};

#endif /* HAVE_VNCL3020 */
#endif /* VNCL3020_PRIVATE_H */
