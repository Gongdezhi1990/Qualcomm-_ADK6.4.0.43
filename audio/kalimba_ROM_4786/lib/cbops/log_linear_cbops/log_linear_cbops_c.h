/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file log_linear_cbops_c.h
 * \ingroup cbops
 *
 */

#ifndef _LOG_LINEAR_CBOPS_C_H_
#define _LOG_LINEAR_CBOPS_C_H_

/****************************************************************************
Public Type Declarations
*/
/** Structure of the log linear cbop operator specific data */
typedef struct
{
    /** Function pointer field for this type of encoding conversion */
    void* g711_func_ptr_field;
}cbops_g711;

/** The address of the function vector table. This is aliased in ASM */
extern unsigned cbops_g711_table[];


/****************************************************************************
Public Function Declarations
*/
/** The address of the a law to linear conversion function */
extern unsigned g711_alaw2linear(unsigned val);
/** The address of the a law to linear conversion function */
extern unsigned g711_linear2alaw(unsigned val);
/** The address of the u law to linear conversion function */
extern unsigned g711_ulaw2linear(unsigned val);
/** The address of the u law to linear conversion function */
extern unsigned g711_linear2ulaw(unsigned val);

#endif /* _LOG_LINEAR_CBOPS_C_H_ */
