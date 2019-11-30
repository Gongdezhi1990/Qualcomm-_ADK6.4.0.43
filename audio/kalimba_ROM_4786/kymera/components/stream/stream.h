/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup stream Stream Subsystem
 *
 * \file  stream.h
 *
 * stream public header file. <br>
 * This file contains public stream functions and types. <br>
 *
 * \section sec1 Contains:
 * stream_if_get_endpoint <br>
 * stream_if_close_endpoint <br>
 * stream_if_configure_sid <br>
 * stream_if_sync_sids <br>
 * stream_if_connect <br>
 * stream_if_transform_disconnect <br>
 * stream_if_disconnect<br>
 * stream_create_sco_endpoints_and_cbuffers <br>
 * stream_delete_sco_endpoints_and_cbuffers <br>
 * stream_create_operator_endpoint <br>
 * stream_destroy_operator_endpoint <br>
 */

#ifndef STREAM_H
#define STREAM_H

/****************************************************************************
Include Files
*/

#include "types.h"
#include "buffer.h"
#include "stream_prim.h"
#include "stream_audio_data_format.h"

/****************************************************************************
Public Type Declarations
*/

typedef struct ENDPOINT ENDPOINT;
typedef struct TRANSFORM TRANSFORM;
typedef struct STREAM_CONNECT_INFO STREAM_CONNECT_INFO;


/** Structure containing properties of a requested buffer's details */
typedef struct
{
    /** Indicates whether the requestee already has a buffer to supply */
    bool supplies_buffer:1;

    /** Indicates that the endpoint can be overriden. */
    bool can_override:1;

    /** Indicates that the endpoint will override
     * the connected endpoint */
    bool wants_override:1;

    /** Indicates that the requestee is going to run in place. For an operator
     * sink this indicates that the input buffer will be re-used at one or more
     * of the output terminals. For an operator source this indicates that the
     * buffer should be the same underlying memory as that of one of it's input
     * terminals. The terminal is indicated by passing the Cbuffer pointer of
     * the input terminal.
     */
    bool runs_in_place:1;

#ifdef INSTALL_METADATA
    /** Indicates that the endpoint is metadata aware or not. If the endpoint is
     * a source then it indicates that it will produce metadata. If it's a sink
     * then it indicates that it can consume/transport metadata. */
    bool supports_metadata:1;

    /** When the supports_metadata flag is set this buffer if present shares
     * metadata with the connection being made. */
    tCbuffer *metadata_buffer;
#endif /* INSTALL_METADATA */

    /** Union discriminated by supplies_buffer. When supplies_buffer is TRUE
     * buffer should be populated. */
    union buffer_info_union{
        /** The buffer parameters being requested when supplies_buffer and runs_in_place is FALSE */
        struct buffer_requirements{
            /** The minimum size of buffer required */
            unsigned int size;
            /** Any configuration flags requested for the buffer */
            unsigned int flags;
        }buff_params;

        /** A pointer to the buffer that the endpoint wants to supply. supplies_buffer is TRUE */
        tCbuffer *buffer;


        /** The buffer parameters being requested when runs_in_place is True */
        struct {
            /**
             * The in place terminal shows where the operator would run in pace on for
             * the asked endpoint. */
            unsigned int in_place_terminal;
            /** The operator minimum buffer size in words. */
            unsigned int size;
            /** A pointer to the buffer that the in_place_terminal is connected to.
             * NULL if the terminal is not yet connected. */
            tCbuffer *buffer;
        }in_place_buff_params;
    }b;

}BUFFER_DETAILS;


/**
 * Enumeration of endpoint directions
 */
typedef enum
{
    SOURCE = 0,  /** Endpoint is a source */
    SINK = 1     /** Endpoint is a sink */
} ENDPOINT_DIRECTION;

/**
* Kick propagation direction
*/
typedef enum
{
   /** Internally-originated kick */
   STREAM_KICK_INTERNAL,
   /** Forwards kick i.e. source-to-sink (AKA "more data") */
   STREAM_KICK_FORWARDS,
   /** Backwards kick i.e. sink-to-source (AKA "more space") */
   STREAM_KICK_BACKWARDS,
   /** Indicates an invalid kick direction value */
   STREAM_KICK_INVALID
} ENDPOINT_KICK_DIRECTION;

/**
 * Enumeration of the type of ratematching an endpoint can implement
 */
typedef enum
{
    RATEMATCHING_SUPPORT_NONE, /** Can't do ratematching */
    RATEMATCHING_SUPPORT_SW, /** Can implement software ratematching */
    RATEMATCHING_SUPPORT_HW, /** Can implement hardware ratematching */
    RATEMATCHING_SUPPORT_AUTO, /** Naturally runs at the rate data is pulled/pushed, no SW/HW effort required */
    RATEMATCHING_SUPPORT_MONITOR /** Endpoint needs its master rate */
}RATEMATCHING_SUPPORT;

/****************************************************************************
Public Constant Declarations
*/
/** The amount to shift a value to represent 1.0 in the fixed point representation
 * expected by the ratematching manager. Note: 22 even on KAL_ARCH4
 */
#define STREAM_RATEMATCHING_FIX_POINT_SHIFT 22

/* macro to convert STREAM_RATEMATCHING rate to a fractional number */
#define STREAM_RATEMATCHING_RATE_TO_FRAC(x) (((int)(1<<STREAM_RATEMATCHING_FIX_POINT_SHIFT) - (int)(x)) \
                                             << (DAWTH - STREAM_RATEMATCHING_FIX_POINT_SHIFT -1))

/* Sampling rate for the streams in the system */
#define STREAM_AUDIO_SAMPLE_RATE_8K    8000
#define STREAM_AUDIO_SAMPLE_RATE_16K  16000
#define STREAM_AUDIO_SAMPLE_RATE_32K  32000
#define STREAM_AUDIO_SAMPLE_RATE_44K1 44100
#define STREAM_AUDIO_SAMPLE_RATE_48K  48000
#define STREAM_AUDIO_SAMPLE_RATE_96K  96000

/****************************************************************************
Public Macro Declarations
*/


/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

/****************************************************************************
Functions from stream_if.c
*/

/**
 * \brief Return the external id for a specified endpoint. Whether a source or
 *        sink is requested is parameterised.
 *        <br> The endpoint might be created as a result of this
 *        call dependant on device type.<br> If the source could
 *        not be found then the id will be returned as 0.<br>
 *
 * \param con_id connection ID of the originator of this request
 * \param device the endpoint device type, such as SCO, USB, PCM or I2S.
 * \param num_params the number of parameters passed in the params array
 * \param params an array of typically 2 values a supporting parameter
 * 		  that typically specifies the particular instance and channel
 * 		  of the device type.
 * \param dir Whether a source or sink is being requested
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_get_endpoint(unsigned con_id, unsigned device, unsigned num_params,
        unsigned *params, ENDPOINT_DIRECTION dir,
        bool (*callback)(unsigned con_id, unsigned status, unsigned source_id));

/**
 * \brief Close and release the resources held by an endpoint
 *
 * \param con_id connection ID of the originator of this request
 * \param endpoint_id endpoint id to close
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_close_endpoint(unsigned con_id, unsigned endpoint_id,
        bool (*callback)(unsigned con_id, unsigned status));

/**
 * \brief Configure a key belonging to a resource with a specific value
 *
 * \param con_id connection ID of the originator of this request
 * \param ep_id source or sink endpoint id
 * \param key parameter key to be configured
 * \param value value to be assigned to the parameter key
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 *
 */
void stream_if_configure_sid(unsigned con_id, unsigned ep_id, unsigned int key, uint32 value,
        bool (*callback)(unsigned con_id, unsigned status));

/**
 * \brief synchronise two stream endpoints <br>two or more
 *        stream sync commands can be chained together<br> if
 *        either endpoint is 0 it removes the specified endpoint
 *        from a sync group
 *
 * \param con_id connection ID of the originator of this request
 * \param ep_id1 first source or sink endpoint id
 * \param ep_id2 second source or sink endpoint id
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_sync_sids(unsigned con_id, unsigned ep_id1, unsigned ep_id2,
        bool (*callback)(unsigned con_id, unsigned status));

/**
 * \brief Create a connection between a source and a sink
 *
 * \param con_id connection ID of the originator of this request
 * \param source_id source endpoint id
 * \param sink_id sink endpoint id
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_connect(unsigned con_id, unsigned source_id, unsigned sink_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned transform_id));


/**
 * \brief It is same as stream_if_connect if state_info is NULL. This API is
 * used when endpoints has been already created (state_info contains the
 * endpoint bufer details for connection) and a tranform id is allocated.
 * It is typically used when dual core feature is in use.
 *
 * \param con_id connection ID of the originator of this request.
 * \param source_id source endpoint id.
 * \param sink_id sink endpoint id.
 * \param state_info Endpoint information for transform.
 * \param transform_id An allocated transform id to be used for connection
 * \param callback the callback function to be called when the result is
 * 		  known
 */
void stream_if_transform_connect(unsigned con_id, unsigned source_id,
                                 unsigned sink_id, unsigned transform_id,
                                 STREAM_CONNECT_INFO *state_info,
                                 bool (*callback)(unsigned con_id,
                                 unsigned status, unsigned transform_id));
/**
 * \brief Destroy a connection between a source and a sink
 *        based on the transform id
 *
 * \param con_id connection ID of the originator of this request
 * \param count
 * \param transforms pointer to an array of transform ids
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_transform_disconnect(unsigned con_id, unsigned count, unsigned *transforms,
        bool (*callback)(unsigned con_id, unsigned status, unsigned count));

/**
 * \brief Destroy a connection between a source and a sink
 *        based on the transform id. It is same as stream_if_transform_disconnect
 *        if success_count is 0. This is typically used while disconnecting transforms
 *        spanned across dual cores.
 *
 * \param con_id connection ID of the originator of this request
 * \param count number of transform ids in the list
 * \param transforms pointer to an array of transform ids
 * \param success_count The number of transform ids already disconnected
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_part_transform_disconnect(unsigned con_id, unsigned count,
                                            unsigned *transforms,
                                            unsigned success_count,
        bool (*callback)(unsigned con_id, unsigned status, unsigned total_count));

/**
 * \brief Destroy a connection between a source and a sink
 *        based on the transform id
 *
 * \param con_id connection ID of the originator of this request
 * \param source source enpoint ID
 * \param sink sink endpoint ID
 * \param callback the callback function to be called when the result is
 * 		  known
 *
 */
void stream_if_disconnect(unsigned con_id, unsigned source_id, unsigned sink_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned transform_id1, unsigned transform_id2));


/**
 * \brief Find source's natural sink for bi-directional endpoint sets
 *
 * \param con_id connection ID of the originator of this request
 * \param source_id source endpoint id
 * \param callback the callback function to be called when the result is known
 *
 */
void stream_if_get_sink_from_source(unsigned con_id, unsigned source_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned sink_id));

/**
 * \brief Find sink's natural source for bi-directional endpoint sets
 *
 * \param con_id connection ID of the originator of this request
 * \param sink_id sink endpoint id
 * \param callback the callback function to be called when the result is known
 *
 */
void stream_if_get_source_from_sink(unsigned con_id, unsigned sink_id,
        bool (*callback)(unsigned con_id, unsigned status, unsigned source_id));

/**
 * \brief Find transform connected to source/sink, if any
 *
 * \param con_id connection ID of the originator of this request
 * \param sid source or sink endpoint id
 * \param callback the callback function to be called when the result is known
 *
 */
void stream_if_transform_from_ep(unsigned con_id, unsigned sid,
        bool (*callback)(unsigned con_id, unsigned status, unsigned tr_id));

/**
 * \brief Passes on a kick from an endpoint to the thing it's connected to
 *
 * \param ep_id endpoint which has generated a kick (identified by external ID).
 *
 */
void stream_if_propagate_kick(unsigned ep_id);

/**
 * \brief Query information about a stream endpoint
 *
 * \param id Source or Sink ID to query
 * \param key Info Key to look up
 * \param value Will be modified to point to the value of the key requested
 * \return TRUE if everything went ok, FALSE on error (e.g. unknown key)
 *
 */
bool stream_if_get_info(unsigned id, unsigned key, uint32* value);


/**
 * \brief Get list of external connection/transform IDs with their endpoint IDs
 *
 * \param con_id connection ID of the originator of this request
 * \param source_id external ID of the source endpoint (if filtering based on that) or zero for no filtering
 * \param sink_id external ID of the sink endpoint (if filtering based on that) or zero for no filtering
 * \param callback the callback function to be called with list of external ID triads: (transform ID, source ID, sink ID)
 *
 */
void stream_if_get_connection_list(unsigned con_id, unsigned source_id, unsigned sink_id,
                                   bool (*callback)(unsigned con_id, unsigned status, unsigned length,
                                                    const unsigned *info_list));


/**
 * \brief Set the system streaming rate
 *
 * \param sampling_rate sampling rate for the system
 * \return TRUE if successful else FALSE
 */
bool stream_if_set_system_sampling_rate(uint32 sampling_rate);

/**
 * \brief Get the system streaming rate
 *
 * \return system sampling_rate
 */
uint32 stream_if_get_system_sampling_rate(void);

/**
 * \brief Set the system kick period
 * 
 * \param kp kick period for the system 
 * \return TRUE if successful else FALSE
 */
bool stream_if_set_system_kick_period(TIME_INTERVAL kp);

/**
 * \brief Get the system kick period 
 * 
 * This is the maximum value across all endpoints which have configurable kick periods
 * 
 * \return kick period in microseconds
 */
TIME_INTERVAL stream_if_get_system_kick_period(void);

#ifdef INSTALL_MCLK_SUPPORT
/**
 * \brief Activate mclk output for an audio interface

 * \param con_id connection ID of the originator of this request
 * \param ep_id for the endpoint
 * \param activate_output if not 0, user wants to activate mclk OUTPUT for this endpoint, otherwise it will
 *        de-activate the output. Activation/De-activation request will only be done if:
 *        - the endpoint can have mclk output (e.g i2s master)
 *        - the audio subsystem can route the MCLK
 * \param enable_mclk makes the mclk available to use by the endpoint. For an interface to use MCLK we need
 *        to make sure that the MCLK is available and stable this should be able to be done automatically
 *        before an interface gets activated(normally at connection point), so we might deprecate this
 *        flag in future.
 *  \param callback callback function to be called
 */

void stream_if_mclk_activate(unsigned con_id, unsigned ep_id, unsigned activate_output,
                           unsigned enable_mclk, bool (*callback)(unsigned con_id, unsigned status));

/**
 * \brief Set mclk source type, external mclk or local one

 * \param con_id connection ID of the originator of this request
 * \param use_external_mclk if TRUE it will use external clock when request is
 *        received from audio interfaces
 * \param external_mclk_freq frequency of the external mclk, this will be ignored
 *        if not using external mclk
 * \param callback callback function to be called
 *
 * NOTE: This interface affects all audio interfaces that are going to
 *       use mclk and not for a particular stream.
 */
void stream_if_set_mclk_source(unsigned con_id, unsigned use_external_mclk,
                            uint32 external_mclk_freq, bool (*callback)(unsigned con_id, unsigned status));
#endif /* #ifdef INSTALL_MCLK_SUPPORT */

/**
 * \brief Provide information about the relative clocking of two endpoints
 * \param ep1_id (External) id of the first endpoint
 * \param ep2_id (External) id of the second endpoint
 * \param same_clock If successful, will be set indicating whether the two
 *                   endpoints have the same clock
 * \return True if the query succeeded; false otherwise, e.g. if either
 *         of the endpoints does not exist.
 */
bool stream_if_eps_have_same_clock_source(unsigned ep1_id, unsigned ep2_id, bool* same_clock);

/****************************************************************************
Functions from stream_sco_hydra.c
*/

/**
 * \brief Causes the kick time scheduling to be updated for the to-air and
 * from-air side of a sco link when new link timing information arrives from
 * BT.
 *
 * \param hci_handle  The hci_handle for sco link that the update belongs to.
 *
 */
void stream_sco_params_update(unsigned int hci_handle);


/****************************************************************************
Functions from stream_operator.c
*/

/**
 * \brief Creates an operator endpoint with the specified operator endpoint id
 *
 * \param opidep the external operator endpoint id
 * \param con_id connection ID of the originator of this request
 *
 * \return \c pointer to the created endpoint if successful, otherwise \c NULL
 *
 */
ENDPOINT *stream_create_operator_endpoint( unsigned opidep, unsigned con_id);

/**
 * \brief Enable all operator endpoints specified by the operator external id
 *
 * \param opid the external operator id
 *
 */
void stream_enable_operator_endpoints( unsigned opid );

/**
 * \brief Request to clear RM decision flags for all operators in the chain
 * (the operator with RM_DEC_UNSETTABLE active will not be affected).
 *
 * \param opidep the external operator endpoint id
 *
 */
void stream_disable_rate_matching_decision( unsigned opidep );

/**
 * \brief Disable all operator endpoints specified by the operator external id
 *
 * \param opid the external operator id
 *
 */
void stream_disable_operator_endpoints( unsigned opid );

/**
 * \brief Destroys an operator endpoint specified by its external id
 *
 * \param opidep the external operator endpoint id
 *
 */
bool stream_destroy_operator_endpoint( unsigned opidep );

/**
 * \brief Destroys all the endpoints of an operator. Sinks are destroyed
 * first.
 *
 * \param opid The external operator id
 * \param num_sinks The maximum number of sinks the operator could have
 * \param num_sources The maximum number of sources the operator could have
 *
 * \return Whether all endpoints were successfully destroyed
 */
bool stream_destroy_all_operators_endpoints(unsigned opid, unsigned num_sinks, unsigned num_sources);

/**
 * \brief get the endpoint that is connected to an operator's terminal
 *
 * \param opid operators id
 * \param terminal_id terminal ID
 *
 * \return connected operator to the terminal, NULL if not connected
 */
ENDPOINT *stream_get_connected_endpoint_from_terminal_id(unsigned opid,  unsigned terminal_id);

/****************************************************************************
Functions from stream_shadow.c
*/

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
ENDPOINT *stream_create_shadow_endpoint(unsigned epid, unsigned con_id);
void stream_enable_shadow_endpoint( unsigned epid );
void stream_disable_shadow_endpoint( unsigned epid );
bool stream_destroy_shadow_endpoint(unsigned epid);
#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */

/****************************************************************************
Functions from stream.c
*/

/**
 * \brief gets the endpoint from the externally visible id
 *
 * \param id id as seen by the host
 *
 */
ENDPOINT *stream_endpoint_from_extern_id(unsigned id);

/**
 * \brief Closes all endpoints with given con_id
 *
 * \param con_id connetion id for the endpoints that should be
 *        closed
 *
 */
void stream_close_endpoints_with_con_id(unsigned con_id);

/**
 * \brief Sets the connection id for all endpoints in a list.
 *        The endpoints in the list MUST be running on the primary core.
 *        Note as in other stream interface functions the argument con_id
 *        truly means the just creator (or sender id).
 */
void stream_set_endpoint_connection_id(ENDPOINT **ep, unsigned num_eps, unsigned con_id);
    
#ifdef UNIT_TEST_BUILD
/* don't document this one....*/
bool get_timing_information_from_transform(unsigned tid, unsigned *period, unsigned *hard_endpoint, unsigned *proc_time);
#endif /*DESKTOP_TEST_BUILD*/

/**
 * \brief  Find out whether endpoint exists in the chain(s).
 *
 * \param  ep - The ENDPOINT structure pointer for the endpoint to be found.
 *
 * \return TRUE/FALSE depending on endpoint found or not.
 */
extern bool stream_does_ep_exist(ENDPOINT* ep);

/****************************************************************************
Functions from stream_monitor_interrupt.c
*/

/****************************************************************************
Functions from stream_audio_bluecore.c
*/
#ifdef CHIP_BASE_BC7
/**
 * \brief message handler for messages relating to the audio hardware
 * on a bluecore device.
 *
 * \param msg_id The id of the message to handle.
 * \param pdu A pointer to the message payload.
 */
extern void bc_audio_ep_message_handler(unsigned msg_id, unsigned *pdu);
#ifdef INSTALL_SPDIF
/**
 * \brief message handler for messages relating to the spdif hardware
 * on a bluecore device.
 *
 * \param msg_id The id of the message to handle.
 * \param pdu A pointer to the message payload.
 */
extern void bc_spdif_ep_message_handler(unsigned msg_id, unsigned *pdu);
#endif /* #ifdef INSTALL_SPDIF */
#endif /* CHIP_BASE_BC7 */

#endif /* STREAM_H */

