/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    display_midas.c

DESCRIPTION

NOTES

*/


#include <stdlib.h>
#include <display.h>
#include <panic.h>
#include <print.h>
#include <stream.h>
#include <app/vm/vm_if.h>
#include <message.h>
#include <string.h>
#include <i2c.h>

#include "display_plugin_if.h"
#include "display_plugin_midas.h"
#include "display_midas.h"
#include "pio_common.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#ifdef DEBUG_DISPLAY
#include <stdio.h>
#include <panic.h>
#define DISPLAY_DEBUG(X) printf X
#define DISPLAY_PANIC() Panic()
#else
#define DISPLAY_DEBUG(X)
#define DISPLAY_PANIC()
#endif

#define LINES 2

typedef struct display_Tag
{

   uint16   volume;
   uint8    battery_level;
   uint8    icon;
   bool     icon_state;
   bool     status_displayed;
   /* The folling 2 members store the information about each line, 0 is used for the location of the status information */
   uint8    line_start[LINES+1];
   uint8    line_len[LINES+1];
   Task     app_task;
   uint8    backlight_pio;

}DISPLAY_MIDAS_t ;

/* The task instance pointer*/
static DISPLAY_MIDAS_t * DISPLAY_MIDAS = NULL;

/* Crescendo Dev Board display and timeout */
#define LCD_I2C_ADDRESS     (0x3E << 1)
#define LCD_TEXT_LEN        16
#define LCD_HEADER_LEN       3

static uint16 write_initial_content(void);
static uint16 lcd_write_text(uint8 line, uint8 length, char * text);

/* ST7032 LCD controller commands */
#define LCD_CMD_CLEAR (0x01)
#define LCD_CMD_HOME (0x02)
#define LCD_CMD_ENTRY_MODE (0x06)
#define LCD_CMD_DISPLAY (0x08)
#define LCD_CMD_CGRAM (0x40)
#define LCD_CMD_DDRAM (0x80)
#define LCD_CMD_ICRAM (0x40)

#define LCD_CMD_FUNCTION_SET_IS                 (0x38)
#define LCD_CMD_INTERNAL_OSC_FREQUENCY          (0x14)
#define LCD_CMD_CONTRAST_SET                    (0x74)
#define LCD_CMD_POWER_ICON_CONTROL_CONTRAST_SET (0x54)
#define LCD_CMD_FOLLOWER_CONTROL                (0x6F)
#define LCD_CMD_DISPLAY_OFF                     (0x08)
#define LCD_CMD_DISPLAY_ON                      (0x0C)
#define LCD_CMD_CLEAR_DISPLAY                   (0x01)

#define RS_CMD                                  (0x00)
#define RS_CMD_CO                               (0x80)
#define RS_DATA                                 (0x40)

#define DISPLAY_MAX_VOLUME (15)

#define DISPLAY_BACKLIGHT_ON_TIME   (3)

#define DISPLAY_BACKLIGHT_OFF   TRUE
#define DISPLAY_BACKLIGHT_ON    FALSE

/* NB. The application numbers lines as 1 and 2, but the plugin numbers them as 0 and 1. */
#define LINE_0_TEXT_ADDRESS     (0x00)
#define LINE_1_TEXT_ADDRESS     (0x40)

/* Reset delay seems to need to be at least 350us from trials. 
         Extended this to 2ms to have safe margin.        */
#define RESET_DELAY_PERIOD_MS     2

#define send_backlight_message(task, backlight_pio, backlight_status)\
                               send_delayed_backlight_message(task, backlight_pio, backlight_status, 0)

static void pio_initialise(uint8 reset_pio)
{
    PRINT(("DISP: reset PIO %u\n", reset_pio));

    if(reset_pio != LCD_INVALID_PIO)
    {
        PioCommonSetPio(reset_pio, pio_pull, TRUE);
    }
}

static void send_delayed_backlight_message(DisplayMidasPluginTaskdata *task, uint8 backlight_pio, bool backlight_status, uint32 delay)
{
    MAKE_DISPLAY_MESSAGE(DispExBacklightMessage);
    message->backlight_pio = backlight_pio;
    message->backlight_status = backlight_status;

    MessageSendLater((TaskData *)task, DISP_EX_SET_BACKLIGHT, message, delay);
}

static void delayed_backlight_off(DisplayMidasPluginTaskdata *task, uint8 backlight_pio)
{
    /* cancel any backlight messages */
    MessageCancelAll((TaskData*)task , DISP_EX_SET_BACKLIGHT);

    send_delayed_backlight_message(task, backlight_pio, DISPLAY_BACKLIGHT_OFF, D_SEC(DISPLAY_BACKLIGHT_ON_TIME));
}

static uint8* write_text_command_buf(void)
{
    static uint8 buf[LCD_HEADER_LEN + LCD_TEXT_LEN] = {
        RS_CMD_CO, LCD_CMD_DDRAM,
        RS_DATA, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    uint8 pos;

    buf[1] = LCD_CMD_DDRAM;
    for (pos = LCD_HEADER_LEN + LCD_TEXT_LEN-1; pos >= LCD_HEADER_LEN; --pos)
    {
        buf[pos] = ' ';
    }
    return buf;
}

static uint16 lcd_initialise(bool display_status)
{
    /* For Vdd = 3V the initialisation sequence is as follows */
    /* See display manual:
     * http://www.farnell.com/datasheets/2021770.pdf */
    static uint8 const init_cmd[] = {
        RS_CMD_CO, LCD_CMD_FUNCTION_SET_IS | 0,
        RS_CMD_CO, LCD_CMD_FUNCTION_SET_IS | 1,
        RS_CMD_CO, LCD_CMD_INTERNAL_OSC_FREQUENCY,
        RS_CMD_CO, LCD_CMD_CONTRAST_SET,
        RS_CMD_CO, LCD_CMD_POWER_ICON_CONTROL_CONTRAST_SET,
        RS_CMD_CO, LCD_CMD_FOLLOWER_CONTROL,
        RS_CMD_CO, LCD_CMD_DISPLAY_ON,
        RS_CMD,    LCD_CMD_CLEAR_DISPLAY};
    static uint8 const set_icons[] = {
        RS_CMD_CO, LCD_CMD_DDRAM | 32,  /* For some reason need to set the write location to DDRAM ... */
        RS_CMD_CO, LCD_CMD_DDRAM | 32,  /*   ... 3 times, otherwise things don't work */
        RS_CMD_CO, LCD_CMD_DDRAM | 32,  /* LCD_CMD_DDRAM|32 is the code of a space, just in case it is written to memory rather than being executed */
        RS_CMD_CO, LCD_CMD_FUNCTION_SET_IS | 0,     /* Switch to Function Set 0 */
        RS_CMD_CO, LCD_CMD_CGRAM,       /* Set write location to CGRAM - where the icons are stored */
        RS_DATA,
    #define LCD_CHR_BATT (0)
        0x0E, /* .###.     Char 0, battery icon (initially empty) */
        0x1F, /* ##### */
        0x11, /* #...# */
        0x11, /* #...# */
        0x11, /* #...# */
        0x11, /* #...# */
        0x11, /* #...# */
        0x1F, /* ##### */
    #define LCD_CHR_VOL_LBR (1)
        0x01, /* ....#     Char 1, volume bar left bracket */
        0x01, /* ....# */
        0x01, /* ....# */
        0x01, /* ....# */
        0x01, /* ....# */
        0x01, /* ....# */
        0x01, /* ....# */
        0x01, /* ....# */
    #define LCD_CHR_VOL_0 (2)
        0x15, /* #.#.#     Char 2, 0 bars */
        0x00, /* ..... */
        0x00, /* ..... */
        0x00, /* ..... */
        0x00, /* ..... */
        0x00, /* ..... */
        0x00, /* ..... */
        0x15, /* #.#.# */
    #define LCD_CHR_VOL_1 (3)
        0x15, /* #.#.#     Char 3, 1 bar */
        0x00, /* ..... */
        0x10, /* #.... */
        0x10, /* #.... */
        0x10, /* #.... */
        0x10, /* #.... */
        0x00, /* ..... */
        0x15, /* #.#.# */
    #define LCD_CHR_VOL_2 (4)
        0x15, /* #.#.#     Char 4, 2 bars */
        0x00, /* ..... */
        0x14, /* #.#.. */
        0x14, /* #.#.. */
        0x14, /* #.#.. */
        0x14, /* #.#.. */
        0x00, /* ..... */
        0x15, /* #.#.# */
    #define LCD_CHR_VOL_3 (5)
        0x15, /* #.#.#     Char 5, 3 bars */
        0x00, /* ..... */
        0x15, /* #.#.# */
        0x15, /* #.#.# */
        0x15, /* #.#.# */
        0x15, /* #.#.# */
        0x00, /* ..... */
        0x15, /* #.#.# */
    #define LCD_CHR_VOL_RBR (6)
        0x10, /* #....     Char 6, volume bar right bracket */
        0x10, /* #.... */
        0x10, /* #.... */
        0x10, /* #.... */
        0x10, /* #.... */
        0x10, /* #.... */
        0x10, /*.#.... */
        0x10};/*.#.... */
    uint16 ret;

    ret = I2cTransfer(LCD_I2C_ADDRESS, init_cmd, sizeof(init_cmd), 0, 0);
    PRINT(("DISP_Init: Init_Cmd returned %u\n", ret));
    if (ret < sizeof(init_cmd))
        ret = 0;
    if (ret > 0)
    {
        ret = I2cTransfer(LCD_I2C_ADDRESS, set_icons, sizeof(set_icons), 0, 0);
        PRINT(("DISP_Init: set_icons returned %u\n", ret));
        if (ret < sizeof(set_icons))
            ret = 0;
    }

    if (display_status)
        write_initial_content();

    return ret != 0;
}

static uint16 write_initial_content(void)
{
    static char status_bar[] = {
    /*  Show bracketed volume bars (initially level 0)  */
        LCD_CHR_VOL_LBR,
        LCD_CHR_VOL_0,
        LCD_CHR_VOL_0,
        LCD_CHR_VOL_0,
        LCD_CHR_VOL_0,
        LCD_CHR_VOL_0,
        LCD_CHR_VOL_RBR,
    /*  Show battery icon  */
        LCD_CHR_BATT};
    return lcd_write_text(0, ARRAY_SIZE(status_bar), status_bar);
}

static uint16 lcd_write_text(uint8 line, uint8 length, char * text)
{
    uint8 min_length;
    uint16 ret;
    uint8 line_len = DISPLAY_MIDAS->line_len[line];
    uint8* buf = write_text_command_buf();

    /* Set DDRAM address for line. */
    buf[1] |= DISPLAY_MIDAS->line_start[line];

    min_length = length < LCD_TEXT_LEN ? length : LCD_TEXT_LEN;
    memcpy(&buf[LCD_HEADER_LEN], text, min_length);
    ret = I2cTransfer(LCD_I2C_ADDRESS, buf, LCD_HEADER_LEN + line_len, 0, 0);
    PRINT(("DISP: I2cTransfer return %d:", ret));
    return ret;
}

/* display plugin functions*/
void DisplayMidasPluginInit(DisplayMidasPluginTaskdata *task,
                            Task app_task,
                            uint8 reset_pio,
                            uint8 backlight_pio)
{
    UNUSED(task);
    if (DISPLAY_MIDAS)
        Panic();

    DISPLAY_MIDAS = PanicUnlessNew ( DISPLAY_MIDAS_t );

    /* initalise the display */
    DISPLAY_MIDAS->app_task = app_task;
    DISPLAY_MIDAS->volume = 0;
    DISPLAY_MIDAS->battery_level = 0;
    DISPLAY_MIDAS->icon = 0;
    DISPLAY_MIDAS->icon_state = FALSE;
    DISPLAY_MIDAS->status_displayed = FALSE;
    DISPLAY_MIDAS->line_start[0] = LINE_1_TEXT_ADDRESS + LCD_TEXT_LEN/2;
    DISPLAY_MIDAS->line_start[1] = LINE_0_TEXT_ADDRESS;
    DISPLAY_MIDAS->line_start[2] = LINE_1_TEXT_ADDRESS;
    DISPLAY_MIDAS->line_len[0] = LCD_TEXT_LEN/2;
    DISPLAY_MIDAS->line_len[1] = LCD_TEXT_LEN;
    DISPLAY_MIDAS->line_len[2] = LCD_TEXT_LEN;
    DISPLAY_MIDAS->backlight_pio = backlight_pio;

    PRINT(("DISPLAYEX: Reseting Display\n"));
    SetDisplayBusy((TaskData*)task);
    pio_initialise(reset_pio);
    
    /* decouple the resetting of the display via PIO and the first I2CTransfer 
        using a delayed DISP_EX_RESET_COMPLETE message */
    MessageSendLater((TaskData*)task, DISP_EX_RESET_COMPLETE, NULL, RESET_DELAY_PERIOD_MS);
}

/* There needs to be a gap of 350 micro-sec between the display reset line being triggered.
   and communication with the display beginning.  This gap was being achieved more by luck than
   judgement.  However, when a new device started using the I2C bus, the code executed differently
   and a gap of only 250 micro-sec was occuring.  Thus the delay at the end of the above function
   (1ms) was added before the code in this function occurs.*/
void DisplayMidasResetComplete(DisplayMidasPluginTaskdata *task)
{
    MAKE_DISPLAY_MESSAGE(DISPLAY_PLUGIN_INIT_IND);
    UNUSED(task);

    if (!DISPLAY_MIDAS)
        Panic();

    PRINT(("DISPLAYEX: Initialising Display\n"));
    message->result = lcd_initialise(DISPLAY_MIDAS->status_displayed);
    /* indicate result */
    PRINT(("DISPLAYEX: Send DISPLAY_PLUGIN_INIT_IND\n"));
    MessageSend(DISPLAY_MIDAS->app_task, DISPLAY_PLUGIN_INIT_IND, message);
    SetDisplayBusy(NULL);
    send_backlight_message(task, DISPLAY_MIDAS->backlight_pio, DISPLAY_BACKLIGHT_ON);
}

void DisplayMidasPluginSetState( DisplayMidasPluginTaskdata *task, bool state )
{
    uint8 display_on_off_cmd[2];

    UNUSED(task);

    PRINT(("DISP: State %u:\n", state));

    display_on_off_cmd[0] = RS_CMD;
    display_on_off_cmd[1] = state == 0 ? LCD_CMD_DISPLAY_OFF : LCD_CMD_DISPLAY_ON;
    I2cTransfer(LCD_I2C_ADDRESS, display_on_off_cmd, sizeof(display_on_off_cmd), 0, 0);
    send_backlight_message(task, DISPLAY_MIDAS->backlight_pio, DISPLAY_BACKLIGHT_ON);
}

void DisplayMidasSetStatusLocation( DisplayMidasPluginTaskdata *task, uint8 line, bool start)
{
    UNUSED(task);
    DISPLAY_MIDAS->line_len[0] = LCD_TEXT_LEN/2;
    DISPLAY_MIDAS->line_start[1] = LINE_0_TEXT_ADDRESS;
    DISPLAY_MIDAS->line_start[2] = LINE_1_TEXT_ADDRESS;
    if ((line > 0) && (line <= LINES))
    {
        DISPLAY_MIDAS->status_displayed = TRUE;
        if (line == 1)
        {
            DISPLAY_MIDAS->line_start[0] = LINE_0_TEXT_ADDRESS;
            DISPLAY_MIDAS->line_len[1] = LCD_TEXT_LEN/2;
            DISPLAY_MIDAS->line_len[2] = LCD_TEXT_LEN;
            if (start)
            {
                DISPLAY_MIDAS->line_start[1] += LCD_TEXT_LEN/2;
            }
            else
            {
                DISPLAY_MIDAS->line_start[0] += LCD_TEXT_LEN/2;
            }
        }
        else    /* line == 2 */
        {
            DISPLAY_MIDAS->line_start[0] = LINE_1_TEXT_ADDRESS;
            DISPLAY_MIDAS->line_len[1] = LCD_TEXT_LEN;
            DISPLAY_MIDAS->line_len[2] = LCD_TEXT_LEN/2;
            if (start)
            {
                DISPLAY_MIDAS->line_start[2] += LCD_TEXT_LEN/2;
            }
            else
            {
                DISPLAY_MIDAS->line_start[0] += LCD_TEXT_LEN/2;
            }
        }
    }
    else
    {
        DISPLAY_MIDAS->status_displayed = FALSE;
        DISPLAY_MIDAS->line_start[0] = LINE_1_TEXT_ADDRESS + LCD_TEXT_LEN/2;
        DISPLAY_MIDAS->line_len[1] = LCD_TEXT_LEN;
        DISPLAY_MIDAS->line_len[2] = LCD_TEXT_LEN;
    }
    write_initial_content();
}

void DisplayMidasPluginSetText( DisplayMidasPluginTaskdata *task, char* text, uint8 text_length, uint8 line, uint8 scroll, bool flash, uint16 scroll_update, uint16 scroll_pause, uint16 display_time )
{
    uint8 line_length = DISPLAY_MIDAS->line_len[line];
    uint8 active_line_clear_msg;
    uint8 active_line_scroll_msg;
    UNUSED(flash);

    PRINT(("DISP: (L%u)(L%u)(F%u)(S%u,%u)(D:%u): ", line,text_length, flash, scroll_update, scroll_pause, display_time));
    PRINT(("%-.*s\n", text_length, text));

    if (line<=0 || line>LINES)
        return;
    if (0==text_length)
    {
        DisplayMidasPluginClearText( task, line );
        return;
    }

    /* configure clear messages so they're active on the correct line number */
    /* NB. The application numbers lines as 1 and 2, but the plugin numbers them as 0 and 1. */
    active_line_clear_msg = (line==1)?DISP_EX_TEXTLN0_CLEAR_INT:DISP_EX_TEXTLN1_CLEAR_INT;

    /* cancel any clear messages */
    MessageCancelAll( (TaskData*)task , active_line_clear_msg);

#ifdef ENABLE_SCROLL
    /* configure scroll messages so they're active on the correct line number */
    active_line_scroll_msg = (line==1)?DISP_EX_SCROLLLN0_TEXT_INT:DISP_EX_SCROLLLN1_TEXT_INT;
    /* cancel any scrolling messages */
    MessageCancelAll( (TaskData*)task , active_line_scroll_msg);

    switch(scroll)
    {
        case DISPLAY_TEXT_SCROLL_SCROLL:
        case DISPLAY_TEXT_SCROLL_BOUNCE:
        {
            if (text_length > line_length)
            {
                /* send message to update display for scroll */
                MAKE_DISPLAY_MESSAGE_WITH_LEN(DispExScrollMessage, text_length);

                message->text_length = text_length;
                message->text_pos = 1;
                message->line = line;
                message->bounce = (scroll == DISPLAY_TEXT_SCROLL_BOUNCE);
                message->scroll_pause = scroll_pause?scroll_pause:scroll_update;
                message->scroll_update = scroll_update;
                memmove(message->text,text,text_length) ;

                /* if scrolling, check if a longer pause is required to start scrolling */
                MessageSendLater((TaskData*)task, active_line_scroll_msg, message, message->scroll_pause);
            }
        }
        break;
        case DISPLAY_TEXT_SCROLL_STATIC:
        default:
            ; /* do nothing */
    }
#else
    UNUSED(scroll);
    UNUSED(scroll_pause);
    UNUSED(scroll_update);
    UNUSED(active_line_scroll_msg);
    UNUSED(line_length);
#endif

    lcd_write_text(line, text_length, text);

    /* check if this display is for a limited time only */
    if(display_time)
    {
        MAKE_DISPLAY_MESSAGE(DispExClearLineMessage);
        message->line = line;
        MessageSendLater((TaskData*)task, active_line_clear_msg, message, D_SEC(display_time));
    }
    send_backlight_message(task, DISPLAY_MIDAS->backlight_pio, DISPLAY_BACKLIGHT_ON);
}

void DisplayMidasPluginSetVolume( DisplayMidasPluginTaskdata *task, uint16 volume )
{
    UNUSED(task);

    PRINT(("DISP: Volume %u:\n", volume));

    DISPLAY_MIDAS->volume = volume;
    if (DISPLAY_MIDAS->status_displayed)
    {
        const uint8 LEVELS_PER_CHAR = 3;
        const uint8 LEN_VOL_STRING = DISPLAY_MAX_VOLUME / LEVELS_PER_CHAR;
        uint8* buf = write_text_command_buf();
        uint8* vol_string = &buf[LCD_HEADER_LEN];
        uint16 level;
        uint16 ret;
        UNUSED(ret);    /* Only used if logging enabled */
        buf[1] |= DISPLAY_MIDAS->line_start[0] + 1;
        /*  Build volume bar from groups of 0, 1, 2 or 3  */
        for (level = 1; level <= DISPLAY_MAX_VOLUME; level += LEVELS_PER_CHAR)
        {
            if (volume < level)
                *vol_string = LCD_CHR_VOL_0;
            else if (volume == level)
                *vol_string = LCD_CHR_VOL_1;
            else if (volume - level == 1)
                *vol_string = LCD_CHR_VOL_2;
            else
                *vol_string = LCD_CHR_VOL_3;
            ++vol_string;
        }
        ret = I2cTransfer(LCD_I2C_ADDRESS, buf, LCD_HEADER_LEN + LEN_VOL_STRING, 0, 0);
        PRINT(("DISP: I2cTransfer of volumn string return %d:", ret));
        send_backlight_message(task, DISPLAY_MIDAS->backlight_pio, DISPLAY_BACKLIGHT_ON);
    }
}

void DisplayMidasPluginSetIcon( DisplayMidasPluginTaskdata *task, uint8 icon, bool state )
{
    UNUSED(task);

    PRINT(("DISP: Icon %u(%u):\n", icon, state));

    DISPLAY_MIDAS->icon = icon;
    DISPLAY_MIDAS->icon_state = state;
    send_backlight_message(task, DISPLAY_MIDAS->backlight_pio, DISPLAY_BACKLIGHT_ON);
    /* Displaying Icon is not currently supported. */
}

void DisplayMidasPluginSetBacklight( DisplayMidasPluginTaskdata *task, uint8 backlight_pio, bool backlight_status)
{
    PRINT(("DISP: backlight PIO %u\n", backlight_pio));

    if(backlight_pio != LCD_INVALID_PIO)
    {
        PioCommonSetPio(backlight_pio, pio_drive, backlight_status);

        if(backlight_status == DISPLAY_BACKLIGHT_ON)
            delayed_backlight_off(task, backlight_pio);
    }
}

void DisplayMidasPluginSetBattery( DisplayMidasPluginTaskdata *task, uint8 battery_level )
{
    static uint8 set_icon[] = {
        RS_CMD_CO, LCD_CMD_DDRAM,       /* For some reason need to set the write location to DDRAM ... */
        RS_CMD_CO, LCD_CMD_DDRAM,       /*   ... twice, otherwise things sometimes don't work */
        RS_CMD_CO, LCD_CMD_FUNCTION_SET_IS | 0,     /* Switch to Function Set 0 */
        RS_CMD_CO, LCD_CMD_CGRAM,       /* Set write location to CGRAM - where the icons are stored */
        RS_DATA,
        0x0E, /* .###. */
        0x1F, /* ##### */
        0x11, /* #...# */
        0x11, /* #...# */
        0x11, /* #...# */
        0x11, /* #...# */
        0x11, /* #...# */
        0x1F};/* ##### */
    const uint8 START_DATA = ARRAY_SIZE(set_icon)-8;
    uint16 level;
    uint16 ret;
    UNUSED(task);
    UNUSED(ret);    /* Only used if logging enabled */

    PRINT(("DISP: Batt %u:\n", battery_level));

    DISPLAY_MIDAS->battery_level = battery_level;

    /* if on charge display a 'C' in the battery */
    if(battery_level == 0xff)
    {
        set_icon[START_DATA + 2] = 0x11; /* #...# */
        set_icon[START_DATA + 3] = 0x17; /* #.### */
        set_icon[START_DATA + 4] = 0x17; /* #.### */
        set_icon[START_DATA + 5] = 0x17; /* #.### */
        set_icon[START_DATA + 6] = 0x11; /* #...# */
    }
    else
    {
        for (level = 5; level; --level)
        {
            if (level > battery_level)
                set_icon[START_DATA + 7 - level] = 0x11; /* #...# */
            else
                set_icon[START_DATA + 7 - level] = 0x1F; /* ##### */
        }
    }

    ret = I2cTransfer(LCD_I2C_ADDRESS, set_icon, sizeof(set_icon), 0, 0);
    PRINT(("DISP: I2cTransfer of battery icon returned: %d:", ret));
}

#ifdef ENABLE_SCROLL
void DisplayMidasPluginScrollText( DisplayMidasPluginTaskdata *task, DispExScrollMessage_T * dispscrmsg)
{
    PRINT(("DISP: Scroll line  %u, %u, %u: %s\n", dispscrmsg->line, dispscrmsg->scroll_update, dispscrmsg->scroll_pause, dispscrmsg->text));
    if ((dispscrmsg->line >= 0) && (dispscrmsg->line <= LINES))
    {
        /* configure scroll messages so they're active on the correct line number */
        const uint8 active_line_scroll_msg = (dispscrmsg->line==1)?DISP_EX_SCROLLLN0_TEXT_INT:DISP_EX_SCROLLLN1_TEXT_INT;
        uint8 line_length = DISPLAY_MIDAS->line_len[dispscrmsg->line];
        uint8 text_pos;

        bool atEndOfText = FALSE;
            /* send message to update display for scroll */
        MAKE_DISPLAY_MESSAGE_WITH_LEN(DispExScrollMessage, dispscrmsg->text_length);

        message->text_length = dispscrmsg->text_length;
        message->text_pos = dispscrmsg->text_pos+1;
        if (dispscrmsg->text_pos <= 0)
        {
            text_pos = -dispscrmsg->text_pos;
            if (0 == text_pos)
                atEndOfText = TRUE;
        }
        else
        {
            text_pos = dispscrmsg->text_pos;
            if ((uint8)message->text_pos > message->text_length-line_length)
            {
                atEndOfText = TRUE;
                if (dispscrmsg->bounce)
                    message->text_pos = 1-dispscrmsg->text_pos;    /* Go back one character, but set negative as now text is going in other direction */
                else
                    message->text_pos = 0;    /* Go back to the beginning */
            }
        }
        message->line = dispscrmsg->line;
        message->bounce = dispscrmsg->bounce;
        message->scroll_pause = dispscrmsg->scroll_pause;
        message->scroll_update = dispscrmsg->scroll_update;
        memmove(message->text,dispscrmsg->text,dispscrmsg->text_length) ;

        /* if scrolling, check if a longer pause is required to start scrolling */
        MessageSendLater((TaskData*)task, active_line_scroll_msg, message, atEndOfText?message->scroll_pause:message->scroll_update);
        lcd_write_text(dispscrmsg->line, line_length, &dispscrmsg->text[text_pos]);
    }
}
#endif

void DisplayMidasPluginClearText( DisplayMidasPluginTaskdata *task, uint8 line )
{
#ifdef ENABLE_SCROLL
    /* configure scroll messages so they're active on the correct line number */
    const uint8 active_line_scroll_msg = (line==1)?DISP_EX_SCROLLLN0_TEXT_INT:DISP_EX_SCROLLLN1_TEXT_INT;
    /* Cancel any scrolling on this line */
    MessageCancelAll( (TaskData*)task , active_line_scroll_msg);
#endif
    UNUSED(task);

    PRINT(("DISP: Clear line (%u)\n", line ));
    lcd_write_text(line, 0, (char *)"");
}
