/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    obex_parse_common.c

DESCRIPTION
    This file defines all common internal functions used by parse and 
    decode functionalities.
*/



#include "obex_parse_internal.h"

/*************************************************************************
 * NAME
 *  opSkipToChar 
 *
 * DESCRIPTION 
 * Skip through the buffer till find the requested character
 ************************************************************************/
__inline__ const char* opSkipToChar(const char* s, const char* e, char c )
{
    while( (s != e) && (*s != c )) ++s;
    return s;
}


/*************************************************************************
 * NAME
 *  opSkipToAnyChar 
 *
 * DESCRIPTION 
 * Skip through the buffer till find any of the requested character from 2. 
 ************************************************************************/
__inline__ const char* opSkipToAnyChar( const char* s, 
                                        const char* e, 
                                        char x , 
                                        char y )
{
    while( (s != e) && (*s != x ) && (*s != y )  ) ++s;
    return s;
}

/*************************************************************************
 * NAME
 *  opSkipToMulChar 
 *
 * DESCRIPTION 
 * Skip through the buffer till find any of the request character from 3.
 ************************************************************************/
__inline__ const char* opSkipToMulChars( const char* s, 
                                         const char* e, 
                                         char x , 
                                         char y,
                                         char z )
{
    while( (s != e) && (*s != x ) && (*s != y ) && (*s != z) ) ++s;
    return s;
}


/*************************************************************************
 * NAME
 *  opSkipChars 
 *
 * DESCRIPTION 
 * ignore the sequense of the requested charactes  
 ************************************************************************/
__inline__ const char* opSkipChars( const char* s, 
                                    const char* e, 
                                    char c , 
                                    char d )
{
    while( (s != e) && (*s == c  || *s == d )) ++s;
    return s;
}


/*************************************************************************
 * NAME
 *  opSkipToTag
 *
 * DESCRIPTION 
 * Proceed to the beginning of the tag
 ************************************************************************/
const char* opSkipToTag( const char* s, 
                          const char* e, 
                          opType type )
{
    char start = (type == op_xml_object)? '<' : 'B';  

    while( (s != e) && (*s !=  start )) ++s;
    return s;
}



/*************************************************************************
 * NAME
 *  opCheckChar
 *
 * DESCRIPTION 
 * Check the next character is a predicted one 
 ************************************************************************/
__inline__ const char* opCheckChar( const char *s, const char *e, char c )
{
    if(!s) return NULL;
    return ((s != e) && (*s == c ))? ++s : NULL; 
}


/*************************************************************************
 * NAME
 *  opAddElement
 *
 * DESCRIPTION 
 * Add a element to the Object Tree
 ************************************************************************/
__inline__ void opAddElement(  ObexParseData* element, 
                               ObexParseFragment fragment,
                               ObexParseObject type,
                               const char* data,
                               uint16  len )
{
    element->type = type;
    element->fragment = fragment;
    element->object = data;
    element->len = len;
} 

