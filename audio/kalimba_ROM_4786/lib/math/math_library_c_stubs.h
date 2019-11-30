// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// 
//
// *****************************************************************************

// Header file for C stubs of "math" library
// Comments show the syntax to call the routine

#if !defined(MATH_LIBRARY_C_STUBS_H)
#define MATH_LIBRARY_C_STUBS_H


// TODO FixP instead of int? TODO add fixp library?
// TODO more existing asm functions to be added to C stubs


/* PUBLIC TYPES DEFINITIONS *************************************************/
typedef struct tFFTStuctTag
{
    unsigned int numPoints;
    int *real;
    int *imag;
} tFFTStruct;



/* PUBLIC FUNCTION PROTOTYPES ***********************************************/


// Library subroutine to evaluate the square root of a number, specifying i
// output q formats
//    input value between 0 and 2^23-1 (ie. 0 and 1.0 fractional)
//    q_in value between 0 and 32 
//    q_out value between 0 and 32
//    result (accurate to 16bits, RMS error is 5bits))
int qsqrt( int x, int q_in, int q_out);


// Optimised FFT subroutine with a simple interface
//    input pointer to fft structure
void fft( tFFTStruct *x);

// bit-reverse an array
//     notes: out must be a circular buffer, size must be power of 2
void bitreverse_array( int *in, int *out, int size);

// Calculate y = log2(x) in taylor series method
//    input double precision, Q1.47 positive number
//    result Q8.16
int log2( long x);


#endif
