tabAsSpaces = '    '

# XML Element tags
pinNameTag = 'pinFriendlyName'
padTag = 'pad'
isChargerTag = 'pioIsCharger'
isVregTag = 'pioIsVreg'
isVirtualTag = 'pioIsVirtual'
messageNameTag = 'messageName'
buttonEventTag = 'buttonEvent'
activePinTag = 'activePinFriendlyName'
negatePinTag = 'negatePinFriendlyName'
timeoutTag = 'timeout_ms'
repeatTag = 'repeat_ms'

eventHandlerHeader = """Task pioEventHandler_Init(void);

"""

eventHandlerHeaderUnitTest = """Task pioEventHandler_Init(void);

#ifdef WIN32
#include "Test\pio_generator_tests_helper.h"
#define ProcessEvent(event)	RecordEvent(event)
#else
#include "stdio.h"
#define ProcessEvent(event) printf(event)
#endif

"""

eventHandlerSourceStart="""#include <csrtypes.h>
#include <vmtypes.h>
#include <message.h>
#include <panic.h>
#include "input_event_manager.h"

/* Generated file headers */
#include "{0}_handler.h"
#include "{1}_config.h"
#include "{2}_data.h"

static void button_message_handler(Task pTask, MessageId pId, Message pMessage)
{{
    UNUSED(pTask);
    UNUSED(pMessage);

    switch(pId)
    {{
"""
eventHandlerSourceEndUnitTest =\
"""		case PIO_RAW:
		{
			pio_bits_t raw_bits = *(pio_bits_t*)pMessage;
			if(raw_bits == 0x200000)
			{
				ProcessEvent("PIO_RAW:AUX:ASSERTED");
			}
			else if(raw_bits == 0x4000000)
			{
				ProcessEvent("PIO_RAW:CHARGER_DETECT:ASSERTED");
			}
			else
			{
				ProcessEvent("PIO_RAW:NEGATED");
			}		
            break;
		}

        default:
            break;
    }
}

Task pioEventHandler_Init(void)
{
    static TaskData button_message_task = { button_message_handler };
    return InputEvent_Initialise(&button_message_task, s_testActionTable, sizeof(s_testActionTable), &s_bankConfig);
}
"""

eventHandlerSourceEnd =\
"""		case PIO_RAW:
           // TODO
		   break;
		

        default:
            break;
    }
}

Task pioEventHandler_Init(void)
{
    static TaskData button_message_task = { button_message_handler };
    return InputEvent_Initialise(&button_message_task, s_testActionTable, sizeof(s_testActionTable), &s_bankConfig);
}
"""

crapRequiredForCompilation = """/* Pio masks that requires map before use it */
/* TODO - platform specific mask that defines which bits are function multiplexed */
#define PIO_MAP_MASK     ((uint32)(0x0033F000))

"""

bankConfig = """static const BankConfig s_bankConfig =
{
	MSG_BITS,
	VIRTUAL_PIO_MASK,
	RAW_BITS,
	CHG_MASK,
	PIO_MAP_MASK,
	CHARGER_BIT,
	VREG_BIT,
    CHG_VREG_ENABLE,
	PIO_DEBOUNCE_COUNT,
	PIO_DEBOUNCE_PERIOD,
	CHARGER_DEBOUNCE_COUNT,
	CHARGER_DEBOUNCE_PERIOD,
	BANK_NUMBER,
};

"""

defaultPioDebounceXML="""
<pio_debounce>
    <!-- Seems to work well with handling dual button events with Crescendo development board -->
    <nreads>4</nreads>S
    <period>5</period>
</pio_debounce>"""

defaultChargerDebounceXML="""
<charger_debounce>
    <nreads>1</nreads>
    <period>0</period>
</charger_debounce>"""

usage = """
Usage: buttonParseXml [-e] [-s] [-h] [-n output_file_Name] [-o output_Directory] xml_input_file xml_schema_file

Options:
    -e                          Prints an example XML file. Not to be used with any other arguments/options
    -s                          Prints a summary of the PIO event map
    -h                          Generates template files <output_file_name>_handler.c/.h for PIO event handling
    -n output_file_Name         Overrides the default name for the generated output (default=xml_input_file name)
    -o output_Directory         Overrides the default directory for the generated output (default=current_directory)

Args:
    xml_input_file              The file that contains the XML button definition
    xml_schema_file             The fully qualified path to the XML Schema definition file (xsd file)

Example 1:
    Where the files are in the current directory, generates buttons_out_config.h and buttons_out_data.h:
    buttonParseXml.py -s -h -n buttons_output buttons.xml ButtonParseXml.xsd

Example 2:
    buttonParseXml.py -e

"""

xmlExample = """<?xml version="1.0" encoding="utf-8"?>

<pio_message_map xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="pio_message_map.xsd">

  <!--Mask of pins requiring setting of function to PIO-->
  <pio_function_mask>0033F000</pio_function_mask>

  <!--Debounce can be set for PIO, if not specified default debounce values  will be used (1-count, 0-period).-->
  <pio_debounce>
    <nreads>4</nreads>
    <period>5</period>
  </pio_debounce>

  <!--Debounce can be set for charger hardware also, same defaults apply if it is not defined.-->  
  <charger_debounce>
    <nreads>2</nreads>
    <period>20</period>
  </charger_debounce>

  <!--Bind named buttons to PIO pins for reference-->
  <!--NOTE: For BlueCore variants with charger hardware, VREG_EN and CHG_EN can be used as input events-->
  <pio>
    <pinFriendlyName>CHARGER_DETECT</pinFriendlyName>
    <pad>90</pad>
    <pioIsCharger>true</pioIsCharger>
  </pio>

  <pio>
    <pinFriendlyName>POWER_SWITCH</pinFriendlyName>
    <pad>64</pad>
    <pioIsVreg>true</pioIsVreg>
  </pio>

  <!--Virtual inputs can be specified. These are events from the application which act like button press/releases. 
  A virtual event is triggered by a call to the InputEvent_SetVirtualPio() function from the client application.-->
  <pio>
    <pinFriendlyName>VIRTUAL_PIO</pinFriendlyName>
    <pad>78</pad>
    <pioIsVirtual>true</pioIsVirtual>
  </pio>
  
  <pio>
    <pinFriendlyName>AUX</pinFriendlyName>
    <pad>85</pad>
  </pio>

  <pio>
    <pinFriendlyName>VOL_PLUS</pinFriendlyName>
    <pad>81</pad>
  </pio>
  
  <pio>
    <pinFriendlyName>VOL_MINUS</pinFriendlyName>
    <pad>80</pad>
  </pio>
 
  <pio>
    <pinFriendlyName>PLAY_STOP</pinFriendlyName>
    <pad>76</pad>
  </pio>

  <pio>
    <pinFriendlyName>FWD</pinFriendlyName>
    <pad>77</pad>
  </pio>

  <pio>
    <pinFriendlyName>BACK</pinFriendlyName>
    <pad>79</pad>
  </pio>


  <!--Messages are triggered when the specified PIO match the specified state defined for that message.

  Messages have 'actions' which control when the message will be sent, the actions are as follows...
      enter        - send a message when the PIO state changes to match
      release      - send a message when the PIO state changes to not match
      held         - send a message when the PIO has been in the state for the specified time period
      held_release - start a timer when the PIO enters the specified state.
                     if the PIO changes state AFTER the timer expires then send the message when the PIO 
                     state changes back.
      double       - send a message when the PIO state changes to match, twice within a specified time period 
                      (double button tap).

  With the 'enter' and 'held' actions a 'repeat' modifier can be used so that the message is repeatedly sent 
  while a button is held down.-->

  <!--Send FWD_ENTER when PIO alias FWD is ON, and repeat the message every 500ms that the FWD button is held down-->
  <message>
    <messageName>FWD_ENTER</messageName>
    <buttonEvent>ENTER</buttonEvent>
    <activePinFriendlyName>FWD</activePinFriendlyName>
    <repeat_ms>500</repeat_ms>
  </message>

  <!--send FWD_RELEASE when PIO 14 is OFF-->
  <message>
    <messageName>FWD_RELEASE</messageName>
    <buttonEvent>RELEASE</buttonEvent>
    <activePinFriendlyName>FWD</activePinFriendlyName>
  </message>

  <!--held and held_release actions can be defined multiple times for the same PIO-->
  <!--The following will result in the following behaviour if PIO alias PLAY_STOP is held down for 7-seconds:
      The PLAY_STOP_HELD_2000 message will be received after 2-seconds
      The PLAY_STOP_HELD_4000 message will be received after 4-seconds
      The PLAY_STOP_HELD_6000 message will be received after 6-seconds
      The PLAY_STOP_HELD_RELEASE_6000 message will be received when the button is release.
      The PLAY_STOP_HELD_RELEASE_2000 and 4000 messages are not recieved.-->
  <message>
    <messageName>PLAY_STOP_HELD_2000</messageName>
    <buttonEvent>HELD</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>2000</timeout_ms>
  </message>

  <message>
    <messageName>PLAY_STOP_HELD_4000</messageName>
    <buttonEvent>HELD</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>4000</timeout_ms>
  </message>

  <message>
    <messageName>PLAY_STOP_HELD_6000</messageName>
    <buttonEvent>HELD</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>6000</timeout_ms>
  </message>

  <message>
    <messageName>PLAY_STOP_HELD_RELEASE_2000</messageName>
    <buttonEvent>HELD_RELEASE</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>2000</timeout_ms>
  </message>

  <message>
    <messageName>PLAY_STOP_HELD_RELEASE_4000</messageName>
    <buttonEvent>HELD_RELEASE</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>4000</timeout_ms>
  </message>

  <message>
    <messageName>PLAY_STOP_HELD_RELEASE_6000</messageName>
    <buttonEvent>HELD_RELEASE</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>6000</timeout_ms>
  </message>
  
  <message>
    <messageName>PLAY_STOP_DOUBLE</messageName>
    <buttonEvent>DOUBLE</buttonEvent>
    <activePinFriendlyName>PLAY_STOP</activePinFriendlyName>
    <timeout_ms>1000</timeout_ms>
  </message>

  <!--More than one PIO can be used to trigger a message e.g. allowing messages triggered when 2 (or possibly more) 
  buttons are pressed at the same time. e.g.-->   
  <message>
    <messageName>MUTE_ON</messageName>
    <buttonEvent>ENTER</buttonEvent>
    <activePinFriendlyName>VOL_PLUS</activePinFriendlyName>
    <activePinFriendlyName>VOL_MINUS</activePinFriendlyName>
  </message>

  <message>
    <messageName>MUTE_OFF</messageName>
    <buttonEvent>RELEASE</buttonEvent>
    <activePinFriendlyName>VOL_PLUS</activePinFriendlyName>
    <activePinFriendlyName>VOL_MINUS</activePinFriendlyName>
  </message>

  <!--This can be a problem when those individual PIO also cause messages e.g.
  VOL_PLUS - triggers a VOL_UP message and/or VOL_MINUS - triggers a VOL_DOWN message. Unless both PIO are activated
  at exactly the same time then all 3-messages may be received! Using the debounce can help avoid this.

  Not only can the PIO that trigger a message be specified but the required state other PIO.

  To ensure that no accidental VOL_UP or VOL_DOWN messages are triggered when trying to activate VOL_PLUS and VOL_MINUS
  together, these messages can be specified as follows...-->
  <message>
    <messageName>VOL_DOWN</messageName>
    <buttonEvent>ENTER</buttonEvent>
    <activePinFriendlyName>VOL_MINUS</activePinFriendlyName>
    <negatePinFriendlyName>VOL_PLUS</negatePinFriendlyName>
    <repeat_ms>500</repeat_ms>
  </message>
  
  <message>
    <messageName>VOL_UP</messageName>
    <buttonEvent>ENTER</buttonEvent>
    <activePinFriendlyName>VOL_PLUS</activePinFriendlyName>
    <negatePinFriendlyName>VOL_MINUS</negatePinFriendlyName>
    <repeat_ms>500</repeat_ms>
  </message>

  <message>
    <messageName>VOL_DOWN_RELEASE</messageName>
    <buttonEvent>RELEASE</buttonEvent>
    <activePinFriendlyName>VOL_MINUS</activePinFriendlyName>
  </message>

  <message>
    <messageName>VOL_UP_RELEASE</messageName>
    <buttonEvent>RELEASE</buttonEvent>
    <activePinFriendlyName>VOL_PLUS</activePinFriendlyName>
  </message>
  
  <message>
    <messageName>VIRTUAL_PIO_ENTERED</messageName>
    <buttonEvent>ENTER</buttonEvent>
    <activePinFriendlyName>VIRTUAL_PIO</activePinFriendlyName>
  </message>

  <message>
    <messageName>VIRTUAL_PIO_RELEASED</messageName>
    <buttonEvent>RELEASE</buttonEvent>
    <activePinFriendlyName>VIRTUAL_PIO</activePinFriendlyName>
  </message>
  
  <message>
    <messageName>POWER_HOOK</messageName>
    <buttonEvent>ENTER</buttonEvent>
    <activePinFriendlyName>POWER_SWITCH</activePinFriendlyName>
  </message>
  

</pio_message_map>"""
