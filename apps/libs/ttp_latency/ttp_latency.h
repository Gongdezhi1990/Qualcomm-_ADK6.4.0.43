/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    ttp_latency.h
 
DESCRIPTION
    Definitions of TTP latency for A2DP sources.
*/

#ifndef TTP_LATENCY_H_
#define TTP_LATENCY_H_

#define TTP_LATENCY_IN_MS (200)
#define TTP_LATENCY_MAX_ERROR_IN_MS (100)
#define TTP_APTX_LL_ESTIMATED_LATENCY_IN_MS (32)

/* This is the minimum time to encode, transmit, receive, decode and play
   2x EC frames with a 22 slot CSB interval */
/*                               [ Worst case, shortest time to tx 2 CSB packets ]   [enc time]   [ec proc]   [ec proc] [dec time]    [decode]
                                 (((2 * CSB_INTERVAL) * 0.625) + (5 * 0.625)       +  5         +  1        +  1         5         +   10  + 3) */
#define CSB_TX_TIME_MIN_MS (25)

/* The window defines the period of time (before the min tx time) in which a
   CSB packet may be created and transmitted */
#define CSB_TX_WINDOW_MS (100)

#define LATENCY_CSB_MS (CSB_TX_TIME_MIN_MS + CSB_TX_WINDOW_MS)

#define BA_A2DP_LATENCY_MS (LATENCY_CSB_MS + 450)

#endif /* TTP_LATENCY_H_ */
