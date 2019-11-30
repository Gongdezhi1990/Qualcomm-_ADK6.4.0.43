/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\file       message_broker.h
\brief      Interface to the Message Broker.

            The Message Broker allows client Application Modules to register interest
            in System Messages by Message Group. The Message Broker sniffs Messages
            sent in the system, and if they belong to the interested Group, forwards
            a copy to the interested client component.

            An example message sequence chart for providing an LED indication:

            @startuml
            skinparam roundcorner 20
            hide footbox
            participant "<<UI_Provider>>\nTelephony" as tel #Thistle
            participant "TaskList" as tl
            participant "Message Broker" as mb
            participant "Message Loop" as ml
            participant "<<UI_Indicator>>\nLEDs" as led #LightBlue
            group Initialisation
                mb  -> tl: TaskList_RegisterMessageSniffer(message_sniffer_t)
                tl --> mb
                ...
                led -> mb: MessageBroker_RegisterInterestInMsgGroups(led_task, led_grps, num_grps)
                mb --> led
            end group
            ...
            tel -> tl: TaskList_SendMessage("Telephony Connected")
            tl -> mb: MessageBroker_SniffMessage("Telephony Connected")
            mb -> ml: MessageSend(led_task, "Telephony Connected")
            ml --> mb
            mb --> tel
            ...
            ml -> led: "Telephony Connected"
            note over led
                Flash LED to indicate connection
            end note
            led --> ml
            ...
            @enduml

*/
#ifndef MESSAGE_BROKER_H
#define MESSAGE_BROKER_H

#include <csrtypes.h>
#include <message.h>

#define MSG_GRP_TO_ID(x)        ((x)<<8)
#define ID_TO_MSG_GRP(x)        ((x)>>8)

/*! \brief MessageBroker_Init
*/
void MessageBroker_Init(void);

/*! \brief MessageBroker_SniffMessage
    \param id
    \param data
    \param size_data

    Sniff message creates duplicates of any interested message sniffed that has data associated with it.
    The copy is forwarded to the registered client. The original message passed to this API is not passed
    to MessageSend and will not be deallocated.
*/
void MessageBroker_SniffMessage(MessageId id, void *data, size_t size_data);

/*! \brief MessageBroker_RegisterInterestInMsgGroups
    \param task the client's message handler, i.e. where the message broker shall send the interested messages to
    \param msg_groups pointer to the array of message groups the client is registering interest in
    \param num_groups the number of message groups the client is registering interest in
*/
void MessageBroker_RegisterInterestInMsgGroups(Task task, uint16* msg_groups, unsigned num_groups);

#endif /* MESSAGE_BROKER_H */

