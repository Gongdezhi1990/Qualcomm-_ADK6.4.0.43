/*****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama_receive_command.h

DESCRIPTION
    Passes incoming command packet to respective handler function
*/
#ifndef __AMA_RECEIVE_COMMAND_H_
#define __AMA_RECEIVE_COMMAND_H_

#include <csrtypes.h>


/***************************************************************************
DESCRIPTION
    Passes incoming command packet to respective handler function
 
PARAMS
    data Pointer to the incoming data
    length Length of incoming data in octets

*/
void amaReceiveCommand(char* data, uint16 length);


#endif /* __AMA_RECEIVE_COMMAND_H_ */

