/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************
FILE
        uart_if.h  - HID interface

CONTAINS
        The global definitions needed for configuring the UART

DESCRIPTION
    This file is seen by the stack, and VM applications, and
    contains things that are common between them.
*/

#ifndef __APP_UART_IF_H__
#define __APP_UART_IF_H__

/*!
    @brief The UART baud rate.
*/
typedef enum
{
    VM_UART_RATE_SAME       = 0,          /*!< The same.*/
    VM_UART_RATE_9K6        = 39,         /*!< 9600 baud.*/
    VM_UART_RATE_19K2       = 79,         /*!< 19200 baud.*/
    VM_UART_RATE_38K4       = 157,        /*!< 38400 baud.*/
    VM_UART_RATE_57K6       = 236,        /*!< 57600 baud.*/
    VM_UART_RATE_115K2      = 472,        /*!< 115200 baud.*/
    VM_UART_RATE_230K4      = 944,        /*!< 230400 baud.*/
    VM_UART_RATE_460K8      = 1887,       /*!< 460800 baud.*/
    VM_UART_RATE_921K6      = 3775,       /*!< 921600 baud.*/
    VM_UART_RATE_1382K4     = 5662        /*!< 1382400 baud.*/
} vm_uart_rate;

/*!
    @brief The number of stop bits.
*/
typedef enum
{
    VM_UART_STOP_ONE,       /*!< One. */
    VM_UART_STOP_TWO,       /*!< Two. */
    VM_UART_STOP_SAME       /*!< The same. */
} vm_uart_stop;

/*!
    @brief The parity to use.
*/
typedef enum
{
    VM_UART_PARITY_NONE,    /*!< None.*/
    VM_UART_PARITY_ODD,     /*!< Odd. */
    VM_UART_PARITY_EVEN,    /*!< Even. */
    VM_UART_PARITY_SAME     /*!< The same. */
} vm_uart_parity;

#endif
