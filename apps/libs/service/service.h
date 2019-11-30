/*
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.
  

  Library to search service records

  JBS, Jun 2004
*/
/*!
\defgroup service service
\ingroup vm_libs
\brief Library to search service records.
*/

/** @{ */

#ifndef SERVICE_H_
#define SERVICE_H_

#include "region.h"

/*!
    @brief Data element types.
*/
typedef enum
{
    /*! The NULL type.*/
    sdtNil,                    
    /*! Data is an unsigned integer.*/
    sdtUnsignedInteger,        
    /*! Data is a signed twos-complement integer.*/
    sdtSignedInteger,        
    /*! Data is a UUID.*/
    sdtUUID,                
    /*! Data is a text string.*/
    sdtTextString,            
    /*! Data is a boolean.*/
    sdtBoolean,                
    /*! Data is a sequence of data elements.*/
    sdtSequence,            
    /*! Data is a sequence of data elements from which one is to be selected.*/
    sdtAlternative,            
    /*! Data is a URL.*/
    sdtURL                    
} ServiceDataType;

typedef uint16 ServiceAttributeId;    /*!< Service attribute identified type. */

/*! 
    @brief Universal attribute definitions.

    The descriptions of these attibutes are take from, and expanded upon in,
    the Bluetooth specification.
*/
enum
{
    /* Universal attribute definitions */
    /* Search for these using ServiceFindAttribute */
	/*! Service Record Handle. A 32bit number that uniquely identifies each
	  service record within an SDP server.*/
    saServiceRecordHandle = 0,				
	/*! Service Class Identifier List. Consists of a data element sequence in
	  which each data element is a UUID representing the service classes that a
	  given service record conforms to.*/
    saServiceClassIdList = 1,				
	/*! Service Record State. A 32bit integer that is used to facilitate
	  caching of Service Attributes.*/
    saServiceRecordState = 2,				
	/*! Service Identifier. A UUID that universally and uniquely identifies the
	  service instance described by the service record.*/
    saServiceID = 3,						
	/*! Protocol Descriptor List. Describes one or more protocol stacks that
	  may be used to gain access to the service described by the service
	  record.*/
    saProtocolDescriptorList = 4,			
	/*! Browse Group List. Consists of a data element sequence in which each
	  element is a UUID that represents a browse group to which the service
	  record belongs.*/
    saBrowseGroupList = 5,					
	/*! Language Base Attribute Identifier List. Used to support human-readable
	  attributes for multiple natural languages in a single service record.*/
    saLanguageBaseAttributeIDList = 6,		
	/*! Service Info Time to Live. A 32bit integer that contains the number of
	  seconds for which the information in a service record is expected to
	  remain valid and unchanged.*/
    saServiceInfoTimeToLive = 7,			
	/*! Service Availibility. An 8bit unsigned integer that represents the
	  relative ability of the service to accept additional clients.*/
    saServiceAvailability = 8,				
	/*! Bluetooth Profile Descriptor List. Consists of a data element sequence
	  in which each element is a profile descriptor that contains information
	  about a Bluetooth profile to which the service represented by this
	  service record conforms.*/
    saBluetoothProfileDescriptorList = 9,	
	/*! Documentation URL. A URL which points to documentation on the service
	  described by a service record.*/
    saDocumentationURL = 10,				
	/*! Client Executable URL. A URL that refers to the location of an
	  application that may be used to utilize the service described by the
	  service record.*/
    saClientExecutableURL = 11,				
	/*! Icon URL. A URL that refers to the location of an icon that may be used
	  to represent the service described by the service record.*/
    saIconURL = 12,							
	/*! Service name.*/
	saServiceName = 0x0100,	
    /*! GoepL2CapPsm */
    saGoepL2CapPsm = 0x0200,
	/*! Pbap Repository */
	saPbapRepos = 0x0314,
	/* Imaging and HFP attributes */
	/*! Supported features.*/
	saSupportedFeatures = 0x0311,
	/* map mas instance id */
	saMapMasInstanceId = 0x0315,
	/* map mas message support feature */
	saMapMasMsgFeature = 0x0316
};


enum
{
    /* Common UUID's */
    UUID_RFCOMM = 3        /*!< The RFCOMM UUID*/
};

/*!
  @brief Step to the next attribute in this service record. 
  
  @param record The service record. Gets advanced on success.

  @param id    The identified. Gets set to the id of the attribute on success.

  @param type The Service Data Type. Gets set to the type of the corresponding
  value.

  @param value The region. Describes the bytes containing the value.

  On success returns TRUE. Returns FALSE on failure (no more records, or
  malformed).
*/

bool ServiceNextAttribute(
    Region *record,
    ServiceAttributeId *id,
    ServiceDataType *type,
    Region *value
);

/*!
  @brief Step to the next attribute in this service record with the specified
  attribute id. Skips past intervening attributes as necessary.
  
  @param record The region. Gets advanced past the attribute on success.

  @param id The attribute id.

  @param type The Service Data Type. Gets set to the type of the corresponding
  value on success.

  @param value Describes the bytes containing the value.

  On success returns TRUE. Returns FALSE on failure (no such record, or
  malformed).
*/

bool ServiceFindAttribute(
    Region *record, 
    ServiceAttributeId id, 
    ServiceDataType *type, 
    Region *value
);

/*!
  @brief Decompose the first data element in a region.

  @param region Get advanced on success.
  @param type  Gets set to the type of the corresponding element on success.
  @param value Describes the bytes containing the value.

  Value is then typically be processed using the Region functions to get
  unsigned integers or compare UUIDs.

  On success returns TRUE. Returns FALSE on failure (no elements left, or
  malformed).
*/

bool ServiceGetValue(
    Region *region,
    ServiceDataType *type,
    Region *value
);

#endif /* SERVICE_H_ */

/** @} */
