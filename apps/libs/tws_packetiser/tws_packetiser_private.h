/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packetiser_private.h

DESCRIPTION
    Private header file for the TWS Packetiser library.

*/

#ifndef TWS_PACKETISER_PRIVATE_H_
#define TWS_PACKETISER_PRIVATE_H_

/* Control debug generation using hydra log. Using hydra log results in more efficient
   utilisation of the log buffer than printf */
#ifdef TWS_PACKETISER_DEBUG_LIB
#include <hydra_log.h>
#define TP_DEBUG(x)  L2_DBG_MSG(x)
#define TP_DEBUG1(x, A)  L2_DBG_MSG1(x, A)
#define TP_DEBUG2(x, A, B)  L2_DBG_MSG2(x, A, B)
#define TP_DEBUG3(x, A, B, C)  L2_DBG_MSG3(x, A, B, C)
#define TP_DEBUG4(x, A, B, C, D)  L2_DBG_MSG4(x, A, B, C, D)
#define TP_LOG_STRING(label, text) HYDRA_LOG_STRING(label, text)
#else
#define TP_DEBUG(x)
#define TP_DEBUG1(x, A)
#define TP_DEBUG2(x, A, B)
#define TP_DEBUG3(x, A, B, C)
#define TP_DEBUG4(x, A, B, C, D)
#define TP_LOG_STRING(label, text)
#endif

/*!
  @brief Map and claim data in a Sink.
  @param sink The sink in which to map and claim data.
  @param len The number of octets that must be claimed in the sink.
  @return The mapped write pointer, or NULL if the sink cannot be mapped or len
  octets cannot be claimed in the sink.
  The caller does not need to flush all that is claimed in the sink.
  If all the claimed data is not flushed, the outstanding data will remain
  claimed in the sink. Subsequent calls to this function will only claim the
  amount extra required to fulfill the new len requirement.
*/
uint8 *tpSinkMapAndClaim(Sink sink, uint32 len);

#endif
