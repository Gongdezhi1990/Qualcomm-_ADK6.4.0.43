/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 */

#ifndef UTILS_BITARRAY_H_
#define UTILS_BITARRAY_H_

/**
 * Extremely simple interface for treating a uint16 array as a bit array.
 * Note: no attempt made to give these function-call semantics - they're
 * just shorthand.
 */

/**
 * Turn the number of bits into the number of underlying words required
 * to store them.
 */
#define _BITARRAY_WORD_LENGTH(length)\
    (((length + (UINT_BIT - 1)) / UINT_BIT) * sizeof(unsigned))

/**
 * Allocate the array via pmalloc if required (stack is better in practice
 * because there's more space)
 */
#define BITARRAY_PMALLOC(array,length)\
    array = (unsigned*)pmalloc(_BITARRAY_WORD_LENGTH(length));

/**
 * pfree it again... this is just here for completeness
 */
#define BITARRAY_PFREE(array)\
    pfree(array);

/**
 * Set all the bits in range [0,length)
 */
#define BITARRAY_SET_ALL(array,length)\
    memset(array,UINT_MASK,_BITARRAY_WORD_LENGTH(length))

/**
 * Clear all the bits in range [0,length)
 */
#define BITARRAY_CLEAR_ALL(array,length)\
    memset(array,0U,_BITARRAY_WORD_LENGTH(length))


/**
 * Set the indicated bit
 */
#define BITARRAY_SET_BIT(array,bit)\
    (array[(bit)/UINT_BIT] = (unsigned)(array[(bit)/UINT_BIT] | (1 << ((bit) & (UINT_BIT-1)))))
/**
 * Clear the indicated bit
 */
#define BITARRAY_CLEAR_BIT(array,bit)\
    (array[(bit)/UINT_BIT] = (unsigned)(array[(bit)/UINT_BIT]  & ~(1 << ((bit) & (UINT_BIT-1)))))
/**
 * Return the indicated bit
 */
#define BITARRAY_GET_BIT(array,bit)\
   ((array[(bit)/UINT_BIT] & (1 << ((bit) & (UINT_BIT-1)))) >> ((bit) & (UINT_BIT-1)))


#endif /* UTILS_BITARRAY_H_ */
