/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#ifndef GATT_AMA_SERVER_ACCESS_H_
#define GATT_AMA_SERVER_ACCESS_H_


#include <ama.h>

/***************************************************************************
NAME
    handleAmaAccess

DESCRIPTION
    Handles the GATT_MANAGER_SERVER_ACCESS_IND message that was sent to the ama library.
*/
void handleAmaAccess(GAMASS *ama_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind);



/***************************************************************************
NAME
    sendAmaConfigAccessRsp

DESCRIPTION
    Sends an client configuration access response back to the GATT Manager library.
*/
void sendAmaConfigAccessRsp(const GAMASS *ama_server, uint16 cid, uint16 client_config);


/***************************************************************************
NAME
    sendAmaPresentationAccessRsp

DESCRIPTION
    Sends an presentation access response back to the GATT Manager library.
*/
void sendAmaPresentationAccessRsp(const GAMASS *ama_server, uint16 cid, uint8 name_space, uint16 description);

void handleAmaServerAccess(GAMASS *ama_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind);







#endif
