/*
Copyright (c) 2005 - 2019 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
    This is the button interpreter for the Sink device
    This file extracts the button messages from the PIO subsystem and figures out
    the button press type and time. It passes the information to the button manager
    which is responsible for translating the button press generated into a system event

*/
#include "sink_main_task.h"
#include "sink_buttonmanager.h"
#include "sink_buttons.h"
#include "sink_events.h"
#include "sink_powermanager.h"
#include "sink_statemanager.h"
#include "sink_sport_health.h"
#include "sink_scan.h" 
#include "sink_pio.h"
#include "sink_usb.h"

#include "sink_debug.h"

#ifdef ENABLE_GAIA
#include "sink_gaia.h"
#endif

#include <pio_common.h>
#include <charger.h>
#include <csrtypes.h>
#include <panic.h>
#include <stdlib.h>
#include <power.h>
#include <psu.h>
#include <stddef.h>

#ifdef ENABLE_CAPACITIVE_SENSOR
#include <capsense.h>
#endif

#ifdef DEBUG_BUTTONS
#define B_DEBUG(x) DEBUG(x)
#else
#define B_DEBUG(x) 
#endif

/* Capacitive sensor sense hardware is not available for a short time after fw boot, ignore any messages generated
   during this time */
#define CAPACITIVE_SENSOR_INIT_INTERVAL 250

typedef enum ButtonsIntMsgTag 
{
    B_MESSAGE_CAPACITIVE_SENSOR_ENABLE,
    B_MULTIPLE_TIMER , 
    B_INTERNAL_TIMER , 
    B_REPEAT_TIMER  
}ButtonsIntMsg_t;

/*
    List of all physical inputs avaliable.
*/
typedef enum __physical_input
{
    physical_input_pio_0 = 0,
    physical_input_pio_1,
    physical_input_pio_2,
    physical_input_pio_3,
    physical_input_pio_4,
    physical_input_pio_5,
    physical_input_pio_6,
    physical_input_pio_7,
    physical_input_pio_8,
    physical_input_pio_9,
    physical_input_pio_10,
    physical_input_pio_11,
    physical_input_pio_12,
    physical_input_pio_13,
    physical_input_pio_14,
    physical_input_pio_15,
    physical_input_pio_16,
    physical_input_pio_17,
    physical_input_pio_18,
    physical_input_pio_19,
    physical_input_pio_20,
    physical_input_pio_21,
    physical_input_pio_22,
    physical_input_pio_23,
    physical_input_pio_24,
    physical_input_pio_25,
    physical_input_pio_26,
    physical_input_pio_27,
    physical_input_pio_28,
    physical_input_pio_29,
    physical_input_pio_30,
    physical_input_pio_31,
    physical_input_pio_32,
    physical_input_pio_33,
    physical_input_pio_34,
    physical_input_pio_35,
    physical_input_pio_36,
    physical_input_pio_37,
    physical_input_pio_38,
    physical_input_pio_39,
    physical_input_pio_40,
    physical_input_pio_41,
    physical_input_pio_42,
    physical_input_pio_43,
    physical_input_pio_44,
    physical_input_pio_45,
    physical_input_pio_46,
    physical_input_pio_47,
    physical_input_pio_48,
    physical_input_pio_49,
    physical_input_pio_50,
    physical_input_pio_51,
    physical_input_pio_52,
    physical_input_pio_53,
    physical_input_pio_54,
    physical_input_pio_55,
    physical_input_pio_56,
    physical_input_pio_57,
    physical_input_pio_58,
    physical_input_pio_59,
    physical_input_pio_60,
    physical_input_pio_61,
    physical_input_pio_62,
    physical_input_pio_63,
    physical_input_pio_64,
    physical_input_pio_65,
    physical_input_pio_66,
    physical_input_pio_67,
    physical_input_pio_68,
    physical_input_pio_69,
    physical_input_pio_70,
    physical_input_pio_71,
    physical_input_pio_72,
    physical_input_pio_73,
    physical_input_pio_74,
    physical_input_pio_75,
    physical_input_pio_76,
    physical_input_pio_77,
    physical_input_pio_78,
    physical_input_pio_79,
    physical_input_pio_80,
    physical_input_pio_81,
    physical_input_pio_82,
    physical_input_pio_83,
    physical_input_pio_84,
    physical_input_pio_85,
    physical_input_pio_86,
    physical_input_pio_87,
    physical_input_pio_88,
    physical_input_pio_89,
    physical_input_pio_90,
    physical_input_pio_91,
    physical_input_pio_92,
    physical_input_pio_93,
    physical_input_pio_94,
    physical_input_pio_95,
    physical_input_cap_0 = 96,
    physical_input_cap_1,
    physical_input_cap_2,
    physical_input_cap_3,
    physical_input_cap_4,
    physical_input_cap_5,
    physical_input_cap_6,
    physical_input_cap_7,
    physical_input_cap_8,
    physical_input_cap_9
 
} physical_input_t;

typedef struct {
    uint32 flags;
} button_flags;

button_flags flagState;

#define CAPACITIVE_SENSOR_INPUT(physical_input)     (physical_input - physical_input_cap_0)  /* broken as can't mask, unless bump cap to 128 */


/*
    LOCAL FUNCTION PROTOTYPES
 */
static void ButtonsMessageHandler ( Task pTask, MessageId pId, Message pMessage )   ;
static bool ButtonsWasButtonPressed ( uint32 pOldState , uint32 pNewState) ;
static uint32 ButtonsWhichButtonChanged ( uint32 pOldState , uint32 pNewState ) ;
static void ButtonsButtonDetected (  ButtonsTaskData * pButtonsTask ,uint32 pButtonMask  , ButtonsTime_t pTime  ) ;
static void ButtonsEdgeDetect ( const uint32 pState , ButtonsTaskData * pButtonsTask ) ;  
static void ButtonsLevelDetect ( const uint32 pState , ButtonsTaskData * pButtonsTask )  ;
static void ButtonsCheckDetection(uint16 CapacitiveSensorState, pio_common_allbits *PioState);

/****************************************************************************
DESCRIPTION
    Initialises the Button Module parameters
*/  
void ButtonsInit ( void )
{
    ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();
#ifdef ENABLE_CAPACITIVE_SENSOR
    uint8 i;
    bool success;
#endif
    
    lButtonsTask->task.handler = ButtonsMessageHandler;
    
    /*connect the underlying PIO task to this task*/
    MessagePioTask(&lButtonsTask->task);

    /*Connect the underlying Charger task to this task*/
    PowerChargerRegisterTask(&lButtonsTask->task);
         
#ifdef ENABLE_CAPACITIVE_SENSOR
    /* set the update rate, currently a fast update rate to detect short touches */    
    success = CapsenseConfigure(CAPSENSE_SET_CINT_UPDATE_DIVIDER, 0);

    /* set an initial trigger level for the cap sensors, this level will depend
       upon hardware and tests will need to be carried out to determine what a particular
       implementation requires */
    for (i = 0; success && (i < BM_CAP_SENSORS); i++)
       success = CapsenseConfigurePad(i, CAPSENSE_SET_TRIGGER_LEVEL, BM_CAP_SENSOR_LOW_SENSITIVITY);
            
    B_DEBUG(("B: capacitive sensor %s\n", success ? "OK" : "FAIL: check capacitive sensor configuration preload")) ;

    /* initialise task handler for capacitive sensor events after a short delay due to spurious events
       generated from the firmware during this time */
    MessageSendLater(&lButtonsTask->task, B_MESSAGE_CAPACITIVE_SENSOR_ENABLE, 0, CAPACITIVE_SENSOR_INIT_INTERVAL);
    
#endif
}

/****************************************************************************
DESCRIPTION
    Initialises the Button Module parameters on HYDRACORE
*/  
void ButtonsInitHardware ( void )
{
#ifdef HYDRACORE
    {
        ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();
        unsigned pio;
        unsigned i;
        unsigned bank;
        pio_common_allbits mask, maskOfInvertedPIOs, PioDirection;
        bool pioUpdateRequired = FALSE;

        PioCommonBitsInit(&mask);
        PioCommonBitsInit(&maskOfInvertedPIOs);
        PioCommonBitsInit(&PioDirection); 

        for (i = 0; i < BM_NUM_BUTTON_TRANSLATIONS; i++)
        {
            pio = lButtonsTask->pTranslations[i].input_number;
            PioCommonBitsSetBit(&mask, pio);
            if (PioIsInverted(pio))
                PioCommonBitsSetBit(&maskOfInvertedPIOs, pio);
        }

        PioCommonSetMap(&mask, &mask);
        
        for(bank = 0;bank < MAX_NUMBER_OF_PIO_BANKS;bank++)
        {
            if(mask.mask[bank] != 0)
            {
                pioUpdateRequired = TRUE;
                break;
            }
        }
        
        if (pioUpdateRequired)
        {
            PioCommonSetDir(mask, PioDirection);
            PioCommonSet(mask, maskOfInvertedPIOs);
            PioCommonSetStrongBias(mask, mask);
        }
    }
#endif /* HYDRACORE */
}

/****************************************************************************
DESCRIPTION
    Called after the configuration has been read and will trigger buttons events
    if a pio has been pressed or held whilst the configuration was still being loaded
    , i.e. the power on button press    
*/
void ButtonsCheckForChangeAfterInit(bool useButtonDebounceConfiguration)
{
    uint32 input_state; /* contains translated pio and capacitive sensor bits, format is inputs */
    pio_common_allbits pioState;
    ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();
    PioCommonBitsRead(&pioState);
    PioCommonXORWithInvertMask(&pioState);
    flagState.flags = CHARGER_VREG_VALUE | CHARGER_CONNECT_VALUE;

    lButtonsTask->gBTime = B_INVALID;
    lButtonsTask->gBButtonEventState = B_EVENT_SENT;

    /* translate the pio and capacitive sensor bits into 'input' bits mask */
    input_state = ButtonsTranslate(OLD_CAPACITIVE_SENSOR_STATE(lButtonsTask), &pioState);

    /* perform a level detect looking for transistion of recently added button definition, mask inputs
       against level configured inputs to prevent false button press indications */
    ButtonsLevelDetect((input_state & lButtonsTask->gPerformInputLevelCheck), lButtonsTask);

    /* perform an edge detect looking for transistion of recently added button definition, mask inputs 
       against edge configured inputs to prevent false button press indications */
    ButtonsEdgeDetect((input_state & lButtonsTask->gPerformInputEdgeCheck), lButtonsTask);

    /* store current input states in order to be able to detect the transition of an input (pio or capacitive sensor)
       in the button handler */
    lButtonsTask->gBOldInputState = input_state;
    PioCommonBitsRead(&lButtonsTask->gOldPIOState);
    PioCommonXORWithInvertMask(&lButtonsTask->gOldPIOState);
    
    /* Only setup PIO debounce if required. */
 
    if(useButtonDebounceConfiguration)
    {
        /* Debounce required PIO lines */
        if(!PioCommonDebounceGroup(0, /* PIO debounce group 0... */
                                   &lButtonsTask->pio_debounce_mask,
                                   lButtonsTask->button_debounce_config->pio_debounce_number,
                                   lButtonsTask->button_debounce_config->pio_debounce_period_ms))
        {
            B_DEBUG(("B: **** ERROR **** PIO NOT AVAILABLE = 0x%lx\n",input_state));
#ifdef DEBUG_BUTTONS
            Panic();
#endif
        }
    }

    ChargerDebounce( (CHARGER_VREG_EVENT|CHARGER_CONNECT_EVENT), lButtonsTask->button_config->charger_debounce_number, lButtonsTask->button_config->charger_debounce_period_ms );
    B_DEBUG(("B: initial buttoncheck\n")) ;
} 
/****************************************************************************
DESCRIPTION
    the button event message handler - converts button events to the system events
*/
static void ButtonsCheckSensorInterrupt(const void* pMessage)
{
    /* Adding interrupt handler for activity monitoring */
    sport_health_interrupt_handler(pMessage);
}

/****************************************************************************
DESCRIPTION
    the button event message handler - converts button events to the system events
*/

static void ButtonsMessageHandler ( Task pTask, MessageId pId, Message pMessage ) 
{   
    ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();
    B_DEBUG(("B:Message\n")) ;
    UNUSED(pTask);

    switch ( pId )
    {
#ifdef ENABLE_CAPACITIVE_SENSOR
        
        /* delay handling of messages until capacitive sensor hardware has settled down */
        case B_MESSAGE_CAPACITIVE_SENSOR_ENABLE :
        {
            /* connect the capacitive sensor task to this task */    
            MessageCapsenseTask(&lButtonsTask->task);
        }
        break;

        /* indication of a change of one of the capacitive touch sensors */
        case MESSAGE_CAPSENSE_CHANGED : 
        {
            uint8 i;
            const MessageCapsenseChanged * lMessage = ( const MessageCapsenseChanged * ) (pMessage ) ;
            uint16 CurrentCapState = lButtonsTask->gOldCapState;
                      
            B_DEBUG(("B:Cap - Events = %x pad = %x dir = %x\n",lMessage->num_events,lMessage->event[0].pad,lMessage->event[0].direction)) ;

            /* the capacitive sensor changed message may contain more than one button press, check for other
               presses or releases */
            for(i=0;i<lMessage->num_events;i++)
            {
                /* update the state of any sensor changes, direction is inverted, 0 = down, 1 = up  */
                if(lMessage->event[i].direction == CAPSENSE_EVENT_POS)
                    CurrentCapState |= (1<<(lMessage->event[i].pad));
                /* due to the fact that the fw will not report multiple press need to maintain
                    which touch sensors are up and which are down and set/reset appropriately */
                else
                    CurrentCapState &= ~(1<<(lMessage->event[i].pad));                
                        
                /* as the touch sensors as less effective when releasing, increase sesitivity 
                   when button indicates pressed to accurately detect the release event */
                if(lMessage->event[i].direction == CAPSENSE_EVENT_POS)
                {
                   if(!CapsenseConfigurePad(lMessage->event[i].pad, CAPSENSE_SET_TRIGGER_LEVEL, BM_CAP_SENSOR_HIGH_SENSITIVITY))
                       B_DEBUG(("B:Cap - invalid threshold %d\n",BM_CAP_SENSOR_LOW_SENSITIVITY));

                   /* button gone down, it is possible to record the time in mS at which the 
                      button went down, this can be used to accurately determine how long the 
                      button was held down for */                   
                   B_DEBUG(("B:Cap - Down Time %x = %ld mS\n",i,(uint32)lMessage->event[i].time_ms)) ;
                }
                /* return to original sensitivity when releasing button */                
                else
                {
                   if(!CapsenseConfigurePad(lMessage->event[i].pad, CAPSENSE_SET_TRIGGER_LEVEL, BM_CAP_SENSOR_LOW_SENSITIVITY))
                       B_DEBUG(("B:Cap - invalid threshold %d\n",BM_CAP_SENSOR_LOW_SENSITIVITY));
                       
                   /* button gone up, the time in mS can be used to get accurate duration information */
                   B_DEBUG(("B:Cap - Up Time %x = %ld mS\n",i,(uint32)lMessage->event[i].time_ms)) ;
                }
            }      
            
            B_DEBUG(("B:Cap - state = %x\n",CurrentCapState)) ;

            /* check whether the sensor status change requires an event to be generated */
            ButtonsCheckDetection(CurrentCapState, &lButtonsTask->gOldPIOState);

            /* update the last state value */
            lButtonsTask->gOldCapState = CurrentCapState;
        }
        break;
#endif  
        
        case MESSAGE_PIO_CHANGED : 
        {
            /* The content of the message is ignored at this point, and re-read ALL 
             * the PIO values.
             * This may be a problem as another button may be being pressed, but not yet 
             * debounced */
#if defined(DEBUG_BUTTONS) && defined(DEBUG_PRINT_ENABLED)
            const MessagePioChanged * lMessage = ( const MessagePioChanged * ) (pMessage ) ;
#endif
            pio_common_allbits pioState;

            PioCommonBitsRead(&pioState);
            PioCommonXORWithInvertMask(&pioState);
            flagState.flags = CHARGER_VREG_VALUE | CHARGER_CONNECT_VALUE;
            
            B_DEBUG(("B:BMH - PIO_CHANGE: %x %x\n",lMessage->state16to31, lMessage->state)) ;
            
            /* check whether the pio status change requires an event to be generated */
            ButtonsCheckDetection(OLD_CAPACITIVE_SENSOR_STATE(lButtonsTask), &pioState);
            ButtonsCheckSensorInterrupt(pMessage);
        }
        break ;
        
        case MESSAGE_CHARGER_CHANGED:
        {
            const MessageChargerChanged *m = (const MessageChargerChanged *) (pMessage );
            pio_common_allbits pioState;
         
            B_DEBUG(("B:BMH - CHG_CHANGED: charger %d vreg_high %d\n",
                     m->charger_connected, m->vreg_en_high));

            PioCommonBitsRead(&pioState);
            PioCommonXORWithInvertMask(&pioState);
            flagState.flags = ((uint32)m->vreg_en_high << VREG_LOGICAL_INPUT_ID) | ((uint32)m->charger_connected << CHG_LOGICAL_INPUT_ID);

            /* when a charger or vreg change event is detectecd perform both an edge and level detection
               passing in only those approriately masked pios for edge or level configured buttons */
                        /* check whether the pio status change requires an event to be generated */
            ButtonsCheckDetection(OLD_CAPACITIVE_SENSOR_STATE(lButtonsTask), &pioState);
        }
        break;

#ifdef ENABLE_CHARGER_V2
        case MESSAGE_CHARGER_DETECTED:
        {
            const MessageChargerDetected *m = (const MessageChargerDetected *) (pMessage);
            UNUSED(m);
            B_DEBUG(("B: Rcv MESSAGE_CHARGER_DETECTED with Type %x, D+ %umV, D- %umV, CC Advert %u\n",
                     m->attached_status, m->charger_dp_millivolts, m->charger_dm_millivolts, m->cc_status));
            usbUpdateConnectorInformation(m->attached_status, m->charger_dp_millivolts, m->charger_dm_millivolts, m->cc_status);
            usbUpdateChargeCurrent();
        }
        break;

        case MESSAGE_CHARGER_STATUS:
        {
            const MessageChargerStatus *m = (const MessageChargerStatus *) (pMessage);
            UNUSED(m);
            B_DEBUG(("B: Rcv MESSAGE_CHARGER_STATUS with status %u\n", m->chg_status));
            powerManagerHandleChargerStatus(m->chg_status);
        }
        break;

#endif /* ENABLE_CHARGER_V2 */

        case B_MULTIPLE_TIMER:
        {
                /*if we have reached here, then a double timer has been received*/
            B_DEBUG(("B:Double[%lx][%x]\n", lButtonsTask->gBMultipleState , B_SHORT_SINGLE)) ;
    
            /* when the multiple press timer has expired, check to see if a double press was
               made and if so indicate it */
            if(lButtonsTask->gBTapCount == DOUBLE_PRESS)
                ButtonsButtonDetected ( lButtonsTask, (lButtonsTask->gBMultipleState & lButtonsTask->gPerformInputLevelCheck) , B_DOUBLE  );                 
            /* no double press and no triple press, therefore indicate as a short single press */            
            else            
                ButtonsButtonDetected ( lButtonsTask, (lButtonsTask->gBMultipleState & lButtonsTask->gPerformInputLevelCheck), B_SHORT_SINGLE );                        
            
            /* reset the multiple button press count */
            lButtonsTask->gBTapCount = 0 ;            
            lButtonsTask->gBMultipleState = 0x0000;
        } 
        break ;
        
        case B_INTERNAL_TIMER:
        {
            /*if we have reached here, then the buttons have been held longer than one of the timed messages*/
            B_DEBUG(("B:Timer\n")) ;

            /* since a long/vlong or vvlong has been triggered, cancel any pending double press checks */
            lButtonsTask->gBMultipleState = 0x0000 ;
            lButtonsTask->gBTapCount = 0 ;
            MessageCancelAll ( &lButtonsTask->task , B_MULTIPLE_TIMER ) ;           

            /* an internal timer has triggered which was initiated from the level detect function call */
            if ( lButtonsTask->gBTime == B_VERY_LONG )
            {
                /* update timer state flag */
                lButtonsTask->gBTime = B_VERY_VERY_LONG ;                
            }
            /* a long press timer event has triggered */
            else if ( lButtonsTask->gBTime == B_LONG )
            {
                /* don't send very very long timer message until needed, i.e. very_long timer expired */
                MessageSendLater ( &lButtonsTask->task , B_INTERNAL_TIMER , 0 ,  (lButtonsTask->button_config->very_very_long_press_time - lButtonsTask->button_config->very_long_press_time ) ) ;                   
                /* update tiemr state flag */
                lButtonsTask->gBTime = B_VERY_LONG ;
                /*notify the app that the timer has expired*/
                sinkSend(EventSysVLongTimer);
            }
            /* the first timer event triggered from the level detect call */
            else
            {
                /* only send very long message when long timer expired to save messaging.                 */
                MessageSendLater ( &lButtonsTask->task , B_INTERNAL_TIMER , 0 , (lButtonsTask->button_config->very_long_press_time - lButtonsTask->button_config->long_press_time)) ;
                /*notify the app that the timer has expired*/
                sinkSend(EventSysLongTimer);
                
                /* Check for held button */
                ButtonsButtonDetected(lButtonsTask, lButtonsTask->gBOldInputState & lButtonsTask->gPerformInputLevelCheck, B_HOLD);
                
                lButtonsTask->gBTime = B_LONG ;
            }    
            /*indicate that we have received a message */
            ButtonsButtonDetected ( lButtonsTask, (lButtonsTask->gBOldInputState & lButtonsTask->gPerformInputLevelCheck) , lButtonsTask->gBTime ); 
        }         
        break ;
        
        case B_REPEAT_TIMER:
        {
            /*if we have reached here, the repeat time has been reached so send a new message*/
            B_DEBUG(("B:Repeat[%lx][%x]\n", lButtonsTask->gBOldInputState , B_REPEAT  )) ;
            
            /*send another repeat message*/
            MessageSendLater ( &lButtonsTask->task , B_REPEAT_TIMER , 0 ,  lButtonsTask->button_config->repeat_time ) ; 

            ButtonsButtonDetected ( lButtonsTask, (lButtonsTask->gBOldInputState & lButtonsTask->gPerformInputLevelCheck) , B_REPEAT ); 
        }
        break;
        default :
           B_DEBUG(("B:?[%x]\n",pId)) ; 
        break ;
    }
}



/****************************************************************************
DESCRIPTION
    helper method - returns true if a button was pressed
*/  
static bool ButtonsWasButtonPressed ( uint32 pOldState , uint32 pNewState)
{
    bool lWasButtonPressed = FALSE ;
 
    uint32 lButton = ButtonsWhichButtonChanged ( pOldState , pNewState ) ;
    
    if ( ( lButton & pNewState ) != 0 )
    {
        lWasButtonPressed = TRUE ;
    }
    
    B_DEBUG(("B:But WasPressed OldSt[%lx] NewSt[%lx] Input[%lx] Pressed[%x]\n", pOldState, pNewState, lButton,lWasButtonPressed)) ;
        
    return lWasButtonPressed ;
}
/****************************************************************************
DESCRIPTION
    helper method - returns mask ofwhich button changed
*/ 
static uint32 ButtonsWhichButtonChanged ( uint32 pOldState , uint32 pNewState )
{
    uint32 lWhichButton = 0 ;
        
    lWhichButton = (pNewState ^ pOldState ) ;
    
    return lWhichButton ;
}

/****************************************************************************  
DESCRIPTION
    function to handle a button press - informs button manager of a change
    currently makes direct call - may want to use message handling (tasks)
*/
static void ButtonsButtonDetected(ButtonsTaskData *pButtonsTask, uint32 pButtonMask, ButtonsTime_t pTime)
{
    B_DEBUG(("B:But Det[%lx]\n", pButtonMask));

    if(pButtonMask == 0)
    {
        MessageCancelAll(&pButtonsTask->task, B_REPEAT_TIMER);
    }
    else
    {
        BMButtonDetected(pButtonMask, pTime);

        if (stateManagerGetState() == deviceTestMode)
        {
            checkDUTKeyRelease(pButtonMask, pTime);
        }
    }
}

/****************************************************************************
DESCRIPTION
    function to detect level changes of buttons / multiple buttons, both 
    PIO and capacitive sensor 
*/ 
static void ButtonsLevelDetect ( const uint32 pInput, ButtonsTaskData * pButtonsTask ) 
{
    uint32 lNewInput = (uint32) (pInput & (pButtonsTask->gPerformInputLevelCheck)) ;
    uint32 lOldInput = (uint32) (pButtonsTask->gBOldInputState & pButtonsTask->gPerformInputLevelCheck);

    B_DEBUG(("But Lev Det|:NewInput[%lx] OldInput[%lx]\n", lNewInput, pButtonsTask->gBOldInputState )) ;
    
    if ( ButtonsWasButtonPressed(lOldInput, lNewInput )  )
    {
        /* check whether device needs to be made connectable as a result of a button press
           on a multipoint device */
        if(BMGetGoConnectableButtonPress())
            sinkEnableMultipointConnectable();

        /* cancel all previously timed messages */
        MessageCancelAll ( &pButtonsTask->task , B_INTERNAL_TIMER ) ;
        MessageCancelAll ( &pButtonsTask->task , B_REPEAT_TIMER ) ;

        /* send new timed messages */
        MessageSendLater ( &pButtonsTask->task , B_INTERNAL_TIMER , 0 ,  pButtonsTask->button_config->long_press_time ) ;
        MessageSendLater ( &pButtonsTask->task , B_REPEAT_TIMER   , 0 ,  pButtonsTask->button_config->repeat_time ) ;

        /* having restarted the timers, reset the time */
        pButtonsTask->gBTime = B_SHORT ;
        pButtonsTask->gBButtonEventState = B_NEWLY_PRESSED;

        if (stateManagerGetState() == deviceTestMode)
        {
            checkDUTKeyPress(lNewInput);
        }
    }
    /* button was released or was masked out, check to make sure there is an input bit change as vreg enable
       can generate an addition MSG without any input's changing state */
    else if(lOldInput!= lNewInput )
    {
        /* it was only released if there was a button actually pressed last time around - 
           buttons we have masked out, still end up here, but no state changes are made;
           but once an event is generated, don't generate another until there is a further button down press */
        if ((lOldInput != 0) && (B_NEWLY_PRESSED == pButtonsTask->gBButtonEventState))
        {
             /* if we have had a double press in the required time
                 and the button pressed was the same as this one*/
             if (  (pButtonsTask->gBTapCount ) && (lOldInput == pButtonsTask->gBMultipleState ) )
             {
                /* button has been released, increment multiple press counter */
                 pButtonsTask->gBTapCount++;

                 B_DEBUG(("TapCount: [%lx][%lx][%x]\n", lOldInput , pButtonsTask->gBMultipleState , pButtonsTask->gBTapCount  )) ;

                 /* if the multiple press count is 2, set the time as a double, this will be used
                    if no further presses are detected before the multiple press timeout occurs */
                 if(pButtonsTask->gBTapCount == DOUBLE_PRESS)
                    pButtonsTask->gBTime = B_DOUBLE ;
                 /* if the multiple press count is 3, process immediately as a triple press and cancel the 
                    multiple press timer */
                 else if(pButtonsTask->gBTapCount == TRIPLE_PRESS)
                 {
                    pButtonsTask->gBTime = B_TRIPLE ;
                    /* indicate a triple press has been detected */
                    ButtonsButtonDetected ( pButtonsTask, lOldInput , B_TRIPLE  ); 
                    /* reset current state and multiple press counter */                    
                    pButtonsTask->gBMultipleState = 0x0000 ;
                    pButtonsTask->gBTapCount = 0;
                    MessageCancelAll ( &pButtonsTask->task , B_MULTIPLE_TIMER ) ;           
                 }
             }

             /*only send a message if it was a short one - long / v long /double handled elsewhere*/
             if ( (pButtonsTask->gBTime == B_SHORT ) )
             {
                 ButtonsButtonDetected ( pButtonsTask, lOldInput , B_SHORT  ); 
                 
                 /*store the double state*/
                 pButtonsTask->gBMultipleState = lOldInput ;
                 pButtonsTask->gBTapCount = 1;

                    /*start the double timer - only applicable to a short press*/
                 MessageSendLater ( &pButtonsTask->task , B_MULTIPLE_TIMER , 0 , pButtonsTask->button_config->double_press_time ) ;
             }
             else if ( (pButtonsTask->gBTime == B_LONG) )
             {
                 ButtonsButtonDetected ( pButtonsTask, lOldInput , B_LONG_RELEASE  );
             }
             else if ( (pButtonsTask->gBTime == B_VERY_LONG) )
             {
                 ButtonsButtonDetected ( pButtonsTask, lOldInput , B_VERY_LONG_RELEASE  );
             }
             else if ( (pButtonsTask->gBTime == B_VERY_VERY_LONG) )
             {
                 ButtonsButtonDetected ( pButtonsTask, lOldInput , B_VERY_VERY_LONG_RELEASE  );
             }

             if (pButtonsTask->gBTime != B_INVALID)
             {
                MessageCancelAll ( &pButtonsTask->task , B_INTERNAL_TIMER) ;
                MessageCancelAll ( &pButtonsTask->task , B_REPEAT_TIMER ) ;
             }

             if ( !lNewInput )
             {   /* This is to prevent further key releases producing events, until the next down */
                 pButtonsTask->gBTime = B_INVALID ;
             }

             pButtonsTask->gBButtonEventState = B_EVENT_SENT;
         }
    }
}


/****************************************************************************

DESCRIPTION
    function to detect edge changes of buttons / multiple buttons. 

*/ 
static void ButtonsEdgeDetect ( const uint32 pInput,  ButtonsTaskData * pButtonsTask) 
{
    uint32 lNewInput = (uint32) (pInput & (pButtonsTask->gPerformInputEdgeCheck) ) ;
    uint32 lOldInput = (uint32) (pButtonsTask->gBOldInputState & pButtonsTask->gPerformInputEdgeCheck);
    uint32 lInput = ButtonsWhichButtonChanged( lOldInput , lNewInput ) ;
    
    B_DEBUG(("But Edge Det: Old Edge[%lx] New Edge[%lx] Button[%lx] Low2High[%lx}\n", lOldInput , lNewInput ,lInput ,(lNewInput & lInput)  )) ;
    
    /*if a button has changed*/
    if ( lInput )
    {
            /*determine which edge has been received and process accordingly*/
        if ( lNewInput & lInput )
        {  
            ButtonsButtonDetected ( pButtonsTask,  lInput , B_LOW_TO_HIGH )   ;
            /* check whether device needs to be made connectable as a result of a button press
               on a multipoint device */
            if(BMGetGoConnectableButtonPress())
                sinkEnableMultipointConnectable();
        }
        else
        {
            ButtonsButtonDetected ( pButtonsTask, lInput , B_HIGH_TO_LOW  )   ;
        }
    }
}

    
/****************************************************************************

DESCRIPTION
    function to detect edge changes of buttons / multiple buttons. 

*/ 
static void ButtonsCheckDetection(uint16 CapacitiveSensorState, pio_common_allbits *PioState)
{   
    ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();
    
    /* take the current capacitive touch state and pio state and translate into a uint32
       input state used for checking button presses */
    uint32 lInputState = ButtonsTranslate(CapacitiveSensorState, PioState);

    B_DEBUG(("But CheckDet: Inputs: Cap[%x] PIO[%lx] Inputs[%lx] OldState[%lx]\n", CapacitiveSensorState , PioState->mask[0], lInputState, lButtonsTask->gBOldInputState)) ;
    B_DEBUG(("But CheckDet: Masks:  Edge [%lx] Lev [%lx]\n",lButtonsTask->gPerformInputEdgeCheck, lButtonsTask->gPerformInputLevelCheck));

    /* when an input is configured for an edge detect only there is significant performance gain to be had
       by only doing an edge detect call and not a level detect. To do this use a previously set edge
       detect mask and check this against the current pio being reported. Also need to check if a previously
       set PIO has now been removed and check for the edge transition once again. */
    if((lButtonsTask->gPerformInputEdgeCheck & lInputState) ||
       (lButtonsTask->gPerformInputEdgeCheck & lButtonsTask->gBOldInputState))
    {
        /* check for a valid edge transition against current pio states masked with edge configured pios
           and perform appropriate action */                
        ButtonsEdgeDetect  ( lInputState, lButtonsTask) ;
    }          
            
    /* only do a level detect call which is vm/messaging intensive when a pio has been configured as 
       short or long or very long or very very long, i.e. not rising or falling */
    if((lButtonsTask->gPerformInputLevelCheck & lInputState ) ||
       (lButtonsTask->gPerformInputLevelCheck & lButtonsTask->gBOldInputState))
    {
        /* perform a level detection, this call uses a number of messages and is quite slow to process */
        ButtonsLevelDetect ( lInputState, lButtonsTask) ;            
    }

#ifdef ENABLE_GAIA
    gaiaReportPioChange(PioState);
#endif

    /* update last button state for next time around */
    lButtonsTask->gBOldInputState = lInputState;
    lButtonsTask->gOldPIOState = *PioState;
}

/****************************************************************************

DESCRIPTION
    this function remaps the capacitive sensor and pio bitmask into an input assignment
    pattern specified by pskey user 10, this allows buttons to be triggered from 
    pios of anywhere from 0 to 31 and capacitive sensor 0 to 5

*/ 
uint32 ButtonsTranslate(uint16 CapacitiveSensorState, pio_common_allbits *PioState)
{
    ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();
    uint32 lResult = 0;
    uint8 i;
 
    /* merge capacitive touch switch data and pio data using the button translation
       data, cycle through all 16 translations */
    for (i = 0; i < BM_NUM_BUTTON_TRANSLATIONS; i++)
    {
        /* if the button is assigned to a pio then check the pio state */
        if(lButtonsTask->pTranslations[i].input_number < physical_input_cap_0 )
        {
            /* if pio state is set then add to result */
             if(PioCommonBitsBitIsSet(PioState,lButtonsTask->pTranslations[i].input_number))
             {
                lResult |= ((uint32)1<<lButtonsTask->pTranslations[i].button_no);
             }
        }
        /* if not a pio check for capacitive sensor input source */
        else if(lButtonsTask->pTranslations[i].input_number >= physical_input_cap_0 )
        {
            /* if capacitive sensor state is set then add to result */
            if(CapacitiveSensorState & ((uint16)1<< CAPACITIVE_SENSOR_INPUT(lButtonsTask->pTranslations[i].input_number)))
            {
                lResult |= ((uint32)1<<lButtonsTask->pTranslations[i].button_no);
            }
        }
    }
    /* Add the flags to the translated result
     * This supports legacy behaviour
     */

    lResult |= flagState.flags;

    B_DEBUG(("But Trans: Cap[%x] Pio[%lx] Input[%lx]\n", CapacitiveSensorState , PioState->mask[0], lResult )) ;

    /* return 32 bit result as a mixture of pio and capacitive sensor inputs */
    return lResult;
}


/*
    this function remaps an input assignment into a capacitive sensor or pio bitmask
    pattern specified by the configuration.

    A pointer to the mask variable returned is supplied as a parameter

*/
pio_common_allbits *ButtonsTranslateInput(pio_common_allbits *mask, 
                                          const event_config_type_t *event_config, 
                                          bool include_capacitive_sensor)
{        
    ButtonsTaskData * lButtonsTask = BMGetSinkButtonTask();

    if (event_config && mask)
    {
        uint8 i;

        /* get current input button requirements */
        uint32 lMask = bmGetButtonEventConfigPioMask(event_config);

        PioCommonBitsInit(mask);

        /* search through the translation table for matching buttons */
        for (i = 0; i < BM_NUM_BUTTON_TRANSLATIONS; i++)
        {
            /* when matching translation found set the translated bit which will be used
               for edge and level checking on the translated pio/capacitive sensor data bits */
            if(lMask & ((uint32)1<<lButtonsTask->pTranslations[i].button_no))
            {          
                /* only include the capacitive sensor bits if specifically required */
                if((include_capacitive_sensor) ||
                   ((!include_capacitive_sensor) && (lButtonsTask->pTranslations[i].input_number < physical_input_cap_0)))
                {
                    /*! @todo Note that the use of capacitive sensor is not supported in
                     *  this release of the CSRA6810x ADK.
                     */
                    PioCommonBitsSetBit(mask,lButtonsTask->pTranslations[i].input_number);
                }
            }
        }  

        B_DEBUG(("But Trans inputs =[0x%lx]\n", lMask));
        
        return mask;
    }
   
#ifdef DEBUG_BUTTONS
    Panic();
#endif    
    return NULL;    

}
