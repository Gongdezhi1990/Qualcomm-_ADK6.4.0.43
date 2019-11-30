/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implementation for the udiv3216 fuction.
 */

#include "optim/optim.h"

uint32 udiv3216(uint16 *r, uint16 d, uint32 n)
{
    if (r != NULL)
    {
        *r = n % d;
    }

    return n / d;
}
