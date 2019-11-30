/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbops_dc_remove_c.h
 * \ingroup cbops
 *
 */

#ifndef _CBOPS_DC_REMOVE_H_
#define _CBOPS_DC_REMOVE_H_

/****************************************************************************
Public Type Declarations
*/

/** Structure of the dc_remove multi-channel cbop operator specific data */
typedef struct cbops_dc_remove
{
    /*  There is one param per channel, some channels may be unused after create.
     *  Assumption is that at least one channel is in use, otherwise why bother...
     *  The truly rigorous abstract way would have been to declare this field with
     *  [], however as it is the only field in the struct, based on the standard one
     *  cannot guarantee that all compuilers will properly handle it (it should
     *  really mean a sizeof(cbops_dc_remove) returning 0, but who knows what
     *  some compiler may do).
     */
    int dc_estimate[1];
}cbops_dc_remove;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_dc_remove_table[];

#endif /* _CBOPS_DC_REMOVE_H_ */
