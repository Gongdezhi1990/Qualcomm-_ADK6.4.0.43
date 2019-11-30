/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    obex_parse_decode.c

DESCRIPTION
    This internal file defines the decode functionality.
*/

#include <string.h>
#include "obex_parse_internal.h"

/*************************************************************************
 *NAME
 *  opFindTag
 *
 *DESCRIPTION 
 *  Find the Tag from the buffer data.
 *
 *PARAMETERS
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 *  tag - Search tag.
 *  tagLen - Length of the tag.
 ************************************************************************/
static const char*  opFindTag( const char *s ,
                               const char *e ,
                               const char *tag,
                               uint16 tagLen )
{
    while( s != e )
    {
        while( s != e && *s != *tag ) s++;

       if((uint16)(e-s) < tagLen ) return e;
       if((memcmp(s, tag, tagLen) == 0) ) break;
       s++;
    }

    return s;
}

/*************************************************************************
 *NAME
 *  opDecode
 *
 *DESCRIPTION 
 *  Decode the element and extract a value associated with the provided
 *  attribute (tag).
 *
 *PARAMETERS
 *  element - The parsed element.
 *  tag - The attribute to be searched. NULL value means "extract all".
 *  tagLen - Length of the tag.
 *  maxValLen - Maximum length of the value to be extracted as input and 
 *              the extracted length as output.
 ************************************************************************/
char *opDecode( ObexParseData*     element,
                   const char*     tag,
                   uint16          tagLen,
                   uint16          *maxValLen )
{
    const char *s = element->object;
    const char *e = s + element->len;
    char * value = NULL;
    uint16 valLen = *maxValLen;

    *maxValLen = 0;

    if(tag)
    {
        char delim = (element->type == op_xml_element)? '=' : ':';
        s = opFindTag( s, e, tag, tagLen );
        s = opSkipToAnyChar( s, e, delim, '"' );
        if( s == e) return NULL;

        if( *s != '"' ) s++;
        s = opSkipSpace(s, e);

        if( s == e) return NULL;

        delim = (element->type == op_xml_element)? ' ': '\n';
        if( *s == '"')
        {
           delim = '"'; s++;
        }
        valLen = opSkipToChar( s, e, delim) - s;

    }
    
    if( (uint16)(e-s) < valLen ) valLen = e-s;
    
    if(valLen && (value = malloc(valLen)))
    {
        memmove(value, s, valLen);
    }

    *maxValLen = valLen;    

    return value;
}   


