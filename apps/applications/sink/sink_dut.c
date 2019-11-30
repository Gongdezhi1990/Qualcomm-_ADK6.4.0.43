/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_dut.c

DESCRIPTION
    Place the sink device into Device Under Test (DUT) mode
*/


/****************************************************************************
    Header files
*/
#include "sink_dut.h"
#include "sink_devicemanager.h"
#include "sink_device_id.h"
#include "sink_config.h"
#include "sink_pio.h"
#include "sink_tones.h"
#include "sink_audio_routing.h"
#include "sink_hfp_data.h"
#include "sink_main_task.h"
#include "sink_powermanager.h"

#include <audio.h>

/* Include config store and definition headers */
#include "config_definition.h"
#include "sink_dut_config_def.h"
#include <config_store.h>

#include <kalimba.h>
#include <file.h>
#include <led.h>
#include <test.h>


#include <ps.h>
#include <string.h>
#include "sink_configmanager.h"
#include "sink_audio_routing.h"


#ifdef CVC_PRODTEST
#include <audio_plugin_if.h>
#endif

#ifdef DEBUG_DUT
#define DUT_DEBUG(x) DEBUG(x)
#else
#define DUT_DEBUG(x) 
#endif


#ifdef CVC_PRODTEST
    #define CVC_PRODTEST_PASS           0x0001
    #define CVC_PRODTEST_FAIL           0x0002
    #define CVC_PRODTEST_NO_CHECK       0x0003
    #define CVC_PRODTEST_FILE_NOT_FOUND 0x0004

    typedef struct
    {
        uint16 id;
        uint16 a;
        uint16 b;
        uint16 c;
        uint16 d;
    } DSP_REGISTER_T;
#endif


#define TX_START_TEST_MODE_LO_FREQ  (2441)
#define TX_START_TEST_MODE_LEVEL    (63)
#define TX_START_TEST_MODE_MOD_FREQ (0)
    


typedef enum
{
    key_test_led0_on,
    key_test_led0_off,
    key_test_led1_on,
    key_test_led1_off
} key_test_led_state;


typedef struct
{
    unsigned led:2;
    unsigned mode:2;
    unsigned unused:12;
} DUT_T;

static DUT_T dut;
#ifdef CVC_PRODTEST
typedef enum
{
    test_cvc,
    test_aptx,
    test_aptx_sprint,
    test_gaia
} security_test_type;

static security_test_type test_type;
#endif

/****************************************************************************
DESCRIPTION
    Gets the currently active DUT mode
*/
dut_test_mode getDUTMode(void)
{
    return dut.mode;
}

static bool isDutAudioActive(void)
{
    if (getDUTMode() == dut_test_audio)
    {
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
DESCRIPTION
    Sets the currently active DUT mode
*/
static void setDUTMode(dut_test_mode mode)
{
    if(isDutAudioActive())
    {
    }

    dut.mode = mode;   
}

/****************************************************************************
DESCRIPTION
    Sets the currently active test LED
*/
static void setDUTLed(key_test_led_state led)
{
    dut.led = led;
}
        


/****************************************************************************
DESCRIPTION
    This function is called to place the sink device  into DUT mode
*/
void enterDutMode(void)
{
    /* set test mode */
    setDUTMode(dut_test_dut);
    
    ConnectionEnterDutMode();
}

/****************************************************************************
DESCRIPTION
    Entera continuous transmit test mode
*/
void enterTxContinuousTestMode ( void )
{
    /* set test mode */
    setDUTMode(dut_test_tx);
    
    TestTxStart (TX_START_TEST_MODE_LO_FREQ,
                 TX_START_TEST_MODE_LEVEL,
                 TX_START_TEST_MODE_MOD_FREQ) ;
}



void dutInit(void)
{
    setDUTMode(dut_test_invalid);
    setDUTLed(key_test_led0_on);
}

void dutDisconnect(void)
{
    setDUTMode(dut_test_invalid);
}
