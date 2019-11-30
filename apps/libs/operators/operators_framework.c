/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    operators_framework.c

DESCRIPTION
    Handles operator framework enable/disable functionalities.
	Delays audio SS poweroff

*/
#include <stdlib.h>
#include <string.h>
#include <vmal.h>
#include <panic.h>
#include <vmtypes.h>

#include <operators.h>
#include "operators_framework.h"
#include <print.h>

#define DELAY_DISABLE_OPERATOR_FRAMEWORK_MS D_SEC(30)

#define OPERATORS_INTERNAL_MSG_BASE (0x0)
#define LAST_INSTATNCE_REMAINING   (0x01)

/* operator internal messages */
typedef enum
{
    OPERATORS_FRAMEWORK_DISABLE = OPERATORS_INTERNAL_MSG_BASE,
    OPERATORS_MSG_TOP
} operator_private_msg_t;

static void OperatorsFrameworkMessageHandler ( Task task, MessageId id, Message message );

const TaskData operators_framework_plugin = { OperatorsFrameworkMessageHandler };
static uint8 operatorEnabledCount = 0;
/****************************************************************************/
void operatorsFrameworkEnablePrivate(void)
{

    /* If there is a operator disable message pending no need
     * to enable the operator again just cancel the message.
     */
    if(!MessageCancelFirst((Task)&operators_framework_plugin, OPERATORS_FRAMEWORK_DISABLE))
    {
        PRINT(("OP: Power On Audio\n"));
        PanicFalse(VmalOperatorFrameworkEnableMainProcessor(TRUE));
        operatorEnabledCount++;
    }
}

/****************************************************************************/
void operatorsFrameworkDisablePrivate(void)
{
    if(operatorEnabledCount == LAST_INSTATNCE_REMAINING)
    {
        PRINT(("OP: Sending Delayed FW Disable\n"));
        MessageSendLater((Task)&operators_framework_plugin, OPERATORS_FRAMEWORK_DISABLE, NULL, DELAY_DISABLE_OPERATOR_FRAMEWORK_MS);
    }
    else
    {
       PRINT(("OP: FW Disable\n"));
       operatorEnabledCount--;
       PanicFalse(VmalOperatorFrameworkEnableMainProcessor(FALSE));
    }
}

/****************************************************************************/
static void OperatorsFrameworkMessageHandler ( Task task, MessageId id, Message message )
{
    UNUSED(task);
    UNUSED(message);
    switch(id)
    {
        case OPERATORS_FRAMEWORK_DISABLE:
            PRINT(("OP: Power Off Audio\n"));
            operatorEnabledCount--;
            PanicFalse(VmalOperatorFrameworkEnableMainProcessor(FALSE));
        break;
        
        default:
        break;
    }
}
