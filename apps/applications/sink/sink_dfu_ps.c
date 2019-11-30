/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_dfu_ps.c

DESCRIPTION
    This file contains example implementation to modify Audio PS keys and Non-Config PS keys on a successful DFU, for customer reference.
    Customers will need to implement SinkDfuPsUpdate() according to their specific needs.
    
NOTES

*/


#ifdef ENABLE_DFU_PS
#include "sink_dfu_ps.h"
#include "sink_configmanager.h"
#include "sink_upgrade.h"
#include "sink_debug.h"
#include <ps.h>

#ifdef DEBUG_DFU_PS
#define DFU_PS_DEBUG(x) DEBUG(x)
#else
#define DFU_PS_DEBUG(x)
#endif


/*Example to update the entire audio PS key*/
#define AUDIO_KEY1 0x2480
#define AUDIO_KEY1_WORDS 31
#define AUDIO_KEY1_WORDSTOWRITE 31
#define AUDIO_KEY1_BUFFERDATA  {0x1006,0x0002,0x0001,0x0000,0x0001,0x0004,0x0004,0x0000,0x0000,0x0065,0xC000,0x0010,0x0000,0x00CC,0xCCCC,0x0011,0x0001,0x0067,0x9000,0x0013,0x0001,0x00CC,0xCCCC,0x0015,0x0001,0x00B9,0x5000,0x0017,0x0001,0x00CC,0xCCCC}

/*******************************************************************************
NAME
    sinkDfuPsPrintAudioKey
	
DESCRIPTION
    Prints the value of the specified Audio PS keys.

RETURNS Length of the key read.
*/

static uint16 sinkDfuPsPrintAudioKey(uint32 key, uint16 WordsToRead)
{
    uint16 readBuffer[WordsToRead];
    uint16 keyLength;
    uint16 WordsRead;

    WordsRead = PsReadAudioKey(key, readBuffer, WordsToRead, 0, &keyLength);

    if(WordsRead == 0)
    {
        DFU_PS_DEBUG(("DFU_PS: Read operation failed\n "));
        return 0;
    }

    if(WordsToRead <= keyLength)
    {
        DFU_PS_DEBUG(("DFU_PS: Key %04x has data ", key));

        for(int i=0;i<WordsToRead;i++)
        {
            DFU_PS_DEBUG((" %04x ", readBuffer[i]));
        }

        DFU_PS_DEBUG(("DFU_PS :\n"));
    }
    else
    {
        DFU_PS_DEBUG(("DFU_PS: Number of words to be read is greater than the Key length.PSkey data will not be displayed\n"));
        return 0;
    }

    return keyLength;
}

/*******************************************************************************
NAME
    sinkDfuPsUpdateAudioKey
	
DESCRIPTION
    Updates the Audio PS keys after the upgrade.

RETURNS None
*/

static void sinkDfuPsUpdateAudioKey(uint32 key, uint16 *buffer, uint16 WordsToWrite)
{
    uint16 keyLength;
    uint16 wordsUpdated;

    DFU_PS_DEBUG(("DFU_PS: Pskey %04x data before updating\n", key));

    keyLength=sinkDfuPsPrintAudioKey(key, WordsToWrite);

    if(keyLength == 0)
    {
        DFU_PS_DEBUG(("DFU_PS: Audio key %04x does not exist\n",key));
        DFU_PS_DEBUG(("DFU_PS: New Audio key will be created\n"));
        keyLength = WordsToWrite;
    }

    if(WordsToWrite <= keyLength)
    {
        wordsUpdated = PsUpdateAudioKey(key, buffer, WordsToWrite, 0, keyLength);

        if(wordsUpdated == 0)
        {
            DFU_PS_DEBUG(("DFU_PS: Write operation failed.Audio keys are not updated.\n"));
            return;
        }
        else
        {
            DFU_PS_DEBUG(("DFU_PS: Pskey %04x data After updating\n", key));
            sinkDfuPsPrintAudioKey(key, WordsToWrite);
        }
    }
    else
    {
        DFU_PS_DEBUG(("DFU_PS: Words to be written is more than the key length.Audio key is not updated\n"));
        return;
    }
}


/*******************************************************************************
NAME
     sinkDfuPsNonConfigKeyBAMode()

DESCRIPTION
    Updates a Non Config PS key - BA mode as an example
    
RETURNS
    void

*/
static void sinkDfuPsNonConfigKeyBAMode(void)
{
    uint16 config_item;
    uint16 read_data_size;
    
    read_data_size = PsRetrieve(CONFIG_BROADCAST_AUDIO_MODE, (void*)&config_item, sizeof(config_item));
    DFU_PS_DEBUG(("DFU_PS: BA mode (Before updating) = 0x%x\n", config_item));

    if (read_data_size)
    {
#define DFU_PS_BA_MODE 1

        if (config_item != DFU_PS_BA_MODE)
        {
            config_item =  DFU_PS_BA_MODE;
            PsStore(CONFIG_BROADCAST_AUDIO_MODE, (const void*)&config_item, sizeof(config_item));

            /*Read again to display*/
#ifdef DEBUG_DFU_PS
            config_item = 0;
            read_data_size = PsRetrieve(CONFIG_BROADCAST_AUDIO_MODE, (void*)&config_item, sizeof(config_item));
            DFU_PS_DEBUG(("DFU_PS: BA mode (After updating) = 0x%x\n", config_item));
#endif
        }
        else
        {
            DFU_PS_DEBUG(("DFU_PS: Config Item value same as before \n"));
        }
    }
    else
    {
        DFU_PS_DEBUG(("DFU_PS: Update failed as the key is not initialised yet (make sure the module where the key is updated is enabled) \n"));
    }
}


/*******************************************************************************
NAME
    sinkDfuPsNonConfigKeys

DESCRIPTION
    Updates the non config PS keys after the upgrade.
    This function provides example implementation only.

RETURNS None
    
*/
static void sinkDfuPsNonConfigKeys(void)
{
    /*This is example-only functionality: DO NOT USE as is*/
    sinkDfuPsNonConfigKeyBAMode();
}

    SinkDfuPsUpdate

/*******************************************************************************/
/*
    NOTE:  
    This is an intentional build error introduced to alert the user/customers to 
    change the implementation in SinkDfuPsUpdate().  

    The below implementation is for reference alone.

    Customers are expected to change the implementation in SinkDfuPsUpdate() 
    suiting their specific needs and NOT use the function as is.

    To build the feature UNCOMMENT code here in the function definition.
*/

/*void*/ SinkDfuPsUpdate(void)
{
    if(SinkUpgradeRunningNewImage())
    {
        uint16 Key1_writeBuffer[AUDIO_KEY1_WORDS] = AUDIO_KEY1_BUFFERDATA;

        /*Example to update Audio PS key*/
        sinkDfuPsUpdateAudioKey(AUDIO_KEY1, Key1_writeBuffer, AUDIO_KEY1_WORDSTOWRITE);

        /*Example to update non-config PS key*/
        sinkDfuPsNonConfigKeys();
    }
    else
    {
        DFU_PS_DEBUG(("DFU_PS: No Audio PS key updated \n"));
    }
}


#endif /*ENABLE_DFU_PS*/
