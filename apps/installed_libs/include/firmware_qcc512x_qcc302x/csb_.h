#ifndef __CSB__H__
#define __CSB__H__

/*! file  @brief CSB AFH map type */
/*!
AFH data type.
An array of uint16s whose lower octets represent the AFH channels.
Each bit of lower octet represents a AFH channel.
Most significant bit(MSB) of last octet of AfhMap array represents reserved channel
and shall be set to 0.
*/
typedef uint16 AfhMap[10];

#endif
