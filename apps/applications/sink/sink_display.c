/****************************************************************************
Copyright (c) 2010 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_display.c

DESCRIPTION

*/

#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_audio_routing.h"
#include "sink_usb.h"

#include "sink_display.h"
#include "sink_statemanager.h"
#include <display.h>
#include <string.h>
#include <display_plugin_if.h>

#ifdef ENABLE_DISPLAY

#include "sink_display_config_def.h"

#ifdef ENABLE_DISPLAY_MIDAS
#include <display_plugin_midas.h>
#else
#include <display_plugin_cns10010.h>
#endif

#ifdef DEBUG_DISPLAY
    #define DISPLAY_DEBUG(x) DEBUG(x)
#else
    #define DISPLAY_DEBUG(x)
#endif


#ifdef ENABLE_DISPLAY_MIDAS
#define DISPLAY_PLUGIN (TaskData *) &display_plugin_midas
#else
#define DISPLAY_PLUGIN (TaskData *) &display_plugin_cns10010
#endif

#define HIDDEN_LINE     0
#define TOP_LINE        1
#define LOWER_LINE      2

#define START_OF_LINE   TRUE
#define END_OF_LINE     FALSE

#define TOTAL_LINES 2

typedef struct line_information
{
    uint16 type;        /* The text type currently displayed on this line */
    uint32 hash;        /* A hash of the content of the text currently on this line */
}LINE_INFO_t ;


/* This array controls which line of the display each of these types appears on */
static const uint8 typeToLine[SINK_TEXT_NUM_TYPES] = {
        0,  /* SINK_TEXT_TYPE_NONE */
        1,  /* SINK_TEXT_TYPE_AUDIO_SOURCE */
        2,  /* SINK_TEXT_TYPE_DEVICE_STATE */
        1,  /* SINK_TEXT_TYPE_CHARGER_STATE */
        2,  /* SINK_TEXT_TYPE_PLAY_STATE */
        1,  /* SINK_TEXT_TYPE_MEDIA_INFO */
        1,  /* SINK_TEXT_TYPE_CALLER_INFO */
        2,  /* SINK_TEXT_TYPE_GATT_INFO */
        1,  /* SINK_TEXT_TYPE_RADIO_FREQ */
        2}; /* SINK_TEXT_TYPE_RADIO_INFO */

/* This array gives a priority lever to each type of text */
static const uint8 priority[SINK_TEXT_NUM_TYPES] = {
        0,  /* SINK_TEXT_TYPE_NONE */
        1,  /* SINK_TEXT_TYPE_AUDIO_SOURCE */
        1,  /* SINK_TEXT_TYPE_DEVICE_STATE */
        1,  /* SINK_TEXT_TYPE_CHARGER_STATE */
        2,  /* SINK_TEXT_TYPE_PLAY_STATE */
        4,  /* SINK_TEXT_TYPE_MEDIA_INFO */
        4,  /* SINK_TEXT_TYPE_CALLER_INFO */
        3,  /* SINK_TEXT_TYPE_GATT_INFO */
        2,  /* SINK_TEXT_TYPE_RADIO_FREQ */
        2}; /* SINK_TEXT_TYPE_RADIO_INFO */

/* Information about what is currently on a particular line */
static LINE_INFO_t lineInfo[TOTAL_LINES] = {{0,0}, {0,0}};

static uint32 displayHash(const char* text, uint8 txtlen)
{
    uint32 hash = 0;
    uint32 chr;
    int shift = 0;
    uint8 pos;

    if(text && txtlen)
    {
        switch (txtlen)
        {
            default:
            case 5:
                chr = text[4];
                hash ^= chr>>7;
                hash ^= chr<<25;
            case 4:
                chr = text[3];
                hash ^= chr<<19;
            case 3:
                chr = text[2];
                hash ^= chr<<13;
            case 2:
                chr = text[1];
                hash ^= chr<<7;
            case 1:
                hash ^= text[0];
        }
        for (pos=5; pos<txtlen; ++pos)
        {
            uint32 topbits;
            uint16 botbits;
            shift += 11;
            if (shift>31)
                shift -= 32;
            chr = text[pos];
            if (shift>16)
            {
                chr <<= (shift-16);
                topbits = chr&0xffff;
                botbits = (chr>>16)&0xffff;
            }
            else
            {
                chr <<= shift;
                botbits = chr&0xffff;
                topbits = (chr>>16)&0xffff;
            }
            hash ^= topbits<<16 | botbits;
        }
    }
    return hash;
}

/****************************************************************************
NAME
     displayGetResetPio()

    DESCRIPTION
    Gets the LCD Display reset PIO

RETURNS
    uint8

*/
static uint8 displayGetResetPio(void)
{
    sink_display_config_def_t* ro_config_data = NULL;
    uint8 pio_value = LCD_INVALID_PIO;

    if(configManagerGetReadOnlyConfig(SINK_DISPLAY_CONFIG_BLK_ID, (const void **)&ro_config_data))
    {
       pio_value = ro_config_data->disp_reset_pio;
       configManagerReleaseConfig(SINK_DISPLAY_CONFIG_BLK_ID);
    }
    return pio_value;
}

/****************************************************************************
NAME
     displayGetBacklightPio()

    DESCRIPTION
    Gets the LCD Display backlight PIO

RETURNS
    uint8

*/
static uint8 displayGetBacklightPio(void)
{
    sink_display_config_def_t* ro_config_data = NULL;
    uint8 pio_value = LCD_INVALID_PIO;

    if(configManagerGetReadOnlyConfig(SINK_DISPLAY_CONFIG_BLK_ID, (const void **)&ro_config_data))
    {
       pio_value = ro_config_data->disp_backlight_pio;
       configManagerReleaseConfig(SINK_DISPLAY_CONFIG_BLK_ID);
    }
    return pio_value;
}

/****************************************************************************
NAME
    displayInit

DESCRIPTION
    Initialise the display

RETURNS
    void
*/
void displayInit(void)
{
    DISPLAY_DEBUG(("DISPLAY: INIT\n"));
    DisplayInit(DISPLAY_PLUGIN, &theSink.task, displayGetResetPio(), displayGetBacklightPio());
}


/****************************************************************************
NAME
    displayIconInit

DESCRIPTION
    Initialise the display's Icon area.

RETURNS
    void
*/
void displayIconInit(void)
{
    DisplaySetStatusLocation(LOWER_LINE, END_OF_LINE);
}

/****************************************************************************
NAME
    displaySetState

DESCRIPTION
    Turn display off (state FALSE) or on (state TRUE)

RETURNS
    void
*/
void displaySetState(bool state)
{
    DISPLAY_DEBUG(("DISPLAY: state %u\n", state));
    DisplaySetState(state);
}


/****************************************************************************
NAME
    displayShowText

DESCRIPTION
    Display text on display

RETURNS
    void
*/
void displayShowText(const char* text,
                        uint8 txtlen,
                        bool  scroll,
                        uint16 scroll_update,
                        uint16 scroll_pause,
                        bool  flash,
                        uint16 display_time,
                        uint8 type)
{
    const uint8 line = typeToLine[type];
    const uint8 lineI = line-1;
    DISPLAY_DEBUG(("DISPLAY: text %u \"%*s\" @ %u  (%u, %u)\n", txtlen, txtlen, text, line, type, priority[type]));
    if (priority[lineInfo[lineI].type] <= priority[type])
    {
        uint32 hash = displayHash(text, txtlen);
        if ((type != lineInfo[lineI].type) || (hash != lineInfo[lineI].hash))
        {
            DisplaySetText(text, txtlen, line, scroll, scroll_update, scroll_pause, flash, display_time);
            lineInfo[lineI].type = type;
            lineInfo[lineI].hash = hash;
        }
    }
}


/****************************************************************************
NAME
    displayShowSimpleText

DESCRIPTION
    Simplified interface to display text

RETURNS
    void
*/
void displayShowSimpleText(const char* text, uint8 type)
{
    displayShowText(text, strlen(text), DISPLAY_TEXT_SCROLL_SCROLL, 1000, 2000, FALSE, 0, type);
}


static void doDisplayRemoveText(const char* text,
                            uint8 txtlen,
                            uint8 line,
                            uint8 type)
{
    const uint8 lineI = line-1;
    if (type == lineInfo[lineI].type)
    {
        if (((NULL==text) && (0==txtlen)) || (displayHash(text, txtlen) == lineInfo[lineI].hash))
        {
            DisplaySetText(NULL, 0, line, FALSE, 0, 0, FALSE, 0);
            lineInfo[lineI].type = SINK_TEXT_TYPE_NONE;
            lineInfo[lineI].hash = 0;
        }
    }
}


/****************************************************************************
NAME
    displayRemoveText

DESCRIPTION
    Remove text of particular type from line

RETURNS
    void
*/
void displayRemoveText(uint8 type)
{
    const uint8 line = typeToLine[type];
    DISPLAY_DEBUG(("DISPLAY: blank @ %u  (%u, %u)\n", line, type, priority[type]));
    doDisplayRemoveText(NULL, 0, typeToLine[type], type);
    UNUSED(line);                           /* avoid unused warning for NOT DISPLAY_DEBUG */
}


/****************************************************************************
NAME
    displayUpdateIcon

DESCRIPTION
    Updates the state of an icon on the display

RETURNS
    void
*/
void displayUpdateIcon(uint8 icon, bool state)
{
    DISPLAY_DEBUG(("DISPLAY: icon %u = %u\n", icon, state));
    DisplaySetIcon(icon, state);
}


/****************************************************************************
NAME
    displayUpdateVolume

DESCRIPTION
    Updates the state of the volume on the display

RETURNS
    void
*/
void displayUpdateVolume( int16 vol )
{
    DISPLAY_DEBUG(("DISPLAY: vol %u\n", vol));
    DisplaySetVolume(vol);
}


/****************************************************************************
NAME
    displayUpdateBatteryLevel

DESCRIPTION
    Updates the state of the battery level on the display

RETURNS
    void
*/
void displayUpdateBatteryLevel(bool charging)
{
    if (charging)
    {
        DISPLAY_DEBUG(("DISPLAY: batt charging\n"));
        DisplaySetBatteryLevel(0xff);
    }
    else
    {   /* update battery display */
        voltage_reading reading;
        PowerBatteryGetVoltage(&reading);
        DISPLAY_DEBUG(("DISPLAY: batt %u", reading.level));
        DisplaySetBatteryLevel(reading.level);
    }
}

/****************************************************************************
NAME
    displayUpdateAppState

DESCRIPTION
    Updates the the display with the application state

RETURNS
    void
*/
void displayUpdateAppState (sinkState newState)
{
    const uint8 type = SINK_TEXT_TYPE_DEVICE_STATE;
    DISPLAY_DEBUG(("DISPLAY: state %u\n", newState));

    switch (newState)
    {
        case deviceLimbo:
            displayRemoveText(type);
            break;
        case deviceConnectable:
            doDisplayRemoveText(DISPLAYSTR_CONNECTED, strlen(DISPLAYSTR_CONNECTED), typeToLine[type], type);
            break;
        case deviceConnDiscoverable:
            displayShowSimpleText(DISPLAYSTR_PAIRING,type);
            break;
        case deviceConnected:
            {
                /* only display connected if moving out of limbo/connectable/conn-disc */
                if (newState > stateManagerGetState())
                    displayShowText(DISPLAYSTR_CONNECTED, strlen(DISPLAYSTR_CONNECTED), DISPLAY_TEXT_SCROLL_SCROLL, 500, 1000, FALSE, 20, type);
            }
            break;
        case deviceOutgoingCallEstablish:
            displayShowSimpleText(DISPLAYSTR_OUTGOINGCALL,type);
            break;
        case deviceIncomingCallEstablish:
            displayShowSimpleText(DISPLAYSTR_INCOMINGCALL,type);
            break;
        case deviceActiveCallSCO:
            displayShowSimpleText(DISPLAYSTR_ACTIVECALL,type);
            break;
        case deviceTestMode:
            displayShowSimpleText(DISPLAYSTR_TESTMODE,type);
            break;
        case deviceThreeWayCallWaiting:
            displayShowSimpleText(DISPLAYSTR_TWCWAITING,type);
            break;
        case deviceThreeWayCallOnHold:
            displayShowSimpleText(DISPLAYSTR_TWCONHOLD,type);
            break;
        case deviceThreeWayMulticall:
            displayShowSimpleText(DISPLAYSTR_TWCMULTI,type);
            break;
        case deviceIncomingCallOnHold:
            displayShowSimpleText(DISPLAYSTR_INCOMINGONHOLD,type);
            break;
        case deviceActiveCallNoSCO:
            if(usbIsCurrentVoiceSinkUsb())
            {
                displayShowSimpleText(DISPLAYSTR_ACTIVECALLNOSCO_USB,type);
            }
            else /* Current voice sink is hfp */
            {
                displayShowSimpleText(DISPLAYSTR_ACTIVECALLNOSCO,type);
            }
            break;
        case deviceA2DPStreaming:
            break;
        default:
            displayRemoveText(type);
    }
}

/****************************************************************************
NAME
    displayUpdateAudioSourceText

DESCRIPTION
    Updates the display with the current routed source name.

RETURNS
    void
*/
void displayUpdateAudioSourceText(audio_sources source)
{
    const uint8 type = SINK_TEXT_TYPE_AUDIO_SOURCE;
    switch(source)
    {
        case audio_source_FM:
            displayShowSimpleText("FM",type);
            break;
        case audio_source_ANALOG:
            displayShowSimpleText("Analogue",type);
            break;
        case audio_source_SPDIF:
            displayShowSimpleText("SPDIF",type);
            break;
        case audio_source_I2S:
            displayShowSimpleText("I2S",type);
            break;
        case audio_source_USB:
            displayShowSimpleText("USB",type);
            break;
        case audio_source_a2dp_1:
            displayShowSimpleText("A2DP 1",type);
            break;
        case audio_source_a2dp_2:
            displayShowSimpleText("A2DP 2",type);
            break;
        default:
            displayRemoveText(type);
            break;
    }
}


/****************************************************************************
NAME
    displayShowLinkKeyEnable

DESCRIPTION
    Enables the feature to allow the display to show the link keys of the connected devices
    (test purposes for capturing air traces).

RETURNS
    void
*/
void displayShowLinkKeyEnable(void)
{
    sinkDataSetDisplayLinkKeys(TRUE);
}


/****************************************************************************
NAME
    displayShowLinkKeyDisable

DESCRIPTION
    Disables the feature to allow the display to show the link keys of the connected devices
    (test purposes for capturing air traces).

RETURNS
    void
*/
void displayShowLinkKeyDisable(void)
{
    sinkDataSetDisplayLinkKeys(FALSE);
}


/****************************************************************************
NAME
	displayShowLinkKey

DESCRIPTION
	Displays the link key contained in the two character arrays passed as arguments,
	one to each line of the display. This requires the status information to be hidden,
	and the key information to be shown at a high priority to avoid display corruption.

RETURNS
    void
*/
void displayShowLinkKey(const char* link_key_line1, const char* link_key_line2)
{
    if (link_key_line1 != NULL && link_key_line2 != NULL)
    {
        DisplaySetStatusLocation( HIDDEN_LINE, FALSE );
        displayShowText(link_key_line1, 16, FALSE, 0, 0, 0, 0, SINK_TEXT_TYPE_CALLER_INFO);
        displayShowText(link_key_line2, 16, TRUE, 1000, 2000, FALSE, 0, SINK_TEXT_TYPE_GATT_INFO);
    }
}

/****************************************************************************
NAME
	displayHideLinkKey

DESCRIPTION
	Return the display to a normal state after showing a link key. Reinstates
	the status information and clears the rest of the display.

RETURNS
    void
*/
void displayHideLinkKey(void)
{
    displayRemoveText(SINK_TEXT_TYPE_CALLER_INFO);
    displayRemoveText(SINK_TEXT_TYPE_GATT_INFO);
    displayIconInit();
}

#endif /*ENABLE_DISPLAY*/
