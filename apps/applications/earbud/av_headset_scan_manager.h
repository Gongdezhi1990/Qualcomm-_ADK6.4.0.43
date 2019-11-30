/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_scan_manager.h
\brief	    Interface to module managing inquiry and page scanning.
*/

#ifndef _AV_HEADSET_SCAN_MAN_H_
#define _AV_HEADSET_SCAN_MAN_H_

/*! Fast inquiry scan interval. */
#define SCAN_MAN_INQSCAN_INTERVAL_FAST       0x200
/*! Fast inquiry scan window. */
#define SCAN_MAN_INQSCAN_WINDOW_FAST         0x12
/*! Fast page scan interval. */
#define SCAN_MAN_PAGESCAN_INTERVAL_FAST      0x200
/*! Fast page scan window. */
#define SCAN_MAN_PAGESCAN_WINDOW_FAST        0x12

/*! Slow inquiry scan interval. */
#define SCAN_MAN_INQSCAN_INTERVAL_SLOW       0x800
/*! Slow inquiry scan window. */
#define SCAN_MAN_INQSCAN_WINDOW_SLOW         0x24
/*! Slow page scan inquiry. */
#define SCAN_MAN_PAGESCAN_INTERVAL_SLOW      0x800
/*! Slow page scan window. */
#define SCAN_MAN_PAGESCAN_WINDOW_SLOW        0x24

/*! Number of scan manager users. */
#define SCAN_MAN_NUM_USERS  3

/*! @brief Enumeration of users of the scan manager.

    Each user expressed as a bit in a mask. New users
    should be added as next bit, i.e. 0x0008.
 */
typedef enum
{
    /*! Pairing user. */
    SCAN_MAN_USER_PAIRING   = 0x0001,

    /*! Main application state machine user. */
    SCAN_MAN_USER_SM        = 0x0002,

    /*! Peer Signalling module user. */
    SCAN_MAN_USER_PEERSIG   = 0x0004
} scanManagerUser;

/*! @brief Enumeration of types of scanning parameters.

    This is an ordered list, the higher the value the higher the priority of
    the parameters. So a request for slow parameters when fast are already
    running will not be honoured (though the scan will still 'start'), while a
    request for fast parameters when slow are already configured will result
    in the parameters being changed to fast.
*/
typedef enum
{
    /*! Undefined parameter type. */
    SCAN_MAN_PARAMS_TYPE_NOT_SET    = 0,

    /*! Slow parameters type, lower duty cycle. */
    SCAN_MAN_PARAMS_TYPE_SLOW       = 1,

    /*! Fast parameters type, higher duty cycle. */
    SCAN_MAN_PARAMS_TYPE_FAST       = 2
} scanParamsType;

/*! @brief Scan Manager state. */
typedef struct
{
    /*! Register of active inquiry scan users. */
    scanManagerUser inq_scan_state;

    /*! Register of active page scan users. */
    scanManagerUser page_scan_state;

    /*! Record of type of inquiry scan params requested by each user. */
    scanParamsType  inq_user_params_type[SCAN_MAN_NUM_USERS];

    /*! Record of type of page scan params requested by each user. */
    scanParamsType  page_user_params_type[SCAN_MAN_NUM_USERS];

    /*! Currently configured inquiry scan parameters. */
    scanParamsType  current_inq_scan_params;

    /*! Currently configured page scan parameters. */
    scanParamsType  current_page_scan_params;
} scanManagerTaskData;

/*! @brief Initialse the scan manager data structure. */
void appScanManagerInit(void);

/*! @brief Enable inquiry scanning for a specifc user, with requested parameters.

    Note the parameters may not be honoured if another scan is already running
    with higher priority parameters.

    @param user     User type requesting the scan.
    @param inq_type Type of inquiry scan parameters;
 */
void appScanManagerEnableInquiryScan(scanManagerUser user, scanParamsType inq_type);

/*! @brief Enable page scanning for a specified user, with requested parameters. 

    Note the parameters may not be honoured if another scan is already running
    with higher priority parameters.

    @param user      User type requesting the scan.
    @param page_type Type of page scan parameters;
*/
void appScanManagerEnablePageScan(scanManagerUser user, scanParamsType page_type);

/*! @brief Enable both inquiry and page scanning for a specified user, with requested parameters.
 
    Note the parameters may not be honoured if another scan is already running
    with higher priority parameters.

    @param user      User type requesting the scan.
    @param inq_type  Type of inquiry scan parameters;
    @param page_type Type of page scan parameters;
*/
void appScanManagerEnableInquiryPageScan(scanManagerUser user, scanParamsType inq_type,
                                         scanParamsType page_type);

/*! @brief Disable inquiry scanning for a specified user.
 
    @param user User type requesting the scan.
*/
void appScanManagerDisableInquiryScan(scanManagerUser user);

/*! @brief Disable page scanning for a specified user.
 
    @param user User type requesting the scan.
*/
void appScanManagerDisablePageScan(scanManagerUser user);

/*! @brief Disable both inquiry and page scanning for a specified user.
 
    @param user User type requesting the scan.
*/
void appScanManagerDisableInquiryPageScan(scanManagerUser user);

/*! @brief Determine if page scan is enabled.

    @param user User type to check if is enabled.

    @return bool TRUE page scan is enabled, FALSE page scan is not enabled.
 */
bool appScanManagerIsPageScanEnabledForUser(scanManagerUser user);

#endif /* _AV_HEADSET_SCAN_MAN_H_ */
