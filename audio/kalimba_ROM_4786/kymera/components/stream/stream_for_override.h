/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup stream Stream Subsystem
 * \file  stream_for_adaptors.h
 * \ingroup stream
 *
 *Stream header file used by adaptor(s). <br>
 *
 */

#ifndef _STREAM_FOR_OVERRIDE_H_
#define _STREAM_FOR_OVERRIDE_H_

/* See description in stream_for_adaptors.h */
extern bool stream_get_endpoint_device(ENDPOINT *endpoint, unsigned* device);

/**
 * \brief  Get configuration from the connected endpoint.
 *
 * \param  endpoint  pointer to an endpoint
 * \param  key ID of the configurtaion
 * \param  result the value.
 *
 * \return TRUE if successful, FALSE for error(s)
 */
extern bool stream_get_connected_to_endpoint_config(ENDPOINT *endpoint, unsigned key, ENDPOINT_GET_CONFIG_RESULT* result);

/**
 * \brief  Set configuration for the connected endpoint.
 *
 * \param  endpoint  pointer to an endpoint
 * \param  key ID of the configurtaion
 * \param  value pointer to the value.
 *
 * \return TRUE if successful, FALSE for error(s)
 */
extern bool stream_configure_connected_to_endpoint(ENDPOINT *endpoint, unsigned key, uint32 value);

/**
 * \brief  Function to check if the connected endpoint is clocked locally or remotely.
 *
 * \param  endpoint  pointer to an endpoint
 *
 * \return TRUE if the connected endpoint is locally clocked, FALSE otherwise.
 */
extern bool stream_connected_to_endpoint_is_locally_clocked(ENDPOINT *endpoint);


/**
 * \brief  Function to tests whether two connected to endpoints use the same clock source
 *
 * \param  ep1  pointer to an endpoint
 * \param  ep2  pointer to another endpoint
 *
 * \return TRUE if the two connected endpoints have same clock source, FALSE otherwise.
 */
extern bool stream_connected_to_endpoints_have_same_clock_source(ENDPOINT *ep1, ENDPOINT *ep2);

/**
 * \brief Finds the endpoint whose clock source is seen at the kymera side
 * boundary of an endpoint.
 *
 * \param ep  The endpoint whose boundary the clock source is requested of
 *
 * \return The endpoint whose clock source is present at the boundary. This may
 * be the endpoint ep.
 */
extern ENDPOINT *stream_rm_get_clk_src_from_pair(ENDPOINT *ep);

/**
 * \brief computes the rate adjustment between endpoints
 *
 * \param ep_src Handle of the source endpoint
 * \param src_rate Rate of source if not enacting endpoint
 * \param ep_sink Handle of sink the endpoint
 * \param sink_rate Rate of sink if not enacting endpoint
 *
 * \return The rate adjustment, If endpoints invalid then zero.
 */
extern unsigned stream_rm_get_rate_adjustment(ENDPOINT *ep_src,unsigned src_rate,ENDPOINT *ep_sink,unsigned sink_rate);

#endif /* _STREAM_FOR_OVERRIDE_H_ */
