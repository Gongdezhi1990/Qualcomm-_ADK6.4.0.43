/*******************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef GATT_DEVICE_INFO_SERVER_H_
#define GATT_DEVICE_INFO_SERVER_H_

#include <csrtypes.h>
#include <message.h>

#include <library.h>

typedef struct
{
    uint8       vendor_id_source;       /* Identifies the source of the Vendor ID field */
    uint16      vendor_id;              /* Identifies the product vendor from the namespace in the Vendor ID Source */
    uint16      product_id;             /* Manufacturer managed identifier for this product */
    uint16      product_version;        /* Manufacturer managed version for this product */
}pnp_id_t;

/*! @brief This is a 64-bit structure which consists of a 40-bit manufacturer-defined identifier
    concatenated with a 24 bit unique Organizationally Unique Identifier (OUI). 
    The OUI is issued by the IEEE Registration Authority (http://standards.ieee.org/regauth/index.html) 
    and is required to be used in accordance with IEEE Standard 802-2001.6 
    while the least significant 40 bits are manufacturer defined.
*/
typedef struct
{
    uint32    system_id[2];
}sys_id_t;

/*! brief The content of this characteristic is determined by the Authorizing Organization 
    that provides Certifications. Refer to 11073-20601 [1] or Continua Design Guidelines [2]
    for more information on the format of this list.

[1]IEEE Std 11073-20601 TM 2008 Health Informatics - Personal Health Device Communication - Application Profile - Optimized Exchange Protocol - version 1.0 or later

[2]	Continua Design Guidelines - Continua Health Alliance; http://www.continuaalliance.org
*/
typedef struct
{
    uint8*    ieee_data_list;
}ieee_t;
    
typedef struct 
{
    const char*     manufacturer_name_string;
    const char*     model_num_string;
    const char*     serial_num_string;
    const char*     hw_revision_string;
    const char*     fw_revision_string;
    const char*     sw_revision_string;
}gatt_dis_strings_t;

/*! @brief Contains DIS supported characteristics,
    As part of initialization of Device Info service.
 */
typedef struct
{
    gatt_dis_strings_t*     dis_strings;
    const sys_id_t*         system_id;
    const ieee_t*           ieee_data;
    const pnp_id_t*         pnp_id;
} gatt_dis_init_params_t;

/*! @brief The Device Information server structure for the server role.

    This structure is visible to the application as it is responsible for managing resource to pass to the Device Information server library.
    The elements of this structure are only modified by the Device information Server library.
    The application SHOULD NOT modify the values at any time as it could lead to undefined behaviour.
 */
typedef struct
{
    TaskData lib_task;
    Task app_task;
    gatt_dis_init_params_t dis_params;
} gdiss_t;

/*!
    @brief Status code returned from the GATT device information server library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    gatt_dis_status_success,
    gatt_dis_status_registration_failed,
    gatt_dis_status_invalid_parameter
}gatt_dis_status_t;

/*!
    @brief Initialises the Device Information Service Library in the Server role.

    @param appTask The Task that will receive the messages sent from this Device Info server library.
    @param dev_info_server A valid area of memory that the service library can use.
    @param init_params Device information server Database paramters.
    @param start_handle start handle
    @param end_handle end handle
    See gatt_dis_init_params_t contains Pointer to Supported DIS information this should not be freed by app.
    managed by the application.
    
    @return TRUE if successful, FALSE otherwise.
*/
bool GattDeviceInfoServerInit(Task appTask,
                              gdiss_t *const dev_info_server,
                              gatt_dis_init_params_t *const init_params,
                              uint16 start_handle,
                              uint16 end_handle);
#endif
