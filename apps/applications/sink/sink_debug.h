/***************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_debug.h
    
DESCRIPTION
    
*/

#ifndef _SINK_DEBUG_H_
#define _SINK_DEBUG_H_


#ifndef RELEASE_BUILD /*allows the release build to ensure all of the below are removed*/
    
#include <stdio.h>

    /*The individual configs*/
    #define DEFAULT_CONFIG_CSR_STEREO
    

#ifndef DO_NOT_DOCUMENT

#endif 
 /*end of DO_NOT_DOCUMENT*/

    /*The global debug enable*/ 
    #define DEBUG_PRINT_ENABLEDx
    /* We want debug print turned on on
       For hydracore at the moment */
    #define DEBUG_PRINT_ENABLED
    #define DEBUG_MAIN
    #define DEBUG_INIT
    #define DEBUG_DORMANT


#define DEBUG_VM_HATSx

#ifdef DEBUG_VM_HATS
/*************************************************************************
NAME
	printVmLogsInTestSystem

DESCRIPTION
	This function prints logs of VM code in conjunction with HATS logs if
	it is enabled.

RETURNS
	void
*/
void printVmLogsInTestSystem (const char *format, ...);

#define PRINT_BUFF_SIZE      50
#define DEBUG(x)   {printVmLogsInTestSystem x;}

#endif /*DEBUG_VM_HATS*/

    #ifdef DEBUG_PRINT_ENABLED

        #ifndef DEBUG_VM_HATS
            #define DEBUG(x) {printf x;}
        #endif


        /* The individual Debug enables */

        #define DEBUG_MAIN_L1x
        #define DEBUG_A2DPx
        #define DEBUG_ACCESSORYx
        #define DEBUG_AHIx
        #define DEBUG_AMA
        /* Debug Always-on-Voice (AoV) module */
        #define DEBUG_AOVx
        #define DEBUG_AT_COMMANDSx
        #define DEBUG_AUDIO_PROMPTSx
        #define DEBUG_AUDIOx
        #define DEBUG_AUTHx
        #define DEBUG_AUTO_POWER_OFFx
        #define DEBUG_AVRCPx
        #define DEBUG_BA_BROADCASTERx
        /* Broadcast Audio Plugin Debug */
        #define DEBUG_BA_PLUGINx
        #define DEBUG_BA_RECEIVERx
        /* Battery Reporting Debug */
        #define DEBUG_BAT_REPx
        /* Broadcast Audio Debug */
        #define DEBUG_BA_COMMONx
        #define DEBUG_BLE_GAPx
        #define DEBUG_BLE_SCx
        /* BLE transport / messages Debug */
        #define DEBUG_BLEx
        /* Button Manager debug */
        #define DEBUG_BUT_MANx
        /* Low Level Button Parsing Debug */
        #define DEBUG_BUTTONSx
        /* Call Manager Debug */
        #define DEBUG_CALL_MANx
        /* Config Manager Debug */
        #define DEBUG_CONFIGx
        #define DEBUG_CSR2CSRx
        /* sink_devicemanager.c Debug */
        #define DEBUG_DEVx
        #define DEBUG_DFU_PSx
        /* Dimming LEDs */
        #define DEBUG_DIMx
        #define DEBUG_DISPLAYx
        /* Device ID */
        #define DEBUG_DIx
        #define DEBUG_DORMANTx
        #define DEBUG_DOWNLOADABLE_CAPSx
        #define DEBUG_DSP_CAPABILITIESx
        #define DEBUG_DUTx
        /* Debug external charger interface */
        #define DEBUG_EXT_CHGx
        #define DEBUG_FAST_PAIRx
        #define DEBUG_FILTER_ENABLEx
        #define DEBUG_FMx
        #define DEBUG_GAIAx
        #define DEBUG_GATT_AMAx
        #define DEBUG_GATT_AMS_CLIENTx
        #define DEBUG_GATT_AMS_PROXYx
        #define DEBUG_GATT_ANCS_CLIENTx
        #define DEBUG_GATT_ANCS_PROXYx
        #define DEBUG_GATT_BATTERY_CLIENTx
        #define DEBUG_GATT_BATTERY_SERVERx
        #define DEBUG_GATT_BISTO_COMM_SERVERx
        #define DEBUG_GATT_CLIENTx
        #define DEBUG_GATT_FP_SERVERx
        #define DEBUG_GATT_DIS_CLIENTx
        #define DEBUG_GATT_HID_CLIENTx
        #define DEBUG_GATT_HID_QUALIFICATIONx
        /* HID Remote Control Debug */
        #define DEBUG_GATT_HID_RCx
        #define DEBUG_GATT_HRS_CLIENTx
        #define DEBUG_GATT_IAS_CLIENTx
        /* Link Loss Server Debug */
        #define DEBUG_GATT_LLS_SERVERx
        #define DEBUG_GATT_MANAGERx
        #define DEBUG_GATT_SERVICE_CLIENTx
        #define DEBUG_GATT_SPC_CLIENTx
        #define DEBUG_GATTx
        #define DEBUG_HIDx
        #define DEBUG_INITx
        #define DEBUG_INPUT_MANAGERx
        /* RSSI Pairing Debug */
        #define DEBUG_INQx
        /* IR Remote Control Debug */
        #define DEBUG_IR_RCx
        /* Lower Level LED Drive Debug */
        #define DEBUG_LEDSx
        #define DEBUG_LINKLOSSx
        /* LED Manager Debug */
        #define DEBUG_LMx
        /* Sink Link Policy Debug */
        #define DEBUG_LPx
        /* Main System Messages Debug */
        #define DEBUG_MAINx
        #define DEBUG_MALLOCx 
        #define DEBUG_MAPCx
        /* Multi-Point Manager Debug */
        #define DEBUG_MULTI_MANx
        #define DEBUG_NFCx
        #define DEBUG_PBAPx
        #define DEBUG_PEER_SMx
        #define DEBUG_PEERx
        /* Lower Level PIO Drive Debug */
        #define DEBUG_PIOx
        /* Power Manager Debug */
        #define DEBUG_POWERx
        /* BR EDR Secure Connection Debug */
        #define DEBUG_SCx
        #define DEBUG_SLCx
        #define DEBUG_SPEECH_RECx
        /* State Manager Debug */
        #define DEBUG_STATESx
        /* Debug Subwoofer Debug */
        #define DEBUG_SWATx
        #define DEBUG_TONESx
        #define DEBUG_USBx

        /* Debug Voice Assistant (VA) module */
        #define DEBUG_VAx

        #define DEBUG_VOLUMEx
        #define DEBUG_WIREDx

        /* Debug Always-on-Voice (AoV) module */
        #define DEBUG_AOVx
        #define DEBUG_AUDIO_CLOCKx

        /* Debug Bisto OTA module */
        #define DEBUG_BISTO_OTAx
        /* Debug Bisto battery module */
        #define DEBUG_BISTO_BATTERYx
    #else
        #ifndef DEBUG_VM_HATS
            #define DEBUG(x)
        #endif
    #endif /*DEBUG_PRINT_ENABLED*/

        /* If you want to carry out cVc license key checking in Production test
           Then this needs to be enabled */
    #define CVC_PRODTESTx

	/* Audio Prompt/Tone Sink Event Queue debug */
	#define DEBUG_AUDIO_PROMPTS_TONES_QUEUEx

	/*Audio Indication module to play prompt and tones*/
	#define DEBUG_AUDIO_INDICATIONx


#else /*RELEASE_BUILD*/    

 /*used by the build script to include the debug but none of the 
          individual debug components*/
		#ifndef DEBUG_VM_HATS
			#ifdef DEBUG_BUILD 
				#define DEBUG(x) {printf x;}
			#else
				#define DEBUG(x) 
			#endif /*DEBUG_BUILD*/
		#endif /*DEBUG_VM_HATS*/    
#endif /*RELEASE_BUILD*/

#ifdef DEBUG_PEER
	#define PEER_DEBUG(x) DEBUG(x)
#else
	#define PEER_DEBUG(x) 
#endif /*DEBUG_PEER*/



#define INSTALL_PANIC_CHECK
#define NO_BOOST_CHARGE_TRAPS
#undef SINK_USB
#define HAVE_VBAT_SEL
#define HAVE_FULL_USB_CHARGER_DETECTION

#define LOG_ERROR(x) {printf x;}

#endif /*_SINK_DEBUG_H_*/
