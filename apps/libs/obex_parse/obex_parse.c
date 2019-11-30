/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    obex_parse.c

DESCRIPTION
    This file defines all obex_parse library APIs. 
*/

#include "obex_parse_internal.h"

/**************************************************************************
 *NAME
 *  ObexParseCreateTree
 *
 *DESCRIPTION 
 *  Parse the obex object and create a level 1 tree of objects and elements.
 *
 *PARAMETERS
 *  Refer obex_parse.h for details. 
 ************************************************************************/ 
ObexParseStatus ObexParseCreateTree ( ObexParseTree *object,
                                      ObexParseObject objType,
                                      const char *data,
                                      uint16     dataLen )
{
    const char *s = data , *e = data + dataLen ;
    opHandle handle;
    
    if( !object  ) return obex_parse_invalid_tree;
    if( !data || !dataLen )  return obex_parse_data_corrupted;
    if( objType == op_vobj_element || 
        objType == op_xml_element ||
        objType == op_unknown )
    {
        return obex_parse_not_required;
    }
    
    object->numElements = 0;
    handle.state = op_new_entry;
    handle.type  = ( objType < op_xml_element )? 
                   op_irda_object:op_xml_object;
    handle.folds = 0;
    handle.openEnv = TRUE;

    s = opSkipCRLF( s, e );
    /* Find out it is a start or partial */
    if( !opCheckStartTag( s, e, handle.type ) )
    {
        char start = ( handle.type  == op_xml_object )? '<' : '\n' ;
        ObexParseFragment fragment = op_element_partial_middle;

        s = data;
        /* Skip till finding the next Tag */
        
        s = opSkipToChar( s, e, start ); 

        /* Add one partial element */
         opAddElement( &object->elements[0], 
                       (s != e)? op_element_partial_end: fragment, 
                       handle.type,
                       data,  
                       s - data );
         object->numElements++;
         handle.openEnv = FALSE;   
    }

    /* Parse and add all elements */
    while( ( s != e ) && ( handle.state == op_new_entry ) && 
           ( object->numElements <  OBEX_PARSE_MAX_ELEMENTS ) )
    {
        s= opGetNextElement( &handle, 
                             &object->elements[object->numElements], 
                             s, e ); 

        if( handle.state != op_error ) object->numElements++; 
    } 
                                    
    /* Process the status */   
    if( handle.state == op_error )
    {
        if(object->numElements ==  OBEX_PARSE_MAX_ELEMENTS ) 
        {
            return obex_parse_tree_full;
        }
        else
        {
            return obex_parse_data_corrupted;
        }
    }
    else if( handle.state == op_new_entry )
    {
        return obex_parse_success;
    } 
    else
    {
        return obex_parse_get_more_data;
    }
}

/**************************************************************************
 *NAME
 *  ObexParseDecode
 *
 *DESCRIPTION 
 *  Decode and extract the value associated with a requested tag.
 *
 *PARAMETERS
 *  Refer obex_parse.h for details. 
 ************************************************************************/ 
char*   ObexParseDecode( ObexParseData* element, 
                         const char*    tag,
                         uint16         tagLen,
                         uint16         *maxValLen )
{
    if( !element ) return NULL;

    if( ( element->type != op_vobj_element &&
          element->type != op_xml_element ) ||
        ( !element->len || !element->object ) ) 
    {
        return NULL;
    } 

    if( *maxValLen == 0 || *maxValLen > OBEX_PARSE_MAX_DATA_SIZE )
    {
        *maxValLen = OBEX_PARSE_MAX_DATA_SIZE;
    }

    if( tagLen == 0 ) tag = NULL;

    return opDecode( element, tag, tagLen, maxValLen );
}
 
