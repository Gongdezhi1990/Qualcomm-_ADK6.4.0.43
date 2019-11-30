/****************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    csr_i2s_CSRA6620_plugin.c
DESCRIPTION
    plugin implentation which has chip specific i2c commands to configure the 
    i2s audio output device for use 
NOTES
*/

#include "audio_i2s_CSRA6620.h"

#include <audio.h>
#include <gain_utils.h>
#include <stdlib.h>
#include <panic.h>
#include <file.h>
#include <print.h>
#include <stream.h> 
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include <message.h>
#include <transform.h>
#include <string.h>
#include <i2c.h>
#include <stdio.h>
 



#define gcsra6620PlugInTask (&csra6620_plugin)

static void message_handler(Task task, MessageId id, Message message);
static uint16 CSRA6220_write(uint16 address, uint8 txdata);
static uint16 CSRA6220_read(uint16 address, uint8* rxdata);
TaskData csra6620_plugin = {message_handler};

static uint8 config_step_index;

/****************************************************************************
DESCRIPTION: AudioI2SInitialise_CSRA6620 :

    This function configures the I2S device 

PARAMETERS:
    
    uint32 sample_rate - sample rate of data coming from dsp

RETURNS:
    none
*/    
void AudioI2SInitialise_CSRA6620(uint32 sample_rate)
{
	config_step_index = 0;
    UNUSED(sample_rate); /* Not handled yet */

    /* Start polling for complete boot status after short delay */
    PRINT(("CSRA6620: Start polling for complete boot status after short delay \n"));
    MessageSendLater(gcsra6620PlugInTask, MESSAGE_CHECK_SW_BOOT_STATUS, 0, 20); 
    
}

void handleNextConfigStep(void)
{
    bool allConfigurationStepsComplete = FALSE;
    uint8 delay_ms = 10; /* default delay between messages */
 
    uint8 messageToPoll= 0; /* 0 means no message is polled */    
    
    switch(config_step_index)
    {
        case 0:
            CSRA6220_write(CHIP_STATE_CTRL_FA, CTRL_VALUE_FOR_CONFIG_MODE);  /*7085*/
            PRINT(("CSRA6620: Step 1:Sending Config Mode => 0 Message\n"));
        break;
        
        case 1:
        {
            delay_ms = 10;
            PRINT(("CSRA6620: Step 2:Delaying for %d mS\n",delay_ms));
        }
        break;
       
        case 2:
        {
            CSRA6220_write(TEMP_PROT_BACKOFF, CTRL_VALUE_FOR_TEMP_PROTECTION);  /*7082*/                        
            PRINT(("CSRA6620: Step 3:Sending TEMP_PROT_BACKOFF settings\n"));
        }
        break;
        
        case 3:
        {   
            CSRA6220_write(EXT_PA_PROTECT_POLARITY, CTRL_VALUE_FOR_PA_POLARITY_PROTECTION);  /*707d*/                       
            PRINT(("CSRA6620: Step 4:Sending EXT_PA_PROTECT_POLARITY settings\n"));
        }
        break;
        
        case 4:
        {
            CSRA6220_write(PWM_OUTPUT_CONFIG, CTRL_VALUE_FOR_PWM_OUTPUT_CONFIG);  /*7087*/ 
            PRINT(("CSRA6620: Step 5:Sending PWM_OUTPUT_CONFIG settings\n"));
        }
        break;
        
        case 5:
        {
            delay_ms = 10;
            PRINT(("CSRA6620: Step 6:Delaying for %d mS\n",delay_ms));
        }
        break;
        
        case 6:
        {
            CSRA6220_write(CHIP_STATE_CTRL_FA, CTRL_VALUE_FOR_ALLOW_RUN);  /*7085*/ 
            PRINT(("CSRA6620: Step 7:Sending ALLOW_RUN indication\n"));
        }
        break;
        
        case 7:        
        {
            
            PRINT(("CSRA6620: Sending volume defaults")); 
            AudioI2SSetVolume_CSRA6620(DEFAULT_LEFT_VOLUME_DB, 
                                           DEFAULT_RIGHT_VOLUME_DB, 
                                           TRUE); 
        }
        break;
        
        default:
        {

            allConfigurationStepsComplete = TRUE;
            PRINT(("CSRA6620: Initialisation Configuration Complete.\n"));
        }
        break;
        
    }

    if(messageToPoll)
    {
        MessageSendLater(gcsra6620PlugInTask, messageToPoll, 0, messageToPoll);
    }
    else if(!allConfigurationStepsComplete)
    {  /* No specific polling message required */
        MessageSendLater(gcsra6620PlugInTask, MESSAGE_DELAY_COMPLETE, 0, delay_ms);
    }
    
    config_step_index++;

}

/****************************************************************************
DESCRIPTION
    Helper function to encapulate the I2C write functionality
*/
static uint16 CSRA6220_write(uint16 address, uint8 data)
{
    uint16 txResult = 0;
    uint8 i2c_data[3];
    const uint8 num_message_bytes = 3;
    i2c_data[0] = (uint8) (address >> 8);
    i2c_data[1] = (uint8) (address & 0x00FF);
    i2c_data[2] = data;      
    txResult = I2cTransfer(ADDR_CSRA6620_7BITS, i2c_data, num_message_bytes, NULL, 0);
    PanicZero(txResult);
    return txResult;
}

/****************************************************************************
DESCRIPTION
    Helper function to encapulate the I2C read functionality
*/
static uint16 CSRA6220_read(uint16 address, uint8* rxData)
{
    uint8 i2c_tx_data[2];  /* only need address bytes for polling a register */
    
    uint16 ack = 0;
    
    const uint8 sizeof_rx_buffer = 1; /* This device will only return 1 byte for single register reads */
    
    i2c_tx_data[0] = (uint8) (address >> 8);
    i2c_tx_data[1] = (uint8) (address & 0x00FF);
    
    memset(rxData, 0, sizeof_rx_buffer);
    
    ack = I2cTransfer(ADDR_CSRA6620_7BITS , i2c_tx_data, sizeof(i2c_tx_data), rxData, sizeof_rx_buffer);  
    PanicZero(ack);
    return ack;
}



/****************************************************************************
DESCRIPTION
    messages to the task are handled here
*/
static void message_handler( Task task , MessageId id, Message message )
{
    UNUSED(task);
    UNUSED(message);
    switch (id)
    {
        case MESSAGE_CHECK_SW_BOOT_STATUS:
        {
            uint16 ack = 0;
            uint8 i2c_rx_data[1]; /* only expect one bytes for polling response */
  
            ack = CSRA6220_read(CHIP_STATE_STATUS_FA,i2c_rx_data);
            
            if(ack)
            {   
                /* check if standby bit set */
                if((i2c_rx_data[0] & STANDY_MODE_VALUE) == 0) 
                {
                    PRINT(("CSRA6620: MESSAGE_CHECK_SW_BOOT_STATUS not yet indicating Standby(2). Status was %d \n",i2c_rx_data[0]));
                    MessageSendLater(gcsra6620PlugInTask, MESSAGE_CHECK_SW_BOOT_STATUS, 0, 50);
                }
                else
                {  /* Booting is over, now start polling for config mode status indication */
                    PRINT(("CSRA6620: Booting complete.\n"));                    
                    handleNextConfigStep();
                }
            }
            else
            {
                PRINT(("CSRA6620: No response from device during MESSAGE_CHECK_SW_BOOT_STATUS polling\n"));
                MessageSendLater(gcsra6620PlugInTask, MESSAGE_CHECK_SW_BOOT_STATUS, 0, 500);
            }
        }
        break ;        
           
        case MESSAGE_DELAY_COMPLETE: /*a delay period has come to pass */
        {
            handleNextConfigStep();
        }
        break;
        default:
        {
           /* Nothing yet to default to */

        }
        break ;
    }
}



/****************************************************************************
DESCRIPTION: AudioI2SSetVolume_CSRA6620

    This function sets the I2S device volume via the I2C
    
PARAMETERS:
    
    int16 left_vol - right_vol : volume level required scaled as 0 to 457 corresponding to Mute(< -90.00dB) to +24dB
    volume_in_dB : boolean which indicates that volume values are representing levels in db x 100.
    

RETURNS:
    none
*/    
void AudioI2SSetVolume_CSRA6620(int16 left_vol, int16 right_vol, bool volume_in_dB)
{   
 /* The 6620 device uses 9 bits for its volume. 
    The 9 bits are spread over two registers (i.e 2 registers for each channel)
        0x000                           MUTE                              
        0x001                           -90 dB
        0x002                           -89.75 dB
        ...                             ...
        0x169                           0 dB
        0x16a                           0.25 dB
        0x16b                           0.5 dB
        ...                             ...
        0x1c9                           24 dB
        0x1ca- 0x1ff                Invalid settings 

    This results in a transfer function of y = 4x + 361
*/    
           
#define Y_INTERCEPT      361
#define SLOPE           (100/4)
           
    if(volume_in_dB)
    {
        PRINT(("I2S: SetVol dB[%x] ", left_vol));

        if(left_vol < MIN_VOLUME_LIMIT_IN_DB)
        	left_vol = MIN_VOLUME_LIMIT_IN_DB;
        else if(left_vol > MAX_VOLUME_LIMIT_IN_DB)
            left_vol = MAX_VOLUME_LIMIT_IN_DB;

        if(right_vol < MIN_VOLUME_LIMIT_IN_DB)
        	right_vol = MIN_VOLUME_LIMIT_IN_DB;
        else if(right_vol > MAX_VOLUME_LIMIT_IN_DB)
        	right_vol = MAX_VOLUME_LIMIT_IN_DB;
        
        /* reduce resolution of dB values from dB*100 to db */
        left_vol = (int16)(left_vol / SLOPE);
        right_vol = (int16)(right_vol / SLOPE);
        /* add offet to yield positive integers only*/
        left_vol = (int16)(left_vol + Y_INTERCEPT);
        right_vol = (int16)(right_vol + Y_INTERCEPT);
        
        PRINT(("scaled[%x]\n", left_vol));
    }
    else
    {
    	if(left_vol < MIN_VOLUME_LIMIT_AS_UINT)
			left_vol = MIN_VOLUME_LIMIT_AS_UINT;
		else if(left_vol > MAX_VOLUME_LIMIT_AS_UINT)
			left_vol = MAX_VOLUME_LIMIT_AS_UINT;

		if(right_vol < MIN_VOLUME_LIMIT_AS_UINT)
			right_vol = MIN_VOLUME_LIMIT_AS_UINT;
		else if(right_vol > MAX_VOLUME_LIMIT_AS_UINT)
			right_vol = MAX_VOLUME_LIMIT_AS_UINT;
    }

    PRINT(("CSRA6620: Sending volume defaults")); 
 /* We are considering the CH1 as the LEFT Channel */
    CSRA6220_write(CH1_VOLUME_0_FA,(uint8) (left_vol & 0xFF));    /*702f*/
    CSRA6220_write(CH1_VOLUME_1_FA,(uint8)(left_vol >> 8));       /*7030*/
    
  /* We are considering the CH2 as the RIGHT Channel */   
    CSRA6220_write(CH2_VOLUME_0_FA,(uint8) (right_vol & 0xFF));   /*7031*/
    CSRA6220_write(CH2_VOLUME_1_FA,(uint8)(right_vol  >> 8));     /*7032*/
}


/****************************************************************************
DESCRIPTION: AudioI2SShutdown_CSRA6620 :

    This function stutdowns the CSRA6620 device 

PARAMETERS:
    
    none

RETURNS:
    none
*/    
void AudioI2SShutdown_CSRA6620(void)
{   
    CSRA6220_write(CHIP_STATE_CTRL_FA,CTRL_VALUE_FOR_SHUTTING_DOWN);     /*7085*/
  
    MessageCancelAll((TaskData*) gcsra6620PlugInTask, MESSAGE_CHECK_SW_BOOT_STATUS);
    MessageCancelAll((TaskData*) gcsra6620PlugInTask, MESSAGE_DELAY_COMPLETE);     
    
    PRINT(("CSRA6620: Shutdown Complete\n"));
}



