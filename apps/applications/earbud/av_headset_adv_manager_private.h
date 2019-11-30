/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Internal defines used by the advertising manager
*/

#ifdef _AV_HEADSET_ADV_MANAGER_PRIVATE_H_
#error "Private header file should only be included in the using module"
#endif

#define _AV_HEADSET_ADV_MANAGER_PRIVATE_H_

/*! Macro to make a message based on type. */
#define MAKE_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
/*! Macro to make a variable length message based on type. */
#define MAKE_MESSAGE_VAR(VAR, TYPE) TYPE##_T *VAR = PanicUnlessNew(TYPE##_T);


/*! Helper macro to check if a pointer to advert settings is valid

    At present checks that the pointer matches the single supported advert

    \param[in]  x   Pointer to advert to check
*/
#define VALID_ADVERT_POINTER(x) ((x)== &advertData[0])

/*! Size of a data element header in advertising data 

      * Octet[0]=length, 
      * Octet[1]=Tag
      * Octets[2]..[n]=Data
*/
#define AD_DATA_HEADER_SIZE         (0x02)   

/*! Maximum data length of an advert if advertising length extensions are not used */
#define MAX_AD_DATA_SIZE_IN_OCTETS  (0x1F)

/*! Given the total space available, returns space available once a header is included

    This makes allowance for being passed negative values, or space remaining 
    being less that that needed for a header.

    \param[in] space    Pointer to variable holding the remaining space
    \returns    The usable space, having allowed for a header
*/
#define USABLE_SPACE(space)             ((*space) > AD_DATA_HEADER_SIZE ? (*space) - AD_DATA_HEADER_SIZE : 0)

/*! Helper macro to return total length of a field, once added to advertising data 

    \param[in] data_length  Length of field
    
    \returns    Length of field, including header, in octets
*/
#define AD_FIELD_LENGTH(data_length)    (data_length + 1)

/*! Size of flags field in advertising data */
#define FLAGS_DATA_LENGTH           (0x02)

/*! Calculate value for the maximum possible length of a name in advertising data */
#define MAX_AD_NAME_SIZE_OCTETS     (MAX_AD_DATA_SIZE_IN_OCTETS - AD_DATA_HEADER_SIZE)

/*! Minimum length of the local name being advertised, if we truncate */
#define MIN_LOCAL_NAME_LENGTH       (0x10)   

/*! Maximum number of UUID16 services supported in an advert. This can be extended. */
#define MAX_SERVICES                        (4)

/*! Number of octets needed to store a UUID16 service entry */
#define OCTETS_PER_SERVICE                  (0x02)

/*! Macro to calculate the number of UUID16 service entries that can fit in a given 
length of advertising data.

    \param[in] space    Total space available in buffer (octets)

    \returns    Number of services that will fit in the space available
*/
#define NUM_SERVICES_THAT_FIT(space)        (USABLE_SPACE(space) / OCTETS_PER_SERVICE)

/*! Helper macro to calculate the length needed to add UUID16 service to advertising data

    \param num_services     Number of UUID16 services to be added

    \returns Total length needed, in octets
 */
#define SERVICE_DATA_LENGTH(num_services)   ((num_services) * OCTETS_PER_SERVICE)

/*! Write a single value to the advertising data buffer

    This macro adds a single value into the buffer, incrementing the pointer
    into the buffer and reducing the space available.

    \param[in,out] ad_data Pointer to the next position in the advertising data
    \param[in,out] space   Variable with the amount of remaining space in buffer
    \param[in]     value   Value (octet) to add to the advertising data
*/
#define WRITE_AD_DATA(ad_data, space, value) do \
                                                { \
                                                    *ad_data = value; \
                                                    ad_data++; \
                                                    (*space)--; \
                                                } while(0)

/*! Enumerated type for messages sent within the advertising manager only. */
enum headset_adv_manager_internal_messages
{
        /*! Start advertising using this advert */
    ADV_MANAGER_START_ADVERT = 1,
        /*! Set advertising data using this advert. Used for connections (from Gatt) */
    ADV_MANAGER_SETUP_ADVERT,
};

/*! Implementation of the anonymous structure for advertisements. 

    This structure is not visible outside of the advertising module.*/
struct _advManagerAdvert 
{
    bool    in_use;     /*!< Is this advertising entry being used */
        /*! Grouping of flags to indicate which information has been configured for
            the advert. 
            This helps remove the need to have a defined NOT_SET value, where 0
            is a valid value for a setting. */
    struct
        {
            /*! Has a name been configured in the advert */
        bool    local_name:1;
            /*! Has the name in the advert been shortened before adding */
        bool    local_name_shortened:1;
            /*! There are flags to be included in the advert */
        bool    flags:1;
            /*! There are services to be included in the advert */
        bool    services:1;
            /*! The advertising parameters have been set (interval & filter) */
        bool    parameters;
            /*! Whether the type of advert has been set (mandatory) */
        bool    advert_type:1;
        }   content;
            /*! The name to use in the advert */
    uint8   local_name[MAX_AD_NAME_SIZE_OCTETS+1];
            /*! The flags to include in advertisements. See the Bluetooth Low Energy GAP flags
                in connection.h, such as BLE_FLAGS_DUAL_HOST. These are not set directly. */
    uint8   flags;
            /*! The channel map to use for advertising. See connection.h for the values,
                such as BLE_ADV_CHANNEL_ALL */
    uint8               channel_map_mask;
            /*! The reason for advertising, which affects the flags used in the advert.*/
    avHeadsetBleGapReadName reason;
            /*! Number of GATT services (uuid16) that are to be included in the advert */
    uint16              num_services;
            /*! Array of GATT services to be included in the advertisement */
    uint16              services_uuid16[MAX_SERVICES];
            /*! Flag indicating whether we want to use a random address in our
                advertisements. This will actually be a resolvable private address (RPA) */
    bool                use_own_random;
            /*! Advertising settings that affect the filter and advertising rate */
    ble_adv_params_t    interval_and_filter;
            /*! The type of advertising to use, when advertising is started */
    ble_adv_type        advertising_type;
};


/*! Generic message used for messages sent by the advertising manager */
typedef struct {
    advManagerAdvert    *advert;        /*!< The advert that the message applies to */
    Task                requester;      /*!< The task that requested the operation (can be NULL) */
} ADV_MANAGER_ADVERT_INTERNAL_T;


/*! Enumerated type used to note reason for blocking

    Advertising operations can be delayed while a previous operation completes.
    The reason for the delay is recorded using these values */
typedef enum {
    ADV_SETUP_BLOCK_NONE,               /*!< No blocking operation at present */
    ADV_SETUP_BLOCK_ADV_DATA_CFM = 1,   /*!< Blocked pending appAdvManagerSetAdvertisingData() completing */
    ADV_SETUP_BLOCK_ADV_PARAMS_CFM = 2, /*!< Blocked pending appAdvManagerHandleSetAdvertisingDataCfm completing */
} advManagerBlockingState;

/*! Structure used for internal message #ADV_MANAGER_START_ADVERT */
typedef ADV_MANAGER_ADVERT_INTERNAL_T ADV_MANAGER_START_ADVERT_T;

/*! Structure used for internal message #ADV_MANAGER_SETUP_ADVERT */
typedef ADV_MANAGER_ADVERT_INTERNAL_T ADV_MANAGER_SETUP_ADVERT_T;

