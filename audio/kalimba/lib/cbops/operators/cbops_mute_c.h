/****************************************************************************
 * Copyright (c) 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_mute_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_MUTE_H_
#define _CBOPS_MUTE_H_

/****************************************************************************
Public Type Declarations
*/

/** Structure of the mute cbop operator specific data */
typedef struct cbops_mute
{
    /* mute flag to be set by the user, any non zero value will mute */
    unsigned mute_enable;

    /* current mute state (internal)*/
    unsigned mute_state;
}cbops_mute;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_mute_table[];

#endif /* _CBOPS_MUTE_H_ */
