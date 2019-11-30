/*
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

/* This implementation is derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm
*/

/* 
Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it is
identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is" without
express or implied warranty of any kind.
These notices must be retained in any copies of any part of this
documentation and/or software.
*/

/*
This is a utility library and provided as a number of
functions. It is provided for use by the PBAP library during
authentication; performance is not adequate for processing large
volumes of data.
A typical application will call MD5Init, make a number of calls to
MD5Update to supply the data, and finally call MD5Final to extract
the data. 
This library uses the same API as the MD5 reference implementation
in RFC-1321, but has been optimised to reduce the code size and
memory consumption on BlueCore.
*/
#ifndef MD5_H_
#define MD5_H_

#include <csrtypes.h>

typedef struct
{
    uint32 buffer[16];
    uint32 state[4];
    uint32 bytes;
} MD5_CTX;

/*!
  @brief Initialise an MD5_CTX.

  MD5 initialization. Begins an MD5 operation, writing a new context. 
*/
void MD5Init(MD5_CTX *);

/*!
   @brief Update an MD5_CTX with the next block of data.

   Continues an MD5 message-digest operation, processing another
   message block, and updating the context.
*/
void MD5Update(MD5_CTX *, const uint8 *bytes, uint16 len);

/*! 
   @brief Extract the digest from the MD5_CTX.

   Ends an MD5 message-digest operation, extracting the digest.
*/
void MD5Final(uint8 digest[16], MD5_CTX *);

#endif /* MD5_H_ */
