/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    obex_parse_internal.h

DESCRIPTION
    Internal header file for OBEX library 

*/

#ifndef OBEX_PARSE_INTERNAL_H_
#define OBEX_PARSE_INTERNAL_H_

#include "obex_parse.h"

typedef enum{
    op_new_entry = 0,       
    op_in_start_tag,     /* e.g: BEGIN: or < in xml*/
    op_in_get_tag,       /* e.g: VCARD   or  msg ../> or <msg> */
    op_in_get_element,   /* e.g: TYPE:EMAIL or xyz=mno */ 
    op_in_end_tag,       /* e.g: END:VCARD or </tag> or /> */  
    op_end,              /* Completed one element */
    op_error
}opState;

typedef enum{
    op_irda_object = op_vobj_element ,
    op_xml_object = op_xml_element
}opType; 

/* OBEX Parser Handle. Handle will be provided to the application after 
   parsing the partial data to call the parser  */ 
typedef struct{
    opState state:3;
    unsigned  openEnv:1; /* unfold the envelop */
    opType  type:4;
    unsigned folds:8;
} opHandle;

#define opSkipCRLF(s, e ) opSkipChars(s, e, '\n', '\r')
#define opSkipSpace(s, e ) opSkipChars(s, e, '\t', ' ' )
#define opSkipNULL( s,e )  opSkipChars(s, e, 0x0, '\v')
#define opSkipJunk(s, e) opSkipNULL( opSkipSpace( opSkipCRLF (s, e), e), e)
#define opSkipBlank(s, e) opSkipSpace( opSkipCRLF (s, e), e)

#define OP_IRDA_START_PROP_LEN      6   /* Length of BEGIN: */

const char* opGetNextElement(   opHandle *handle,
                                ObexParseData *element,
                                const char *s,
                                const char *e );

const char* opCheckChar( const char *s, const char *e, char c );

char *opDecode( ObexParseData *element,
                const char *tag,
                uint16  tagLen,
                uint16  *maxValLen );

bool opCheckStartTag(   const char *s,
                        const char *e, 
                        opType type );

/* Inline functions */
__inline__ const char* opSkipToChar(  const char* s, 
                                      const char* e, 
                                      char c );

__inline__ const char* opSkipChars( const char* s, 
                                    const char *e, 
                                    char c, 
                                    char d  );

__inline__ const char* opSkipToAnyChar( const char* s, 
                                        const char* e, 
                                        char x , 
                                        char y );

__inline__ const char* opSkipToMulChars( const char* s, 
                                         const char* e, 
                                         char x , 
                                         char y,
                                         char z );

__inline__ const char* opSkipToTag( const char *s, 
                                    const char *e,  
                                    opType type );

__inline__ void opAddElement(  ObexParseData* element, 
                               ObexParseFragment fragment,
                               ObexParseObject type,
                               const char* data,
                               uint16  len );

#endif /* OBEX_PARSE_INTERNAL_H_ */

