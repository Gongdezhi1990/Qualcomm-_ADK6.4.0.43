/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    obex_parse.h

VERSION
    0.5 
    
DESCRIPTION
	Header file for the OBEX Parser library. This support library provides 
    utility  APIs to parse OBEX objects like bMessage objects, 
    folder listing objects, MAP message listing objects, 
    MAP event report objects etc.

    The library supports 2 formats and provides 2 features. Supported 
    formats are XML and Irda style formats used in the Bluetooth 
    OBEX profiles. Supprted features are 1) Creating a Parse tree from 
    the received object 2) Decoding the elements of the tree to extract 
    the associated value of a requested tag.
   
   

HISTORY
    0.5 - This initial version supports only messaging Objects for MAP 1.0.
*/

/*!
@file   obex_parse.h
@brief	Header file for the OBEX Object Parser (OP) library. This support 
        library provides  utility  APIs to parse the received OBEX objects.
*/

#ifndef OBEX_PARSE_H_
#define OBEX_PARSE_H_

#include <stdlib.h>

#define OBEX_PARSE_MAX_ELEMENTS     6
#define OBEX_PARSE_MAX_DATA_SIZE    30

/*! @brief OBEX Object Types */
typedef enum{
    op_unknown,           /*!< unknown object */

    op_vobj_element,      /*!< A child vObj data element. e.g: TYPE:EMAIL */
    op_vobj_folded_element, /*!< A Vobj folded item.  */
    op_vobj_vcard,        /*!< A vCard object starts with BEGIN:VCARD */
    op_vobj_vcal,         /*!< A vCal object starts with BEGIN:VCAL */
    op_vobj_bMsg,         /*!< A bMessage object starts with BEGIN:BMSG */
    op_vobj_bEnv,         /*!< A bEnv object starts with BEGIN:BENV */
    op_vobj_bBody,        /*!< A body object starts with BEGIN:BBODY */ 
    op_vobj_msg,          /*!< A message object starts with BEGIN:MSG */

    op_xml_element,       /*!< A child xml attr/value pair <a>1</a> */
    op_xml_folded_element,/*!< A xml element e.g: \<d\>\<a\>1\</a\>\<b\>2\</b\>\</d\>*/
    op_xml_map_event,     /*!< A MAP Event report object */
    op_xml_map_msg_list,  /*!< one or more MAP msg listing elements*/
    op_xml_folder_list,   /*!< x-obex/folder-listing or one folder element*/
    op_xml_file_list      /*!< one file element from x-obex/folder-listing*/
}ObexParseObject;

/*! @brief The fragmentation status of the object */
typedef enum
{
    op_element_full = 0,          /*!< A full object */
    op_element_partial_start = 1, /*!< A partial element with start Tag */
    op_element_partial_end = 2,   /*!< A partial element with end tag */ 
    op_element_partial_middle = 3 /*!< A partial element without start/end*/
}ObexParseFragment;

/*! @brief The data representation of a parsed element in the object tree */
typedef struct
{
    ObexParseFragment   fragment;/*!< Status of data completeness */
    ObexParseObject     type;    /*!< Type of the parsed element object */
    const char*         object;  /*!< The parsed element object */
    uint16              len;     /*!< Length of the parsed element object */
} ObexParseData;

/*! @brief The tree representation of a parsed object */
typedef struct
{
    uint8           numElements;  /*!< Number of elements in the objects */
    ObexParseData   elements[OBEX_PARSE_MAX_ELEMENTS];/*!< List of objects*/
} ObexParseTree;

/*! @brief Status of Object Parsing */
typedef enum obex_parse_status
{
    obex_parse_success,      /*!< Successfully parsed the object */
    obex_parse_get_more_data,/*!< Parsed the fragmented data. Get more data.*/
    obex_parse_tree_full,    /*!< Parse tree is full. Empty the tree */ 
    obex_parse_not_required, /*!< No more parsing is possible on this data */
    obex_parse_invalid_tree, /*!< Invalid tree input received */
    obex_parse_data_corrupted /*!< Invalid data received */

} ObexParseStatus;

/*!
    @brief  Parse the received OBEX object

    @param  objTree The parser library parses the data and fills this tree
    with elements. Each element will be of type ObexParseObject. The 
    object field of each element points to its start location 
    in the input object, which is pointed by "data". 

    @param  objType Type of the object pointed by "data". The library 
    returns obex_parse_not_required error if the objType is set to 
    op_vobj_element or op_xml_element. This is because 
    op_vobj_element and op_xml_element types are used only for the 
    child element in the tree. Child elements does not require
    any more parsing and can be decoded instead.

    @param  data    pointer to the object to be parsed.

    @param  dataLen length of the object to be parsed.

    This API parses the object one level down and creates a tree of 
    all elements on level 1. The application may call this API recursively
    to parse the object until retrieving the child element. 
    A element type of the child element will be  op_vobj_element or 
    op_xml_element. 
    
    e.g: An object \<a\> \<b\> \<c\>1\</c\> \<d\>2\</d\> \</b\> \<e\> 3
    \</e\> \<a\>  has 3 levels (level 0 at \<a\> level 1 at \<b\>,\<e\> and
    level 2 at \<c\>,\<d\>) and this API  parses only 1 level down and
    creates a tree of 2 elements with element 1 points to \<b\> and element
    2 points to \<e\>. The application must call this API again using the
    data pointer of element \<b\> to parse the  next level to retrieve the
    elements \<c\> and \<d\>.    

    This API returns obex_parse_success on successfully parsing a full
    object. It returns obex_parse_get_more_data, if the input data contains
    partial fragmented object. In that case, the application can process
    the already parsed element and get more data to parse the rest.
    It returns obex_parse_tree_full if the data contains more than 
    OBEX_PARSE_MAX_ELEMENTS. In this scenario, the application must 
    empty the tree and call this API again with the unprocessed data.
*/ 
ObexParseStatus ObexParseCreateTree ( ObexParseTree *objTree,
                                      ObexParseObject objType,
                                      const char *data,
                                      uint16     dataLen );

/*
    @brief  Decode a Tag(attribute)/value pair and extract the value.

    @param element  An valid element from the ObexParseTree.

    @param  tag     The attribute, whose value to be extracted. e.g: 
     in a op_vobj_element of "TYPE:EMAIL", TYPE is the tag and EMAIL is the 
     value to be extracted. In a op_xml_element of "<msg handle="203050" />"
     handle is the tag for the value "203050". If this is set to NULL, this
     API extracts the entire element.

    @param tagLen   The length of the tag.

    @param maxValLen This is an in-out value. As input, this is the maximum
    requested length for the value and as output this is the length of the 
    returned value.

    If there are multiple instance of tag in the element, it returns the 
    first instance and ignore the others.

    This API allocates memory of length maxValLen and extracts the value to
    the buffer on return. The application MUST free the returned buffer 
    after processing it.
*/
char*   ObexParseDecode( ObexParseData* element, 
                         const char*    tag,
                         uint16         tagLen,
                         uint16         *maxValLen );


#endif /* OBEX_PARSE_H_ */

