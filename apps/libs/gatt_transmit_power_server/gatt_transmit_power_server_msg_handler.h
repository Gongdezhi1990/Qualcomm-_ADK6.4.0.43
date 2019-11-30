/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */
/* 
    FILE NAME
    gatt_transmit_power_server_msg_handler.h

DESCRIPTION
    Header file for the Transmit Power Server Service Message Handler.
*/

/*!
@file   gatt_transmit_power_server_msg_handler.h
@brief  Header file for the Transmit Power Server Service Message Handler.

        This file documents the message handler API for Transmit Power Server.
*/


#ifndef GATT_TRANSMIT_POWER_SERVER_HANDLER_H_
#define GATT_TRANSMIT_POWER_SERVER_HANDLER_H_

/***************************************************************************
NAME
    transmit_power_server_ext_msg_handler

DESCRIPTION
    Handler for external messages sent to the library.
*/
void transmit_power_server_ext_msg_handler(Task task, MessageId id, Message msg);

#endif /*GATT_TRANSMIT_POWER_SERVER_HANDLER_H_*/

