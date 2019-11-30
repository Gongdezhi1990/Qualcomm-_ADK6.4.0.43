/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup status events for Kymera 
 *
 * \file sys_event.h
 * \ingroup system
 *
 */

#ifndef SYS_EVENT_H
#define SYS_EVENT_H


/****************************************************************************
Public Constant Declarations
*/

#define SYS_EVENT_OP_CREATE         0x0001
#define SYS_EVENT_OP_DESTROY        0x0002
#define SYS_EVENT_OP_START          0x0004
#define SYS_EVENT_OP_STOP           0x0008
#define SYS_EVENT_OP_RESET          0x0010

#define SYS_EVENT_EP_CONNECT        0x0020
#define SYS_EVENT_EP_DISCONNECT     0x0040

#define SYS_EVENT_REAL_EP_CREATE    0x0100
#define SYS_EVENT_REAL_EP_DESTROY   0x0200

#define SYS_EVENT_UCID_CHANGE       0x0400

#define SYS_EVENT_PS_READY          0x1000
#define SYS_EVENT_LICENCE_READY     0x2000
/* Whether profiler-related commands are available in this firmware build
 * (GET_MIPS_USAGE_REQ, ENABLE_PROFILER_REQ) */
#define SYS_EVENT_MIPS_AVAILABLE    0x4000

#define SYS_EVENT_RESET_MASK        0xF000


/****************************************************************************
Public Function Definitions
*/

/**
 * \brief  Set the system status
 *
 * \param  sys_evt - event flag
 */
void set_system_event(unsigned sys_evt);

/**
 * \brief  Clear the system status
 *
 * \param  sys_evt - event flag
 */
void clr_system_event(unsigned sys_evt);

/**
 * \brief  Get system event status
 */
unsigned  get_system_event(void);

/**
 * \brief  Initialize event status
 */
void  init_system_event(void);



#endif /* SYS_EVENT_H */
