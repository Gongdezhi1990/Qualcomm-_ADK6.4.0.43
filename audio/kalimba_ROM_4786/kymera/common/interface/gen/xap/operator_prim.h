/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef OPERATOR_PRIM_H
#define OPERATOR_PRIM_H


/*******************************************************************************

  NAME
    OPERATOR_CREATE_EX

  DESCRIPTION
    Create operator (extended) configuration keys

 VALUES
    OPERATOR_CREATE_NO_KEY       - no extension create_op
    OPERATOR_CREATE_OP_PRIORITY  - operator priority
    OPERATOR_CREATE_PROCESSOR_ID - processor ID

*******************************************************************************/
typedef enum
{
    OPERATOR_CREATE_NO_KEY = 0,
    OPERATOR_CREATE_OP_PRIORITY = 1,
    OPERATOR_CREATE_PROCESSOR_ID = 2
} OPERATOR_CREATE_EX;


#define OPERATOR_PRIM_ANY_SIZE 1


#endif /* OPERATOR_PRIM_H */

