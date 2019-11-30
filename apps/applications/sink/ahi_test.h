/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi_test.h

DESCRIPTION
    Application module that handles AHI related operations. For example:

    Intercepting events and echoing them back to the Host when running
    automated tests.

*/

#ifndef AHI_TEST_H_
#define AHI_TEST_H_

#ifdef ENABLE_AHI_TEST_WRAPPER
void ahiTestEchoConnectionMessage(MessageId id, Message message);
void ahiTestEchoHfpMessage(MessageId id, Message message);
void ahiTestEchoAvrcpMessage(MessageId id, Message message);
void ahiTestEchoA2dpMessage(MessageId id, Message message);
#else
#define ahiTestEchoConnectionMessage(id, message) ((void)(0))
#define ahiTestEchoHfpMessage(id, message) ((void)(0))
#define ahiTestEchoAvrcpMessage(id, message) ((void)(0))
#define ahiTestEchoA2dpMessage(id, message) ((void)(0))
#endif

#endif /* AHI_TEST_H_ */
