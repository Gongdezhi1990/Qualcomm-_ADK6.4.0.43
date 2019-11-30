/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2019

\file
    This defines functions for controlling the LCD display on the
    development board.

    An application wishing to use this should call dev_board_lcd_initialise()
    once during initialisation and can then follow with calls to
    dev_board_lcd_write().

*/

#include "dev_board_lcd.h"

#include <Panic.h>
#include <i2c.h>

#if defined(H13672v2)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       26
    #define LCD_BACKLIGHT_PIO   0
    #define LCD_I2C_CLOCK_PIO   0
    #define LCD_I2C_DATA_PIO    0
    #define LCD_DISPLAY_LEN     16
#elif defined(CE446)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       26
    #define LCD_BACKLIGHT_PIO   0
    #define LCD_I2C_CLOCK_PIO   0
    #define LCD_I2C_DATA_PIO    0
    #define LCD_DISPLAY_LEN     16
#elif defined(CF376_CE692H3)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       26
    #define LCD_BACKLIGHT_PIO   72
    #define LCD_I2C_CLOCK_PIO   24
    #define LCD_I2C_DATA_PIO    25
    #define LCD_DISPLAY_LEN     16
#elif defined(CF376_CE826)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       28
    #define LCD_BACKLIGHT_PIO   24
    #define LCD_I2C_CLOCK_PIO   60
    #define LCD_I2C_DATA_PIO    61
    #define LCD_DISPLAY_LEN     16
#elif defined(CF376_CF212)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       43
    #define LCD_BACKLIGHT_PIO   45
    #define LCD_I2C_CLOCK_PIO   48
    #define LCD_I2C_DATA_PIO    49
    #define LCD_DISPLAY_LEN     16
#elif defined(CF376_CF429)
    #define HAVE_LCD            0
#elif defined(CF376_CF440)
    #define HAVE_LCD            0
#elif defined(CF133)
    #define HAVE_LCD            0
#elif defined(QCC5127_AA_DEV_BRD_R2_AA)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       21
    #define LCD_BACKLIGHT_PIO   54
    #define LCD_I2C_CLOCK_PIO   56
    #define LCD_I2C_DATA_PIO    57
    #define LCD_DISPLAY_LEN     16
#elif defined(CG437)
    #define HAVE_LCD            0
#elif defined(CF376_CG724)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       55
    #define LCD_BACKLIGHT_PIO   54
    #define LCD_I2C_CLOCK_PIO   56
    #define LCD_I2C_DATA_PIO    57
    #define LCD_DISPLAY_LEN     16
#elif defined(QCC5127_AB_DEV_BRD_R2_AA)
    #define HAVE_LCD            1
    #define LCD_I2C_ADDRESS     (0x3E << 1)
    #define LCD_RESET_PIO       5
    #define LCD_BACKLIGHT_PIO   45
    #define LCD_I2C_CLOCK_PIO   40
    #define LCD_I2C_DATA_PIO    41
    #define LCD_DISPLAY_LEN     16	
#else
    #error HW Variant not supported
#endif

#define SW_PIO_MASK(bank,pio)   (1ul << ((pio)-((bank)*PIOS_PER_BANK)))
#define SW_PIO_BANK(pio)        ((pio)/PIOS_PER_BANK)

#if HAVE_LCD

/* Temporary replacement for VmgetClock() which is not yet available */
extern uint32 get_milli_time(void);

static bool display_initialised = FALSE;

/*!
 * \brief Local function to perform a busy_loop for a short delay.
 *
 * \param n_us Delay in microseconds
 */
static void internal_delay_us(uint32 n_us)
{
    /* Note that we can't call get_milli_time before the scheduler is running.
       Causes a panic */
    uint32 clock = get_milli_time();
    while ((get_milli_time() - clock) < n_us/1000 + 1);
}

/**
 * \brief setup_i2c
 *
 * This function initialises the PIOs controlling the I2C bus.
 */
static void setup_i2c(void)
{
    if (LCD_I2C_CLOCK_PIO != 0 && LCD_I2C_DATA_PIO != 0)
    {
        const uint16 i2cbank_clock = SW_PIO_BANK(LCD_I2C_CLOCK_PIO);
        const uint16 i2cbank_data  = SW_PIO_BANK(LCD_I2C_DATA_PIO);
        const uint32 i2cPio_clock  = SW_PIO_MASK(i2cbank_clock,LCD_I2C_CLOCK_PIO);
        const uint32 i2cPio_data   = SW_PIO_MASK(i2cbank_data,LCD_I2C_DATA_PIO);
        if (i2cbank_clock == i2cbank_data)
        {
            /* Both PIOs can be setup at the same time */
            const uint32 i2cPios = i2cPio_clock | i2cPio_data;
            PioSetMapPins32Bank(i2cbank_clock, i2cPios, i2cPios);
        }
        else
        {
            PioSetMapPins32Bank(i2cbank_clock, i2cPio_clock, i2cPio_clock);
            PioSetMapPins32Bank(i2cbank_data, i2cPio_data, i2cPio_data);
        }
    }
}

/*!
 * \brief Initialise the LCD display.
 *
 * This function should be called once during initialisation of
 * the system. If it is not called then calls to
 * dev_board_lcd_write() will panic.
 */
void dev_board_lcd_initialise(void)
{
    const uint16 lcd_reset_bank = SW_PIO_BANK(LCD_RESET_PIO);
    const uint32 lcd_reset_pin = SW_PIO_MASK(lcd_reset_bank,LCD_RESET_PIO);
    const uint16 lcd_backlight_bank = SW_PIO_BANK(LCD_BACKLIGHT_PIO);
    const uint32 lcd_backlight_pin = SW_PIO_MASK(lcd_backlight_bank,LCD_BACKLIGHT_PIO);
    static uint8 const init_cmd[] = { 0x80, 0x38, 0x80, 0x39,
                                      0x80, 0x14, 0x80, 0x74,
                                      0x80, 0x54, 0x80, 0x6f,
                                      0x80, 0x0c, 0x00, 0x01};

    setup_i2c();
    PioSetMapPins32Bank(lcd_reset_bank, lcd_reset_pin, lcd_reset_pin);
    PioSetDir32Bank(lcd_reset_bank, lcd_reset_pin, 0);
    PioSet32Bank(lcd_reset_bank, lcd_reset_pin, lcd_reset_pin);

    internal_delay_us(5000);
    I2cTransfer(LCD_I2C_ADDRESS, init_cmd, sizeof(init_cmd), 0, 0);
    internal_delay_us(5000);

    if (LCD_BACKLIGHT_PIO != 0)
    {
        PioSetMapPins32Bank(lcd_backlight_bank, lcd_backlight_pin, lcd_backlight_pin);
        PioSetDir32Bank(lcd_backlight_bank, lcd_backlight_pin, lcd_backlight_pin);
    }

    display_initialised = TRUE;
}

/*!
 * \brief Write the supplied text to the specified
 * line of the LCD display.
 *
 * \internal See header file for full description
 */
void dev_board_lcd_write(uint8 line, char * text)
{
    uint8 size;
    uint8 buf[LCD_DISPLAY_LEN+3] = {0x80, line ? 0xc0:0x80, 0x40};

    if (!display_initialised)
    {
        Panic();
    }

    size = (uint8)(MIN(strlen(text),LCD_DISPLAY_LEN));
    memcpy(&buf[3], text, size);
    I2cTransfer(LCD_I2C_ADDRESS, buf, (uint8)(size + 3), 0, 0);
}

#else

/* NOP functions when there is no hardware support for LCD */
void dev_board_lcd_initialise(void)
{
}

void dev_board_lcd_write(uint8 line, char * text)
{
    UNUSED(line);
    UNUSED(text);
}

#endif
