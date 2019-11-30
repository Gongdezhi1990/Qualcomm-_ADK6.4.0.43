/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       adxl362.h
\brief      Header file for adxl362 accelerometer
*/

#ifndef ADXL362_PRIVATE_H
#define ADXL362_PRIVATE_H

#ifdef HAVE_ADXL362

#include <types.h>

/*! @name ADXL362 SPI communication commands. */
//!@{
#define REG_WRITE_CMD              (0x0a)
#define REG_READ_CMD               (0x0b)
#define FIFO_READ_CMD              (0x0d)
//!@}

/*! @name ADXL362 SPI registers. */
//!@{
#define ADXL362_DEVID_AD_REG       (0x00)
#define ADXL362_DEVID_MST_REG      (0x01)
#define ADXL362_PARTID_REG         (0x02)
#define ADXL362_REVID_REG          (0x03)
#define ADXL362_XID_REG            (0x04)

#define ADXL362_XDATA8_REG         (0x08)
#define ADXL362_YDATA8_REG         (0x09)
#define ADXL362_ZDATA8_REG         (0x0A)
#define ADXL362_STATUS_REG         (0x0B)
#define ADXL362_FIFO_ENTRIES_L_REG (0x0C)
#define ADXL362_FIFO_ENTRIES_H_REG (0x0D)
#define ADXL362_XDATAL_REG         (0x0e)
#define ADXL362_XDATAH_REG         (0x0f)
#define ADXL362_YDATAL_REG         (0x10)
#define ADXL362_YDATAH_REG         (0x11)
#define ADXL362_ZDATAL_REG         (0x12)
#define ADXL362_ZDATAH_REG         (0x13)
#define ADXL362_TEMPL_REG          (0x14)
#define ADXL362_TEMPH_REG          (0x15)
#define ADXL362_X_ADCL_REG         (0x16)
#define ADXL362_X_ADCH_REG         (0x17)

#define ADXL362_SOFT_RESET_REG     (0x1f)
#define ADXL362_THRESH_ACTL_REG    (0x20)
#define ADXL362_THRESH_ACTH_REG    (0x21)
#define ADXL362_TIME_ACT_REG       (0x22)
#define ADXL362_THRESH_INACTL_REG  (0x23)
#define ADXL362_THRESH_INACTH_REG  (0x24)
#define ADXL362_TIME_INACTL_REG    (0x25)
#define ADXL362_TIME_INACTH_REG    (0x26)
#define ADXL362_ACT_INACT_CTL_REG  (0x27)
#define ADXL362_FIFO_CONTROL_REG   (0x28)
#define ADXL362_FIFO_SAMPLES_REG   (0x29)
#define ADXL362_INTMAP1_REG        (0x2a)
#define ADXL362_INTMAP2_REG        (0x2b)
#define ADXL362_FILTER_CTL_REG     (0x2c)
#define ADXL362_POWER_CTL_REG      (0x2d)
#define ADXL362_SELF_TEST_REG      (0x2e)
//!@}

/* Bit field definitions and register values */

/*! Important note, this code assumes the compiler fills bitfields from the
    least significant bit, as KCC does, such that the following register
    bitfield definitions correctly map onto the physical bits */
#ifndef __KCC__
#warning "Not compiling with KCC. The bitfield register definitions may be incorrect."
#endif

/*! Read only device status register */
typedef union adxl362_status_register
{
    /*! The register bit fields */
    struct
    {
        /*! 1 indicates that a new valid sample is available to be read.
            This bit clears when a FIFO read is performed.*/
        unsigned data_ready : 1;
        /*! 1 indicates that there is at least one sample available in the
            FIFO output buffer. */
        unsigned fifo_ready : 1;
        /*! 1 indicates that the FIFO contains at least the desired number of
            samples, as set in the FIFO_SAMPLES register. */
        unsigned fifo_watermark : 1;
        /*! 1 indicates that the FIFO has overrun or overflowed, such that new
            data replaces unread data. */
        unsigned fifo_overrun : 1;
        /*! 1 indicates that the activity detection function has detected an
            over threshold condition. */
        unsigned act : 1;
        /*! 1 indicates that the inactivity detection function has detected an
            inactivity or a free fall condition. */
        unsigned inact : 1;
        /*! Indicates whether the accelerometer is in an active (AWAKE = 1) or
            inactive (AWAKE = 0) state, based on the activity and inactivity
            functionality. To enable autosleep, activity and inactivity
            detection must be in linked mode or loop mode (LINK/LOOP bits in the
            ACT_INACT_CTL register); otherwise, this bit defaults to 1 and must
            be ignored. */
        unsigned awake : 1;
        /*! 1 indicates one of two conditions: either an SEU event, such as an
            alpha particle of a power glitch, has disturbed a user register
            setting or the ADXL362 is not configured. This bit is high upon
            both startup and soft reset, and resets as soon as any register
            write commands are performed. */
        unsigned err_user_regs : 1;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} adxl362_status_register_t;

/*! Read/write activity/inactivity control register */
typedef union adxl362_activity_control_register
{
    /*! The register bit fields */
    struct
    {
        /*! 1 = enables the activity (overthreshold) functionality. */
        unsigned act_en : 1;
        /*! 1 = activity detection function operates in referenced mode.
            0 = activity detection function operates in absolute mode. */
        unsigned act_ref : 1;
        /*! 1 = enables the inactivity (underthreshold) functionality. */
        unsigned inact_en : 1;
        /*! 1 = inactivity detection function operates in referenced mode.
            0 = inactivity detection function operates in absolute mode. */
        unsigned inact_ref : 1;
        /*! X0 = Default mode: Activity and inactivity detection are both
                enabled, and their interrupts (if mapped) must be acknowledged
                by the host processor by reading the STATUS register. Autosleep
                is disabled in this mode. Use this mode for free fall detection
                applications.
            01 = Linked mode: Activity and inactivity detection are linked
                sequentially such that only one is enabled at a time. Their
                interrupts (if mapped) must be acknowledged by the host processor
                by reading the STATUS register.
            11 = Loop mode: Activity and inactivity detection are linked
                sequentially such that only one is enabled at a time, and their
                interrupts are internally acknowledged (do not need to be serviced
                by the host processor). To use either linked or looped mode, both
                ACT_EN (Bit 0) and INACT_EN (Bit 2) must be set to 1; otherwise,
                the default mode is used. */
        unsigned link_loop : 2;
        /*! Not used */
        unsigned _unused : 2;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} adxl362_activity_control_register_t;

/*! Read/write FIFO control register */
typedef union adxl362_fifo_control_register
{
    /*! The register bit fields */
    struct
    {
        /*! 00: FIFO is disabled
            01: Oldest saved mode
            10: Stream mode
            11: Triggered mode */
        unsigned fifo_mode : 2;
        /*! 1 = temperature data is stored in the FIFO together with x-, y-, and
            z-axis acceleration data. */
        unsigned fifo_temp : 1;
        /*! This bit is the MSB of the FIFO_SAMPLES register, allowing FIFO
            samples a range of 0 to 511 */
        unsigned above_half : 1;
        /*! Not used */
        unsigned _unused : 4;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} adxl362_fifo_control_register_t;

/*! Read/write interrupt control registers (INTMAP1/2).
    If its corresponding bit is set to 1, the function generates an interrupt on
    the INT pin. Bit B7 configures whether the pin operates in active high
    (B7 low) or active low (B7 high) mode. Any number of functions can be
    selected simultaneously for each pin. If multiple functions are selected,
    their conditions are OR'ed together to determine the INT pin state. The
    status of each individual function can be determined by reading the STATUS
    register. If no interrupts are mapped to an INT pin, the pin remains in a
    high impedance state, held to a valid logic state by a bus keeper */
typedef union adxl362_interrupt_control_register
{
    /*! The register bit fields */
    struct
    {
        /*! Map data ready status to the interrupt pin */
        unsigned data_ready : 1;
        /*! Map the FIFO ready status to the interrupt pin */
        unsigned fifo_ready : 1;
        /*! Map the FIFO watermark status to the interrupt pin */
        unsigned fifo_watermark : 1;
        /*! Map the FIFO overrun status to the interrupt pin */
        unsigned fifo_overrun : 1;
        /*! Map the activity status to the interrupt pin */
        unsigned act : 1;
        /*! Map the inactivity status to the interrupt pin */
        unsigned inact : 1;
        /*! Map the awake status to the interrupt pin */
        unsigned awake : 1;
        /*! The interrupt pin is active low */
        unsigned int_low : 1;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} adxl362_interrupt_control_register_t;

/*! Read/write filter control register */
typedef union adxl362_filter_control_register
{
    /*! The register bit fields */
    struct
    {
        /*! Output Data Rate. Selects ODR and configures internal filters to a
            bandwidth of 1/2 or 1/4 the selected ODR, depending on the HALF_BW
            bit setting.
            000: 12.5Hz
            001: 25Hz
            010: 50Hz
            011: 100Hz
            100: 200Hz
            101...111: 400Hz */
        unsigned odr : 3;
        /*! External Sampling Trigger. 1 = the INT2 pin is used for external
            conversion timing control. */
        unsigned ext_sample : 1;
        /*! Halved Bandwidth.
            1 = the bandwidth of the antialiasing filters is set to 1/4 the output
                data rate (ODR) for more conservative filtering.
            0 = the bandwidth of the filters is set to 1/2 the ODR for a wider
                bandwidth. */
        unsigned half_bw : 1;
        /*! Reserved */
        unsigned _reserved : 1;
        /*! Measurement Range Selection
            00: +/- 2g
            01: +/- 4g
            1X: +/- 8g */
        unsigned range : 2;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} adxl362_filter_control_register_t;

/*! Read/write Power control register */
typedef union adxl362_power_control_register
{
    /*! The register bit fields */
    struct
    {
        /*! 00: Standby
            01: Reserved
            10: Measurement Mode
            11: Reserved */
        unsigned measure : 2;
        /*! Activity and inactivity detection must be in linked mode or loop mode
            (LINK/LOOP bits in ACT_INACT_CTL register) to enable autosleep;
            otherwise, the bit is ignored. 1 = autosleep is enabled, and the
            device enters wake-up mode automatically upon detection of inactivity. */
        unsigned autosleep : 1;
        /*! 1 = the part operates in wake-up mode.  */
        unsigned wakeup : 1;
        /*! Selects Power vs. Noise Tradeoff:
            00: Normal Operation
            01: Low noise mode
            10: Ultralow noise mode
            11: Reserved */
        unsigned low_noise : 2;
        /*! 1 = the accelerometer runs off the external clock provided on the INT1 pin. */
        unsigned ext_clk : 1;
        /*! Reserved */
        unsigned _reserved : 1;
    } bits;
    /*! The entire register as a byte */
    uint8 reg;
} adxl362_power_control_register_t;

/*! The 'on' PIO will not be controlled when __accelerometer_config.pio.on is
    set to this value */
#define ADXL362_ON_PIO_UNUSED 255

/*! The high level configuration for taking measurement */
struct __accelerometer_config
{
    /*! Value to write to ADXL362_THRESH_ACTL_REG */
    uint16 activity_threshold;
    /*! Value to write to ADXL362_THRESH_INACTL_REG */
    uint16 inactivity_threshold;
    /*! Value to write to ADXL362_TIME_INACTL_REG */
    uint16 inactivity_timer;
    /*! The SPI clock frequency is kHz */
    uint16 spi_clock_khz;
    /*! The PIOs used to control/communicate with the sensor */
    struct
    {
        /*! PIO used to power-on the accelerometer or #ADXL362_ON_PIO_UNUSED */
        uint8 on;
        /*! Interrupt PIO driven by the accelerometer */
        uint8 interrupt;
        /*! SPI clock PIO */
        uint8 spi_clk;
        /*! SPI chip select (active low) PIO */
        uint8 spi_cs;
        /*! SPI master-out-slave-in PIO */
        uint8 spi_mosi;
        /*! SPI master-in-slave-out PIO */
        uint8 spi_miso;
    } pios;
};

#endif /* HAVE_ADXL362 */
#endif /* ADXL362_PRIVATE_H */
