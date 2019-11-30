/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_scan_manager.c
\brief	    Implementation of module managing inquiry and page scanning.
*/

#include "av_headset.h"
#include "av_headset_scan_manager.h"
#include "av_headset_log.h"

#include <connection.h>

/*! @brief Determine which scan type to use when enabling a specified scan type. */
static hci_scan_enable appScanManagerEnableType(scanManagerTaskData* scanning,
                                                hci_scan_enable requested_type)
{
    if (requested_type == hci_scan_enable_inq)
    {
        /* requested inquiry scan and no page scan active, just enable inquiry scan */
        if (!scanning->page_scan_state)
            return hci_scan_enable_inq;
    }
    else
    {
        /* requested page scan and no inquiry scan active, just enable page scan */
        if (!scanning->inq_scan_state)
            return hci_scan_enable_page;
    }
    /* requested either inquiry or page scan, but the other is active, enable both */
    return hci_scan_enable_inq_and_page;
}

/*! @brief Determine which scan type to use when disabling a specified scan type. */
static hci_scan_enable appScanManagerDisableType(scanManagerTaskData* scanning,
                                                 hci_scan_enable requested_type)
{
    /* if inquiry scan disable, but page scan is active then enable just
     * page scanning. */ 
    if ((requested_type == hci_scan_enable_inq) && (scanning->page_scan_state))
        return hci_scan_enable_page;

    /* if page scan disable, but inquiry scan is active then enable just
     * inquiry scanning. */ 
    if ((requested_type == hci_scan_enable_page) && (scanning->inq_scan_state))
        return hci_scan_enable_inq;

    /* Otherwise just disable all scanning */
    return hci_scan_enable_off;
}

/*! @brief Modify the inquiry scan parameters and record current configuration. */
static void appScanManagerChangeInqParams(scanManagerTaskData* scanning,
                                          scanParamsType inq_type)
{
    DEBUG_LOGF("SCANMAN Inquiry scan params changed to %d", inq_type);

    if (inq_type == SCAN_MAN_PARAMS_TYPE_SLOW)
        ConnectionWriteInquiryscanActivity(SCAN_MAN_INQSCAN_INTERVAL_SLOW,
                                           SCAN_MAN_INQSCAN_WINDOW_SLOW);
    else
        ConnectionWriteInquiryscanActivity(SCAN_MAN_INQSCAN_INTERVAL_FAST,
                                           SCAN_MAN_INQSCAN_WINDOW_FAST);

    /* record current configuration for future checks */
    scanning->current_inq_scan_params = inq_type;
}

/*! @brief Modify the page scan parameters and record current configuration. */
static void appScanManagerChangePageParams(scanManagerTaskData* scanning,
                                           scanParamsType page_type)
{
    DEBUG_LOGF("SCANMAN Page scan params changed to %d", page_type);

    if (page_type == SCAN_MAN_PARAMS_TYPE_SLOW)
        ConnectionWritePagescanActivity(SCAN_MAN_PAGESCAN_INTERVAL_SLOW,
                                        SCAN_MAN_PAGESCAN_WINDOW_SLOW);
    else
        ConnectionWritePagescanActivity(SCAN_MAN_PAGESCAN_INTERVAL_FAST,
                                        SCAN_MAN_PAGESCAN_WINDOW_FAST);

    /* record current configuration for future checks */
    scanning->current_page_scan_params = page_type;
}

/*! @brief Find the highest priority scan parameters for a specified scan type. */
static scanParamsType appScanManagerHiPriParams(scanManagerTaskData* scanning,
                                                hci_scan_enable type)
{
    int i = 0;
    scanParamsType highest_pri_params = SCAN_MAN_PARAMS_TYPE_NOT_SET;
    scanParamsType* user_params = NULL;

    if (type == hci_scan_enable_inq)
        user_params = scanning->inq_user_params_type;
    else
        user_params = scanning->page_user_params_type;

    for (i=0; i < SCAN_MAN_NUM_USERS; i++)
    {
        if (user_params[i] > highest_pri_params)
            highest_pri_params = user_params[i];
    }
    
    return highest_pri_params;
}

/*! @brief Find index into user params storage for a specific user. */
static int appScanManagerIndexUserParams(scanManagerUser user)
{
    int i = 0;

    do
    {
        user >>= 1;
        i++;
    } while (user);

    return i-1;
}

/*! @brief Initialse the scan manager data structure. */
void appScanManagerInit(void)
{
    scanManagerTaskData* scanning = appGetScanning();
    memset(scanning, 0, sizeof(*scanning));

    DEBUG_LOG("SCANMAN Init");

    /* setup scan type of interlaced for both inquiry and page scans */
    ConnectionWritePageScanType(hci_scan_type_interlaced);
    ConnectionWriteInquiryScanType(hci_scan_type_interlaced);
}

/*! @brief Enable inquiry scanning for a specifc user, with requested parameters. */
void appScanManagerEnableInquiryScan(scanManagerUser user, scanParamsType inq_type)
{
    scanManagerTaskData* scanning = appGetScanning();

    DEBUG_LOGF("SCANMAN Enable Inquiry Scan, user %d type %d", user, inq_type);

    /* if no inquiry scan is running */
    if (!scanning->inq_scan_state)
    {
        /* if requested params differ from current settings, we can set them to
         * the requested as no other inquiry scan is running. */
        if (inq_type != scanning->current_inq_scan_params)
        {
            appScanManagerChangeInqParams(scanning, inq_type);
        }
        /* start the scan */
        ConnectionWriteScanEnable(appScanManagerEnableType(scanning, hci_scan_enable_inq));
    }
    else
    {
        /* if the requested params are such that we should change the scanning
         * setup, then do so and remember the new params */
        if (inq_type > scanning->current_inq_scan_params)
        {
            appScanManagerChangeInqParams(scanning, inq_type);
        }
    }

    /* record user and the params this user requested for optimal settings change
     * when scan users stop */
    scanning->inq_scan_state |= user;
    scanning->inq_user_params_type[appScanManagerIndexUserParams(user)] = inq_type;
}

/*! @brief Enable page scanning for a specified user, with requested parameters. */
void appScanManagerEnablePageScan(scanManagerUser user, scanParamsType page_type)
{
    scanManagerTaskData* scanning = appGetScanning();

    DEBUG_LOGF("SCANMAN Enable Page Scan, user %d type %d", user, page_type);

    /* if no page scan is running */
    if (!scanning->page_scan_state)
    {
        /* if requested params differ from current settings, we can set them to
         * the requested as no other page scan is running. */
        if (page_type != scanning->current_page_scan_params)
        {
            appScanManagerChangePageParams(scanning, page_type);
        }
        /* start the scan */
        ConnectionWriteScanEnable(appScanManagerEnableType(scanning, hci_scan_enable_page));
    }
    else
    {
        /* if the requested params are such that we should change the scanning
         * setup, then do so and remember the new params */
        if (page_type > scanning->current_page_scan_params)
        {
            appScanManagerChangePageParams(scanning, page_type);
        }
    }

    /* record the user of the scan */
    scanning->page_scan_state |= user;
    /* record the params this user requested for optimal settings change when
     * scan users stop */
    scanning->page_user_params_type[appScanManagerIndexUserParams(user)] = page_type;
}

/*! @brief Enable both inquiry and page scanning for a specified user, with requested parameters. */
void appScanManagerEnableInquiryPageScan(scanManagerUser user, scanParamsType inq_type,
                                        scanParamsType page_type)
{
    appScanManagerEnableInquiryScan(user, inq_type);
    appScanManagerEnablePageScan(user, page_type);
}

/*! @brief Disable inquiry scanning for a specified user. */
void appScanManagerDisableInquiryScan(scanManagerUser user)
{
    scanManagerTaskData* scanning = appGetScanning();
    int user_params_index = appScanManagerIndexUserParams(user);

    DEBUG_LOGF("SCANMAN Disable Inquiry Scan, user %d", user);

    /* remove user from list of current users and clear the user params */
    scanning->inq_scan_state &= ~user;
    scanning->inq_user_params_type[user_params_index] = SCAN_MAN_PARAMS_TYPE_NOT_SET;

    /* if we still have active users, check remaining users requested params
     * against current params and see if we can modify current running params
     * to better satisfy remaining users */
    if (scanning->inq_scan_state)
    {
        scanParamsType highest_pri_params = appScanManagerHiPriParams(scanning,
                                                                      hci_scan_enable_inq);
        if (highest_pri_params != scanning->current_inq_scan_params)
        {
            appScanManagerChangeInqParams(scanning, highest_pri_params);
        }
    }
    else
    {
        /* no active users, so we can disable inquiry scan completely */
        ConnectionWriteScanEnable(appScanManagerDisableType(scanning, hci_scan_enable_inq));
    }
}

/*! @brief Disable page scanning for a specified user. */
void appScanManagerDisablePageScan(scanManagerUser user)
{
    scanManagerTaskData* scanning = appGetScanning();
    int user_params_index = appScanManagerIndexUserParams(user);
    
    DEBUG_LOGF("SCANMAN Disable Page Scan, user %d", user);

    /* remove user from list of current users */
    scanning->page_scan_state &= ~user;
    scanning->page_user_params_type[user_params_index] = SCAN_MAN_PARAMS_TYPE_NOT_SET;

    /* if we still have active users, check remaining users requested params
     * against current params and see if we can modify current running params
     * to better satisfy remaining users */
    if (scanning->page_scan_state)
    {
        scanParamsType highest_pri_params = appScanManagerHiPriParams(scanning,
                                                                      hci_scan_enable_page);
        if (highest_pri_params != scanning->current_page_scan_params)
        {
            appScanManagerChangePageParams(scanning, highest_pri_params);
        }
    }
    else
    {
        /* no active users, so we can disable page scan completely */
        ConnectionWriteScanEnable(appScanManagerDisableType(scanning, hci_scan_enable_page));
    }
}

/*! @brief Disable both inquiry and page scanning for a specified user. */
void appScanManagerDisableInquiryPageScan(scanManagerUser user)
{
    appScanManagerDisableInquiryScan(user);
    appScanManagerDisablePageScan(user);
}

/*! @brief Determine if page scan is enabled. */
bool appScanManagerIsPageScanEnabledForUser(scanManagerUser user)
{
    scanManagerTaskData* scanning = appGetScanning();
    return ((scanning->page_scan_state & user) == user);
}
