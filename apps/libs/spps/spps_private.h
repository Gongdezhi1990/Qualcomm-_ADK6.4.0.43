/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    spps_private.h
    
DESCRIPTION
	Header file for the SPP Server profile library containing private members
	
*/

#ifndef SPPS_PRIVATE_H_
#define SPPS_PRIVATE_H_

#include "spps.h"
#include "../spp_common/spp_common_private.h"

/* Default Spp service record */
#define SSP_SERVICE_RECORD_SIZE		62
extern const uint8 spp_service_record [SSP_SERVICE_RECORD_SIZE];


/* private SPP Server funcitons */
void sppStoreServiceHandle(uint32 service_handle);
uint32 sppGetServiceHandle(void);

#endif /* SPPS_PRIVATE_H_ */
