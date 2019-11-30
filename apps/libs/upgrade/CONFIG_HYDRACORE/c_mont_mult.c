/****************************************************************************
Copyright (c) 2016-2017 Qualcomm Technologies International, Ltd.


FILE NAME
    c_mont_mult.c

DESCRIPTION
    Implements Montgomery modular multiplcation/reduction

NOTES

*/

#include <string.h>
#include <rsa_decrypt.h>

typedef uint16 word;
typedef uint32 dword;
typedef int32 sdword;

#if defined (UPGRADE_RSA_2048)
#define Wn 128
#elif defined (UPGRADE_RSA_1024)
#define Wn 64
#else
#error "Neither UPGRADE_RSA_2048 nor UPGRADE_RSA_1024 defined."
#endif

#define T 16
/* R = 2 to the power of the data bus size (in bits). */
#define R  (1L << T)

/* modR is used to perform the operation mod R. */
#define modR ((word)(R - 1))

/* Forward references. */
static void subtract (word p[Wn + 1], const word m[Wn]);
static bool a_biggerthan_b (const word a[Wn + 1], const word b[Wn]);

/****************************************************************************
NAME
	c_mont_mult  -  Montgomery modular multiplcation/reduction
*/
void c_mont_mult (word *A, const word *B, const rsa_mod_t *ms)
{
    word P [Wn + 1]; /* The result product is (Wn + 1) words wide */
    static dword X, Y; /* X and Y are supposed to be registers 2T bits long */
    word q;     /* the quotient q is 1 word (T bits) long, i.e. q < R */
    int i,j;

    /* Sets P to zero: */
    /*for (i = 0; i < Wn + 1; i++)
        P[i] = 0; */
    (void) memset (P, 0, sizeof(word)*(Wn + 1));

    /* A[i] loop: */
    for (i = Wn - 1; i > -1 ; i--)
    {
        X = P[Wn] + A[i]*(dword)B[Wn-1];
        q = (word)(X & modR)*(ms->M_dash);
        Y = q*(dword)(ms->M[Wn-1]) + (X & modR);

        /* B[i] loop: */
        for (j = Wn - 2; j > -1 ; j--)
        {
            X = P[j + 1] + A[i]*(dword)B[j] + (X >> T);
            /* Y >> T performs Y div R (R = 2^T) */
            Y = q*(dword)(ms->M[j]) + (X & modR) + (Y >> T);
            P[j + 2] = (word) (Y & modR); /* P[j + 2] = Y mod R */
        }
        /* at this point we have that j = -1 */
        X = P[j + 1] + (X >> T) + (Y >> T);
        P[j + 2] = (word) (X & modR);
        P[j + 1] = (word) (X >> T);
    }

    /* If P >= M do P = P - M */
    if (a_biggerthan_b (P, &(ms->M[0])))
        subtract( P, &(ms->M[0]));

    /* Copy result into A, the input/output variable */
    /* for (i = Wn - 1; i > -1 ; i--)
        A[i] = P[i+1]; */
    (void) memcpy( A, P+1, sizeof(word)*Wn);
}


/****************************************************************************
NAME
	subtract  -  multi-precision integer subtraction

FUNCTION
	This function subtracts the multi-precision unsigned integer m[Wn]
	from p[Wn+1]. It is assumed that p is greater than or equal to m.

	Inputs: array of unsigned p[] and m[]
	Output: array of unsigned p such that p = p - m
*/
static
void subtract (word P[Wn + 1], const word M[Wn])
{
    int i;
    word Carry;
    sdword Sum;

    Carry = 0;

    for (i = Wn; i > 0 ; i--)
    {
        Sum   = (sdword)P[i] - (sdword)M[i-1] - (sdword)Carry; /* Gets digit-by-digit sum and adds carry */
        Carry = (Sum < 0);
        if (Carry)
            Sum = Sum + (sdword)R; /* Twos's complement value of the negative number */

        P[i] = (word)Sum; /* Assigns sum to the correct digit in the result */
    }

    P[0] =  Carry; /* If there is a carry, it goes to the most significant digit */
}


/****************************************************************************
NAME
	a_biggerthan_b  -  compare multi-precision integers

FUNCTION
	This function returns the value TRUE if the multi-precision integer
	number a[Wn+1] is greater than or equal to b[Wn], otherwise, returns
	FALSE.

	Inputs: unsigned a and b
	Output: TRUE if a >= b, FALSE if a < b
*/

static
bool a_biggerthan_b (const word a[Wn + 1], const word b[Wn])
{
    int i;

    if (a[0] > 0)
        return TRUE;
    else
    for (i = 1; i < Wn+1; i++)
    {
        if (a[i] > b[i-1])
            return TRUE;
        else
        if (a[i] < b[i-1])
            return FALSE;
    }
    return TRUE;
}

