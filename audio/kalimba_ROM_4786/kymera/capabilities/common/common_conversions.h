/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/

#ifndef COMMON_CONVERSIONS_H
#define COMMON_CONVERSIONS_H

/*****************************************************************************
Public Constant Declarations
*/
#define CONVERSION_SAMPLE_RATE_TO_HZ   (25)

/*****************************************************************************
Public Function Declarations
*/
/** Converts 60ths of a dB to a linear fractional gain */
extern unsigned dB60toLinearFraction(int db);

/** Converts 60ths of a dB to a linear Q5.N fractional gain */
extern unsigned dB60toLinearQ5(int db);

/** Converts a linear fractional gain to 60ths of a dB*/
extern unsigned gain_linear2dB60(int db);

#endif /* COMMON_CONVERSIONS_H */
