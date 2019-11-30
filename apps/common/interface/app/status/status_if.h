/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*!
 @file status_if.h
 @brief Status masks used with StatusQuery() and MessageStatusTask() calls
*/

#ifndef __APP_STATUS_IF_H__
#define __APP_STATUS_IF_H__

typedef enum
{
    status_in_inquiry = 1,    /*!< Non-zero if we are inquiring */
    status_in_inquiry_scan,   /*!< Non-zero if we are in inquiry-scan */
    status_in_page,           /*!< Non-zero if we are paging */
    status_in_page_scan,      /*!< Non-zero if we are in page-scan */

    status_tx_acl,            /*!< The number of ACL packets sent (cleared on read) */
    status_rx_acl,            /*!< The number of ACL packets received (cleared on read) */
    status_rx_sco,            /*!< The number of SCO packets sent (cleared on read) */
    status_tx_sco,            /*!< The number of SCO packets received (cleared on read) */
    status_tx_host,           /*!< The number of packets sent to the host (cleared on read) */
    status_rx_host,           /*!< The number of packets received from the host (cleared on read) */

    status_count_scos,        /*!< The number of open SCO connections */

    status_count_master_acls, /*!< The total number of open ACL connections for which we are the master */
    status_count_master_hold, /*!< The number of ACL connections in hold for which we are the master */
    status_count_master_park, /*!< The number of ACL connections in park for which we are the master */
    status_count_master_sniff,/*!< The number of ACL connections in sniff for which we are the master */
    status_count_slave_acls,  /*!< The total number of open ACL connections for which we are the slave */
    status_count_slave_hold,  /*!< The number of ACL connections in hold for which we are the slave */
    status_count_slave_park,  /*!< The number of ACL connections in park for which we are the slave */
    status_count_slave_sniff, /*!< The number of ACL connections in sniff for which we are the slave */
    /*! The total number of open ACL connections (status_count_master_acls + status_count_slave_acls)  */
    status_count_acls,
    /*! The number of ACL connections in hold (status_count_master_hold + status_count_slave_hold) */
    status_count_hold,
    /*! The number of ACL connections in park (status_count_master_hold + status_count_slave_hold) */
    status_count_park,
    /*! The number of ACL connections in sniff (status_count_master_sniff + status_count_slave_sniff) */
    status_count_sniff
} status_field;

#endif
