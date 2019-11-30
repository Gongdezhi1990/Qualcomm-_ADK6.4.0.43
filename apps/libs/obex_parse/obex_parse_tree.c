/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    obex_parse_tree.c

DESCRIPTION
    This internal file defines the parsing functionality.
*/

#include "obex_parse_internal.h"

/**************************************************************************
 *NAME
 *  opParseVobjType
 *
 *DESCRIPTION 
 *  Find the type of the vObject element.
 *
 *PARAMETERS
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 ************************************************************************/ 
static  ObexParseObject opParseVobjType( const char *s , const char* e )
{
    ObexParseObject obj = op_vobj_folded_element;

    if( *s == 'V' || *s == 'B' )
    {
        if( (opCheckChar( opCheckChar( opCheckChar(s, e, 'V'), 
                 e, 'C'), e, 'A') ) )
        {
            s += 3;
            if( ( opCheckChar( opCheckChar(s, e, 'R'), e, 'D') ) )
            {
                obj = op_vobj_vcard;
            }
            else if(opCheckChar(s, e, 'L') )
            {
                obj =  op_vobj_vcal;
            }          
        }
        else
        {
            if( s!= e) s++;
            if( (opCheckChar( opCheckChar( opCheckChar(s, e, 'M'),
                      e, 'S'), e, 'G' ) ) )
            {
                obj = op_vobj_bMsg;
            }
            else if( (opCheckChar( opCheckChar( opCheckChar(
                        s, e, 'E'), e, 'N'), e, 'V' ) ) )
            {
                obj = op_vobj_bEnv;
            }
            else if( (opCheckChar( opCheckChar( opCheckChar( 
                      opCheckChar( s, e, 'B'), e, 'O'), e, 'D' ),
                      e, 'Y' ) ) )
            {
                obj = op_vobj_bBody;
            } 
        }
    }
    else
    {
        if((opCheckChar( opCheckChar( opCheckChar( 
                s, e, 'M'), e, 'S'), e, 'G') ) )
        {
                obj = op_vobj_msg;
        }
    } 

    return obj;
}

/**************************************************************************
 *NAME
 *  opParseXmlType
 *
 *DESCRIPTION 
 *  Find the type of the XML element.
 *
 *PARAMETERS
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 ************************************************************************/ 
static  ObexParseObject opParseXmlType( const char *s , const char* e )
{
    ObexParseObject obj =  op_xml_folded_element;

    if((opCheckChar( opCheckChar( opCheckChar( opCheckChar( opCheckChar(
                s, e, 'e'), e, 'v'), e, 'e'), e, 'n'), e, 't' )))
    {
        obj = op_xml_map_event;
    }
    else if((opCheckChar( opCheckChar( opCheckChar(
            s, e, 'm'), e, 's'), e, 'g')))
    {
        obj = op_xml_map_msg_list;
    }
    else if(opCheckChar(s, e, 'f') )
    {
        if( s != e ) s++;
        if((opCheckChar( opCheckChar( opCheckChar(
            s, e, 'i'), e, 'l'), e, 'e')))
        {
            obj = op_xml_file_list;
        }
        else if((opCheckChar( opCheckChar( opCheckChar( opCheckChar
                ( opCheckChar( s, e, 'o'), e, 'l'), e, 'd'), e, 'e'),
                 e, 'r' )))
        {
            obj = op_xml_folder_list;
        }
    }  

    return obj;

}

/**************************************************************************
 *NAME
 *  opExtractTag
 *
 *DESCRIPTION 
 *  Extract the Attribute Tag
 *
 *PARAMETERS
 *  handle - Handle for the current parse status.
 *  element - Current element being parsed. 
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 ************************************************************************/ 
static const char* opExtractTag(  opHandle *handle,
                                  ObexParseData *element,
                                  const char *s,  
                                  const char *e )
{
    ObexParseObject obj = op_unknown;

    if( handle->type == op_xml_object )
    {
        obj = opParseXmlType(s, e);
    }
    else
    {
        obj = opParseVobjType(s, e);
    }

    if( element->type == op_xml_element ||
        element->type == op_vobj_element )
    {
        element->type = obj;
    }

    s = opSkipToAnyChar( s, e, ( handle->type == op_xml_object )? '>':'\n',
                               ( handle->type == op_xml_object )? ' ':'\r');
    if( s!= e ) s++;
    s = opSkipBlank(s, e );

    return s;

}   

/*************************************************************************
 *NAME
 *  opCheckStartTag
 *
 *DESCRIPTION 
 *  Validate the Begining of a object
 *
 *PARAMETERS
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 *  type - Type of the element. XML or vObject type.
 ************************************************************************/
bool opCheckStartTag(const char *s, const char *e, opType type )
{
    if(type == op_xml_object )
    {
        if( (s= opCheckChar ( s, e, '<' ) ) )
        {
            return !opCheckChar ( s , e, '/' ) ;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return  (TRUE && opCheckChar ( opCheckChar ( opCheckChar (
                opCheckChar( opCheckChar ( opCheckChar (
                s, e, 'B' ), e, 'E' ), e, 'G' ), e, 'I' ),
                e, 'N' ), e, ':') );
    }
}
 
/*************************************************************************
 *NAME
 *  opCheckEndTag
 *
 *DESCRIPTION 
 *  Validate the End of a object
 *
 *PARAMETERS
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 *  type - Type of the element. XML or vObject type.
 ************************************************************************/
static bool opCheckEndTag(const char *s, const char *e, opType type )
{
    if(type == op_xml_object )
    {
        if( opCheckChar ( s, e, '/' ) )
        {
            return ( *(s-1) == '<' ||  opCheckChar ( s+1, e, '>' ) );
        }
        else
        {    
            return ( TRUE && 
                     opCheckChar (opCheckChar ( s, e, '<' ), e, '/' ) );
        }
    }
    else
    {
        return (TRUE && opCheckChar ( opCheckChar ( opCheckChar (
                opCheckChar( s, e, 'E'), e, 'N' ), e, 'D'), e, ':') );
    }
}
 
/*************************************************************************
 *NAME
 *  opOnEndTag
 *
 *DESCRIPTION 
 *  Action on finding the end Tag.
 *
 *PARAMETERS
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 *  handle - Handle for the current parse status.
 ************************************************************************/
static const char* opOnEndTag( const char *s,
                               const char *e , 
                               opHandle *handle)
{
    if( opCheckEndTag( s, e, handle->type ) ) 
    {
        handle->state = op_in_end_tag;

        if( handle->folds )
        {
            s = opSkipToChar( s, e,
                            ( handle->type == op_xml_object )? '>':'\n');
        }
    }
    else
    {
        if( !opCheckStartTag( s, e, handle->type ) ) if(s != e) s++; 

    }

    return s;
}

/*************************************************************************
 *NAME
 *  opHandleNewEntry
 *
 *DESCRIPTION 
 *  Action on detecting a new element.
 *
 *PARAMETERS
 *  handle - Handle for the current parse status.
 *  element - Pointer to the element data being extracted.
 *  s - Start of the input buffer.
 *  e - End of the input buffer.
 ************************************************************************/
static const char* opHandleNewEntry(   opHandle *handle,
                                       ObexParseData *element,
                                       const char *s,
                                       const char *e )
{
    s = opSkipBlank(s, e); 
    
    /* Register the start of the new element */
    opAddElement( element, op_element_partial_start, handle->type, s, e-s );

    if( handle->type == op_xml_object )
    {
        handle->state = (*s == '<'  )? op_in_start_tag: op_error;
    }
    else
    {
        handle->state = ( *s =='B'  )? op_in_start_tag : op_in_get_element;
    }

    return s;
}

/*************************************************************************
 *name
 *  opgetprop
 *
 *description 
 *  action on detecting a potential start tag 
 *
 *parameters
 *  handle - handle for the current parse status.
 *  s - start of the input buffer.
 *  e - end of the input buffer.
 ************************************************************************/
static const char* opGetProp(   opHandle *handle,
                                const char *s,
                                const char *e )
{
    if( opCheckStartTag( s, e, handle->type ) )
    {
         handle->state = op_in_get_tag;
         s += (handle->type == op_xml_object )? 1: OP_IRDA_START_PROP_LEN;
    }
    else 
    {
        s = opOnEndTag( s, e, handle );

        if( handle->state == op_in_start_tag ) 
        {
            handle->state = op_in_get_element;
        }
    }

    return s;
}

/*************************************************************************
 *name
 *  opGetTag
 *
 *description 
 *  action on detecting a start tag 
 *
 *parameters
 *  handle - handle for the current parse status.
 *  element - Pointer to the element data being extracted.
 *  s - start of the input buffer.
 *  e - end of the input buffer.
 ************************************************************************/
static const char* opGetTag(   opHandle *handle,
                               ObexParseData *element,
                               const char *s,
                               const char *e )
{
    s = opExtractTag(handle, element, s, e);

    if( s != e )
    {
        if(handle->openEnv)
        {
            /* remove only the top envelop */
            s= opSkipSpace( s, e );
            element->object = s;
            element->len = e - s;
            handle->openEnv = FALSE;
            element->type = handle->type;
        }
        else
        {
            handle->folds++;
        }
        handle->state = op_in_get_element;
    }
    else
    {
        /* Failed to get the Tag */
        if( handle->openEnv ) handle->state = op_error;
    }

    return s;
}

/*************************************************************************
 *name
 *  opGetElement
 *
 *description 
 *  Extract the element value after parsing the tag
 *
 *parameters
 *  handle - handle for the current parse status.
 *  element - Pointer to the element data being extracted.
 *  s - start of the input buffer.
 *  e - end of the input buffer.
 ************************************************************************/
static const char* opGetElement(   opHandle *handle,
                                   ObexParseData *element,
                                   const char *s,
                                   const char *e )
{
    char start = 'B';   
    char end = '\n';  
    char end2 = '\r';
    
    if( handle->type == op_xml_object )
    {
        start = '<';
        end2 = '/';
        end = '>';
    }

    s = opSkipSpace( s, e );
    s = opSkipToMulChars( s, e, start, end, end2 );

    if( s != e )
    {
        char c = *s;

        if( c == start )
        {
            handle->state = op_in_start_tag;
        }
        else 
        {
            if( !handle->folds && ( *s == end ||
                 opCheckChar(opCheckChar( s, e, end2), e, end ) ) )
            {
                handle->state = op_in_end_tag;
            }
            else
            {
              s= opSkipBlank( s, e );
              s = opOnEndTag( s, e, handle );

            }
        }
    }
    return s; 
}

/*************************************************************************
 *name
 *  opHandleEnd
 *
 *description 
 *  Action of detecting the potential end of the element.
 *
 *parameters
 *  handle - handle for the current parse status.
 *  element - Pointer to the element data being extracted.
 *  s - start of the input buffer.
 *  e - end of the input buffer.
 ************************************************************************/
static const char* opHandleEnd(   opHandle *handle,
                                  ObexParseData *element,
                                  const char *s,
                                  const char *e )
{
    if( handle->folds ) handle->folds --;

    if( handle->folds )
    {
        /*
        handle->state = op_in_start_tag;
        s = opSkipToTag(s, e, handle->type ); */
        handle->state = op_in_get_element;
    }
    else
    {
        const char *p = element->object;

        if (handle->type == op_xml_object )
        {
            if(*p == '<')
            {
                s = opSkipToChar ( s, e, '>');
            }
            else
            {
                /* Happens on skipping the tag */
                if(( *s == '/' || *s == '>') && (p != s)) s--;
                if( *s == '<' && (p != s)) s--; 
            } 
        }
        else
        {
            while( ( s != p )  &&
                 ( *s == '\t' || *s == ' ' || *s == '\n' || *s == '\r') )
                 s--; 
        }
        if(s != e) s++;
        element->len =  s-p;
        element->fragment = op_element_full;
        handle->state = op_end;
        s= opSkipBlank(s,e);
        if( handle->type == op_xml_object && *p != '<' )
        {
            s = opSkipToChar ( s, e, '>'); 
            if( s != e) s++;
        }
    }
    

    return s;

}




/*************************************************************************
 * NAME
 *  opGetNextElement
 *
 * DESCRIPTION 
 * Get the Next element and add it to the element buffer
 ************************************************************************/
const char* opGetNextElement(   opHandle *handle,
                                ObexParseData *element,
                                const char *s,
                                const char *e )
{
    while( ( s != e ) && ( handle->state != op_error ) &&
           ( handle->state != op_end ) ) 
    {
        switch( handle->state )
        {
            case op_new_entry:
                s= opHandleNewEntry(handle, element, s, e );
                break;
        
            case op_in_start_tag:
                s = opGetProp(handle, s, e );
                break;

            case op_in_get_tag:
                s= opGetTag(handle, element, s, e );
                break;

            case op_in_get_element:
                s = opGetElement(handle, element, s, e );
                break;

            case op_in_end_tag:
                s = opHandleEnd(handle, element, s, e);
            
             default: 
                break;    
        } 
    }

    if( handle->state == op_end )
    {
        handle->state = op_new_entry;


        while(opCheckEndTag( s, e, handle->type ))
        {
            s = opSkipToChar( s, e,
                            ( handle->type == op_xml_object )? '>':'\n');
            if(s != e) s++;
        }
    }
    s = opSkipJunk(s, e); 
    
    return s;
}

