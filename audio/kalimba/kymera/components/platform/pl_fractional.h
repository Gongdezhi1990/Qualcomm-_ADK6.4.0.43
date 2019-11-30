/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file pl_fractional.h
 * \ingroup platform
 *
 * Handle fixed point fractional operations.
 */

#ifndef FRACTIONAL_H
#define FRACTIONAL_H

/* Convert x into 1.(DAWTH - 1) format */
#define FRACTIONAL(x) ( (int)( (x) * ((1l<<(DAWTH-1)) - 1) ))

/* Convert x from 1.(DAWTH - 1) format to
 * (DAWTH - q).q format
 */
#define FRACTIONAL_TO_QFORMAT(x, q) ((x) >> (DAWTH-1-(q)))

#ifdef __GNUC__
/* Alternative definition for GCC builds */
#define frac_mult(a, b) ((int)((((long long)a)*(b)) >> (DAWTH-1)))

#define frac_div(n, d) ((n)/(d))
#else
/**
 * \brief Inline assembly helper function for performing a fractional multiplication
 *
 * \param a The first value for the multiplication in fractional encoding
 * \param b The value to multiply a by in fractional encoding
 * \return Multiplication result in fractional encoding.
 */
asm int frac_mult(int a, int b)
{
    @{} = @{a} * @{b} (frac);
}

/**
 * \brief Inline assembly helper function for performing a fractional divide
 *
 * \param n The numerator of the division in fractional encoding
 * \param d The denominator of the division in fractional encoding
 * \return Division result in fractional encoding.
 */
asm int frac_div(int n, int d)
{
    @[    .change rMACB
          .restrict n:bank1_with_rmac, d:bank1_with_rmac
     ]
    rMACB = @{n} ASHIFT -1 (56bit);
    Div = rMACB / @{d};
    @{} = DivResult;
}

#endif /* __GNUC__ */

#endif /* FRACTIONAL_H */
