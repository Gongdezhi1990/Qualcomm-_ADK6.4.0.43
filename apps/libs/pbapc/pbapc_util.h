/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_util.h
    
DESCRIPTION
 Helper header file

*/

#ifndef PBAP_UTIL_H_
#define PBAP_UTIL_H_

#include <stdlib.h>

/* length of APP Params */
#define APP_PARAM_HDR_LEN   2
#define BYTE_APP_PARAM_LEN  3
#define WORD_APP_PARAM_LEN  4
#define LONG_LONG_WORD_APP_PARAM_LEN 10

enum
{
    pbapc_app_order = 0x01,
    pbapc_app_search_value,
    pbapc_app_search_attr,
    pbapc_app_max_list_count,
    pbapc_app_start_offset,
    pbapc_app_filter,
    pbapc_app_format,
    pbapc_app_phonebook_size,
    pbapc_app_new_missed_calls
};

void pbapcFrame2ByteAppParams( uint8* ptr, uint8 param, uint16 value );
void pbapcFrameByteAppParams( uint8* ptr, uint8 param, uint8 value );
void pbapcFrame4ByteAppParams( uint8* ptr, uint8 param, uint32 value );
uint16 pbapcFramevCardListAppParams( uint8 order ,
                                     const uint8* srchVal,
                                     uint16 srchValLen,
                                     uint8 srchAttr,
                                     uint8* listData );
uint16 pbapcFrameListAppParams( uint16 maxListCount,
                              uint16 startOffset,
                              uint8* listData );
uint16 pbapcFrameFilterFormat( uint32  filterHi,
                               uint32  filterLo,
                               uint8   format,
                               uint8* listData );
const uint8 *pbapcGetPbNameFromID(uint8 book, uint16 *len);
const uint8* pbapcGetSimName( uint16 *len );
uint16 pbapcGetPbPathFromID( bool  sim,  uint8 book, uint8* path); 
const uint8 *pbapcGetPhonebookMimeType(uint16 *len);
const uint8 *pbapcGetvCardListingMimeType(uint16 *len);
const uint8 *pbapcGetvCardMimeType(uint16 *len);
 
#endif /* PBAP_COMMON_H_ */
