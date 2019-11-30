/*****************************************************************************

            (c) Qualcomm Technologies International, Ltd. 2017
            Confidential information of Qualcomm

            Refer to LICENSE.txt included with this source for details
            on the license terms.

            WARNING: This is an auto-generated file!
                     DO NOT EDIT!

*****************************************************************************/
#ifndef PERSIST_PRIM_H
#define PERSIST_PRIM_H


/*******************************************************************************

  NAME
    PERSIST

  DESCRIPTION
    Persistent store ranking of data

 VALUES
    ANY                         - Persisted in any writable store. When reading,
                                  the data is attempted to be found from store
                                  with PERSIST_UNTIL_POWERDOWN ranking first and
                                  move toward stores with higher rank. Writing
                                  is attempted to lowest-ranking writable store
                                  of rank > UNTIL_POWERDOWN, if such exists on
                                  the platform. Otherwise a write to
                                  PERSIST_UNTIL_POWERDOWN is the last resort.
    UNTIL_POWERDOWN             - Persisted in current powercycle only
    ACROSS_POWERCYCLES          - Persisted between power cycles
    ACROSS_POWERCYCLES_DEFAULTS - Persisted between power cycles and acting as
                                  "factory defaults"

*******************************************************************************/
typedef enum
{
    PERSIST_ANY = 0,
    PERSIST_UNTIL_POWERDOWN = 1,
    PERSIST_ACROSS_POWERCYCLES = 2,
    PERSIST_ACROSS_POWERCYCLES_DEFAULTS = 3
} PERSIST;


/*******************************************************************************

  NAME
    PERSIST_ID_TYPE

  DESCRIPTION
    Data type of the PS key values. Unfortunately current script only
    generates output for items with same name as filename as preamble, there
    are pros and cons for this. However, actual name of the interface type is
    re-instated in the ps.h header, which is to be used across Kymera (hence
    internal to DSP).

*******************************************************************************/
typedef uint24 PERSIST_ID_TYPE;


#define PERSIST_PRIM_ANY_SIZE 1


#endif /* PERSIST_PRIM_H */

