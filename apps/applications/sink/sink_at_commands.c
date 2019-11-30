/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    Application level control of AT commands sent and received via the HFP
    connections.

    These can be configured in 2 ways,
    received AT commands that ate unrecognised by the HFP library will be passed
    to the applciation (here)

    If these match any of the commands in the cofiguration, then a response also
    in the configuration will be sent to the AG device.

    Alternatively, User events can be configured to send configured AT commands
    (from the same list) to the AG on receipt of any user event,

    E.g. This allows the user to configure an event that can be sent to the AG on
    connection or when the battery level changes.

NOTES


*/
/****************************************************************************
    Header files
*/
#include "sink_at_commands.h"
#include "sink_config.h"
#include "sink_audio.h"
#include "sink_hfp_data.h"
#include "sink_malloc_debug.h"
#include "sink_powermanager.h"
#include <string.h>
#include <byte_utils.h>

#ifdef DEBUG_AT_COMMANDS
    #define AT_DEBUG(x) DEBUG(x)
#else
    #define AT_DEBUG(x)
#endif

static const char * const mic_test_string               = "+MICTEST" ;
static const char * const mic_test_string_res_success   = "AT+MICTEST=1\r" ;
static const char * const mic_test_string_res_fail      = "AT+MICTEST=0\r" ;
static const char * const output_test_string            = "+OUTTEST";
static const char * const output_test_string_res_success= "AT+OUTTEST=1\r";
static const char * const output_test_string_res_fail   = "AT+OUTTEST=0\r";
static const uint8 battery_scaling_2[] = {0,1,3,5,7,9};

static void sinkAtCommandsCheckAndProcessAtCommandMicTest(HFP_UNRECOGNISED_AT_CMD_IND_T *ind,
                                                            const char * const command_string)
{
    if(strncmp(command_string, mic_test_string, strlen(mic_test_string)) == 0)
    {
        AT_DEBUG(("Handle Mic Switch\n"));
        if (audioHandleMicSwitch())
        {
            /* Send the success response AT command */
            AT_DEBUG(("Response %s\n", mic_test_string_res_success));
            HfpAtCmdRequest(ind->priority, mic_test_string_res_success);
        }
        else
        {
            /* Send the failed response AT command */
            AT_DEBUG(("Response %s\n", mic_test_string_res_fail));
            HfpAtCmdRequest(ind->priority, mic_test_string_res_fail);
        }
    }
}

static void sinkAtCommandsCheckAndProcessAtCommandOutputIncrement(HFP_UNRECOGNISED_AT_CMD_IND_T *ind,
                                                                    const char * const command_string)
{
    if(strncmp(command_string, output_test_string, strlen(output_test_string)) == 0)
    {
        AT_DEBUG(("Handle Output Switch\n"));
        if(audioHandleOutputSwitch())
        {
            AT_DEBUG(("Response %s\n", output_test_string_res_success));
            HfpAtCmdRequest(ind->priority, output_test_string_res_success);
        }
        else
        {
            AT_DEBUG(("Response %s\n", output_test_string_res_fail));
            HfpAtCmdRequest(ind->priority, output_test_string_res_fail);
        }
    }
}

static void sinkAtCommandsCheckAndProcessProductionTestCommands(HFP_UNRECOGNISED_AT_CMD_IND_T *ind,
                                                                const char * const command_string)
{
    sinkAtCommandsCheckAndProcessAtCommandMicTest(ind, command_string);
    sinkAtCommandsCheckAndProcessAtCommandOutputIncrement(ind, command_string);
}

static char *sinkAtCommandsGetStrings(void)
{
    char *search_string;
    uint16 at_commands_size;
    
    at_commands_size = 2 * sinkHfpDataGetAtCommandsSize();
    search_string = mallocDebugNoPanic(at_commands_size);
    if (search_string)
    {
        ByteUtilsMemCpyUnpackString((uint8 *) search_string, sinkHfpDataGetAtCommands(), at_commands_size);
    }
    
    return search_string;
}

/****************************************************************************
DESCRIPTION
    handles an unrecognised AT command from the HFP library

    Attempts to match an AT command to those configured. If configured, a response
    will be sent.

    If not configured, a response will not be sent.

    If the +MICTEST is configured, the mic test switching will take place


*/
void sinkHandleUnrecognisedATCmd(HFP_UNRECOGNISED_AT_CMD_IND_T *ind)
{
    char* pData;
    char *pAtCommandData;
    uint16 i = 0 ;
    char *pDataEnd = (char*)(ind->data + ind->size_data);

    AT_DEBUG(("HFP_UNRECOGNISED_AT_CMD_IND\n" )) ;

    /* Skip to start of AT command */
    for(pData = (char*)ind->data; pData < pDataEnd; pData++)
        if(*pData == '+' || *pData == '-')
            break;
    
    /* Truncate AT command at CR or LF */    
    for (i = 0; &pData[i] < pDataEnd && pData[i] != '\0'; i++)
    {
        if ((pData[i] == '\r') || (pData[i] == '\n'))
        {
            pData[i] = '\0';
            break;
        }
    }

    AT_DEBUG(("AT command = %s\n", pData));

    sinkAtCommandsCheckAndProcessProductionTestCommands(ind, pData);

    /* check if there's any more configured AT commands to check */
    if(!sinkHfpDataHasAtCmdsData())
    {
        AT_DEBUG(("AT: Unrecognised AT Cmd -  no config\n" )) ;
        return;
    }

    pAtCommandData = sinkAtCommandsGetStrings();

    if (pAtCommandData)
    {
        char *pSearchString = pAtCommandData;

        /* Try to match the AT command */
        while(*pSearchString != '\0')
        {
            /* Command followed by response string */
            char* res_string = pSearchString + strlen(pSearchString) + 1;
            AT_DEBUG(("Matching %s\n", pSearchString));
            
            /* If the incoming command matches; handle & respond */
            if(*pData == '+' && strncmp(pData, pSearchString, strlen(pSearchString)) == 0)
            {            
                /* Send the response AT */
                AT_DEBUG(("Response %s\n", res_string));
                HfpAtCmdRequest(ind->priority, res_string);
                break;
            }
            
            /* Move to the next command */
            pSearchString = res_string + strlen(res_string) + 1;
        }
       
        freeDebugNoPanic(pAtCommandData);
    }
}

/****************************************************************************
DESCRIPTION
    Sends a given AT command from the configuration

    works through the configuration looking for '\0' signifying end of string,
    moves to next string in list until string == '\0'

    If a string matching the id passed in, then the command is sent to the AG.
    If not, no command is sent

*/
void sinkSendATCmd( uint16 at_id )
{
    char *at_command_data;

    if(!sinkHfpDataHasAtCmdsData())
    {
        AT_DEBUG(("AT: Send Cmd -  no config\n" )) ;
        return;
    }

    at_command_data = sinkAtCommandsGetStrings();
    
    if (at_command_data)
    {
        char *search_string = at_command_data;
        uint8 count = 0;
    
        AT_DEBUG(("AT: Send Cmd\n" )) ;

        /* get to the nth string if it exists */
        while(*search_string != '\0')
        {
            AT_DEBUG(("String %s\n", search_string));
            
            if (count == at_id )
                break ;
            /* Move to the next command */
            search_string = search_string + strlen(search_string) + 1;
            count++;
        }
        /*only send the string if we actually matched the command*/
        if (count == at_id)
        {
            uint16 i = 0 ;    
            unsigned char *replace_string = (unsigned char *)search_string;
        
                /*replace any values which need replacing*/
            for (i = 0 ; replace_string[i] != '\0' ;i++)
            {
                switch (replace_string[i]) 
                {               
                    /* 0x81 - battery reporting scaling of 0,1,2,3,4,5 */
                    case at_cmd_battery_0_to_5 :
                    /* 0x82 - battery reporting scaling of 0,2,4,6,8,9 */
                    case at_cmd_battery_0_to_9_scaling_1 :
                    {
                        /*multiply the battery level (0-5) by 1 or 2 to get scale*/
                        unsigned scale_factor = (replace_string[i] - at_cmd_battery_0_to_5) + 1;
                        unsigned batt_lvl = powerManagerGetBatteryLevel() * scale_factor;
                        
                        if (batt_lvl > 9) 
                        {
                            batt_lvl = 9 ;
                        }
    
                            /*convert to ascii*/
                        replace_string[i] = (char)(batt_lvl + '0');
                    }  
                    break;
                    /* 0x83 - battery reporting scaling of 0,1,3,5,7,9 */
                    case at_cmd_battery_0_to_9_scaling_2 :
                    {
                        /* use scaling of 0,1,3,5,7,9 */
                        uint8 batt_lvl = battery_scaling_2[powerManagerGetBatteryLevel()] ;
                        /*convert to ascii*/
                        replace_string[i] = (char)(batt_lvl + '0');
                    }
                    break;
                    
                    /* 0x84 - charger connected 0,1 */
                    case at_cmd_charger_docked:
                        replace_string[i] = powerManagerIsChargerConnected() ? '1' : '0';
                        break;
                        
                    default:
                        break ;
                 }
            }
                
            AT_DEBUG(("AT Send:[%s]\n", search_string));
            
            HfpAtCmdRequest(hfp_primary_link, search_string);
            HfpAtCmdRequest(hfp_secondary_link, search_string);
        }
        
        freeDebugNoPanic(at_command_data);
    }
}


/****************************************************************************
DESCRIPTION
    Checks for a user event -> AT command match configured in CONFIG_USR_3
    If configued, AT command sent to valid HFP connections using
    sinkSendATCommand

    Up to MAX_AT_COMMANDS_TO_SEND AT commands can be stored in the PSKEY
    for Sending to the AG

    An EventSysGasGauge0 or an EventSysChargerGasGauge0 configured will result in the
    same message being sent for any of the gas gauge events.

*/
void ATCommandPlayEvent ( sinkEvents_t id )
{
    uint16 i = 0 ;

    if(!sinkHfpDataHasAtCmdsData())
    {
        AT_DEBUG(("AT: Play Cmd -  no config\n" )) ;
        return;
    }

    for (i =0; i < sinkHfpDataGetEventCount(); i++)
    {
        if ( (id) == sinkHfpDataGetEventAtCommandsEvent(i))
        {
            AT_DEBUG(("AT: Ev [%x] AT CMD [%d]" , id , sinkHfpDataGetEventAtCommands(i)));
            sinkSendATCmd(sinkHfpDataGetEventAtCommands(i));
        }

            /*special handling for gas gauge events */
        switch (sinkHfpDataGetEventAtCommandsEvent(i))
        {
            case EventSysGasGauge0:
                if ( ( id >= EventSysGasGauge0 ) && (id <= EventSysGasGauge3))
                {
                    AT_DEBUG(("AT: Ev Gas Gauge n [%x] AT CMD [%d]" , id , sinkHfpDataGetEventAtCommands(i) )) ;
                    sinkSendATCmd ( sinkHfpDataGetEventAtCommands(i) ) ;
                }
            break ;
            case EventSysChargerGasGauge0:
                if ( ( id >= EventSysChargerGasGauge0 ) && ( id <= EventSysChargerGasGauge3))
                {
                    AT_DEBUG(("AT: Ev Charger Gas Gauge n [%x] AT CMD [%d]" , id , sinkHfpDataGetEventAtCommands(i))) ;
                    sinkSendATCmd ( sinkHfpDataGetEventAtCommands(i) ) ;
                }
            break ;
            default:
            break ;
        }

    }
}


