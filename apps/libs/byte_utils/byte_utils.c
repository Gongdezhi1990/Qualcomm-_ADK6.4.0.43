/****************************************************************************
Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    byte_utils.c
    
DESCRIPTION
    Utility functions to deal with different byte sizes on 
    XAP and the rest of the world.
*/

#include "byte_utils.h"

uint16 ByteUtilsMemCpyToStream(uint8 *dst, uint8 *src, uint16 size)
{
#ifdef HYDRACORE 

    memcpy(dst, src, size);

#else

    uint16 i;

    for(i = 0; i < size; ++i)
    {
        if(i%2)
        {
            dst[i] = src[i/2] & 0xFF;
        }
        else
        {
            dst[i] = src[i/2] >> 8;
        }
    }    
#endif /* HYDRACORE */

    return size;
}

uint16 ByteUtilsMemCpyUnpackString(uint8 *dst, const uint16 *src, uint16 size)
{    
    uint16 i;

    /* This function copies a uint16 string in big endian format to a uint8 buffer in little endian format*/
    for(i = 0; i < size; ++i)
    {
        if(i%2)
        {
           dst[i] = src[i/2] & 0xFF; 
        }
        else
        {
            dst[i] = src[i/2] >> 8 ;
        }
    } 
    return size;
}

/******************************************************************************
NAME
    ByteUtilsMemCpyPackString

DESCRIPTION
    Pack a uint8 string into a uint16 string.
    
RETURNS
    Number of uint16s into which the data was packed.
*/
uint16 ByteUtilsMemCpyPackString(uint16 *dst, const uint8 *src, uint16 size)
{
    uint16 count = 0;
    
    while (size > 1)
    {
        *dst = *src++ << 8;
        *dst++ |= *src++;
        
        ++count;
        size -= 2;
    }
    
    if (size)
    {
        *dst = *src << 8;
        ++count;
    }
    
    return count;
}


uint16 ByteUtilsGetPackedStringLen(const uint16 *src, const uint16 max_len)
{
    uint16 size = 0;
    if (max_len)
    {
        /* Find the actual number of characters in a packed ASCII string. */
        uint16 i;
        char first, second;
        for (i=0; i<max_len; i++)
        {
            first = ((src[i] >> 8) & 0xFF);
            if (first == 0x0)
                break;
            size++;
            second = (src[i] & 0xFF);
            if (second == 0x0)
                break;
            size++;
        }
    }
    return size;
}

uint16 ByteUtilsMemCpyFromStream(uint8 *dst, const uint8 *src, uint16 size)
{
    uint16 i;

    for(i = 0; i < size; ++i)
    {
        ByteUtilsSet1Byte(dst, i, src[i]);
    }

    return size;
}

/******************************************************************************
NAME
    ByteUtilsMemCpy

DESCRIPTION
    Do a memcpy where size is always in octets.
*/
uint16 ByteUtilsMemCpy(uint8 *dst, uint16 dstIndex, const uint8 *src, uint16 srcIndex, uint16 size)
{
#ifdef HYDRACORE 

    memcpy(dst+dstIndex, src+srcIndex, size);

#else	
    uint16 i;
    uint8 val;

    for(i = 0; i < size; ++i)
    {
        ByteUtilsGet1Byte(src, (i + srcIndex), &val);
        ByteUtilsSet1Byte(dst, (i + dstIndex), val);
    }
#endif
    return size;
}

/******************************************************************************
NAME
    ByteUtilsMemCpy16

DESCRIPTION
    This function will copies an array of uint16's and swaps the endianness of each element.
*/
uint16 ByteUtilsMemCpy16(uint8 *dst, uint16 dstIndex, const uint16 *src, uint16 srcIndex, uint16 size)
{
#ifdef HYDRACORE    

    uint16 i = 0;
    
    for(i = 0; i < size; ++i)
	{
		if(i%2)
		{
		   dst[i+dstIndex] = src[(i+srcIndex)/2] & 0xFF; 
		}
		else
		{
			dst[i+dstIndex] = src[(i+srcIndex)/2] >> 8 ;            
		}
	} 
#else

    ByteUtilsMemCpy(dst, dstIndex, (uint8 *)src, srcIndex, size);

#endif
        return size;

}

uint16 ByteUtilsSet1Byte(uint8 *dst, uint16 byteIndex, uint8 val)
{
#ifdef HYDRACORE
    dst[byteIndex] = val;
#else
    uint16 *ptr2Byte = (uint16 *)dst;

    if(byteIndex%2)
    {
        ptr2Byte[byteIndex/2] |= val;
    }
    else
    {
        ptr2Byte[byteIndex/2] = val << 8;
    }
#endif    

    return 1;
}

uint16 ByteUtilsSet2Bytes(uint8 *dst, uint16 byteIndex, uint16 val)
{
#ifdef HYDRACORE
    dst[byteIndex] = val >> 8;
    dst[byteIndex+1] = val;
#else    
    uint16 *ptr2Byte = (uint16 *)dst;

    if(byteIndex%2)
    {
        ptr2Byte[byteIndex/2] |= val >> 8;
        ptr2Byte[byteIndex/2+1] = val << 8;
    }
    else
    {
        ptr2Byte[byteIndex/2] = val;
    }
#endif    

    return 2;
}

uint16 ByteUtilsSet4Bytes(uint8 *dst, uint16 byteIndex, uint32 val)
{
    byteIndex += ByteUtilsSet2Bytes(dst, byteIndex, val >> 16);
    ByteUtilsSet2Bytes(dst, byteIndex, val);

    return 4;
}

uint8 ByteUtilsGet1ByteFromStream(const uint8 *src)
{
    return src[0];
}

uint16 ByteUtilsGet2BytesFromStream(const uint8 *src)
{
    uint16 val = 0;

    val = src[1];
    val |= (uint16)src[0] << 8;

    return val;
}

uint32 ByteUtilsGet4BytesFromStream(const uint8 *src)
{
    uint32 val = 0;

    val = ((uint32)src[3] & 0xff);
    val |= ((uint32)src[2] & 0xff) << 8;
    val |= ((uint32)src[1] & 0xff) << 16;
    val |= ((uint32)src[0] & 0xff) << 24;

    return val;
}

uint16 ByteUtilsGet1Byte(const uint8 *src, uint16 byteIndex, uint8 *val)
{
#ifdef HYDRACORE
    *val = src[byteIndex];
#else    
    uint16 *ptr2Byte = (uint16 *)src;

    if (byteIndex % 2)
        *val = (ptr2Byte[byteIndex/2] & 0xFF);
    else
        *val = (ptr2Byte[byteIndex/2] >> 8);

#endif

    return 1;

}

uint16 ByteUtilsGet2Bytes(const uint8 *src, uint16 byteIndex, uint16 *val)
{
#ifdef HYDRACORE
    *val = src[byteIndex];
    *val |= src[byteIndex+1] << 8;
#else
    uint16 *ptr2Byte = (uint16 *)src;

    if(byteIndex%2)
    {
        *val = ptr2Byte[byteIndex/2] << 8;
        *val |= ptr2Byte[byteIndex/2 + 1] >> 8;
    }
    else
    {
        *val = ptr2Byte[byteIndex/2];
    }
#endif

    return 2;
}

uint16 ByteUtilsGet4Bytes(const uint8 *src, uint16 byteIndex, uint32 *val)
{
    uint16 msb, lsb;

    byteIndex += ByteUtilsGet2Bytes(src, byteIndex, &msb);
    ByteUtilsGet2Bytes(src, byteIndex, &lsb);

    *val = (uint32)msb << 16;
    *val |= lsb;

    return 4;
}

/******************************************************************************
NAME
    ByteUtilsStrLCpyUnpack

DESCRIPTION
    Unpacks and copies up to dstsize - 1 characters from the string src to dst,
    NUL-terminating the result if dstsize is not 0.
    
    If the return value is >= dstsize, the output string has been truncated.
    It is the caller's responsibility to handle this.

RETURNS
    Number of characters we tried to copy (length of src).
    
*/
uint16 ByteUtilsStrLCpyUnpack(uint8 *dst, const uint16 *src, uint16 dstsize)
{
    uint16 srclen = 0;
    
    if (dstsize)
    {
        uint8 data;
        
        do
        {
            data = *src >> 8;
            
            if (data)
            {
                ++srclen;
                
                if (srclen < dstsize)
                {
                    *dst++ = data;
                }
                
                data = *src & 0xFF;
                
                if (data)
                {
                    ++srclen;

                    if (srclen < dstsize)
                    {
                        *dst++ = data;
                    }
                    
                    ++src;
                }
            }    
        } while (data);
        
        *dst = '\0';
    }
    
    return srclen;
}
