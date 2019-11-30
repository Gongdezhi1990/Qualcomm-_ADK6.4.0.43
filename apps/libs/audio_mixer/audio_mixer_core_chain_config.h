/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_core_chain_config.h

DESCRIPTION
       Implementation of chain construction functions
*/

#ifndef _AUDIO_MIXER_CORE_CHAIN_CONFIG_H_
#define _AUDIO_MIXER_CORE_CHAIN_CONFIG_H_

#include <operators.h>
#include <chain.h>

typedef enum _audio_mixer_core_roles
{
    sync_role,
    mixer_role
} audio_mixer_core_roles_t;

/****************************************************************************
DESCRIPTION
    Get the chain configuration for the core mixer object
RETURNS
    Core mixer chain configuration
*/
const chain_config_t* mixerCoreGetChainConfig(connection_type_t connection_type, bool mono_path_enabled);

/****************************************************************************
DESCRIPTION
    Get source sync sink group configuration. This defines how inputs to
    source sync are grouped.
RETURNS
    Pointer to the sink group configuration
*/
const source_sync_sink_group_t* mixerCoreGetSourceSyncSinkGroup(connection_type_t connection_type, uint16* num_groups);

/****************************************************************************
DESCRIPTION
    Get mono path source sync sink group configuration. This defines how inputs to
    source sync are grouped.
RETURNS
    Pointer to the mono path sink group configuration
*/
const source_sync_sink_group_t* mixerCoreGetMonoPathSourceSyncSinkGroup(connection_type_t connection_type, uint16* num_groups);

/****************************************************************************
DESCRIPTION
    Get source sync source group configuration. This defines how outputs from
    source sync are grouped.
RETURNS
    Pointer to the source group configuration
*/
const source_sync_source_group_t* mixerCoreGetSourceSyncSourceGroup(connection_type_t connection_type, uint16* num_groups);

/****************************************************************************
DESCRIPTION
    Get mono path source sync source group configuration. This defines how outputs from
    source sync are grouped.
RETURNS
    Pointer to the mono path source group configuration
*/
const source_sync_source_group_t* mixerCoreGetMonoPathSourceSyncSourceGroup(connection_type_t connection_type, uint16* num_groups);

/****************************************************************************
DESCRIPTION
    Get source sync source routes for general connection. This defines how inputs are
    routed to outputs, and must be free'd after use.
RETURNS
    Pointer to the routes configuration
*/
source_sync_route_t* mixerCoreGetSourceSyncChannelModeRoutes(AUDIO_MUSIC_CHANNEL_MODE_T channel_mode, uint16* num_routes, const uint32 sample_rate);

/****************************************************************************
DESCRIPTION
    Get source sync source routes configuration. This defines how inputs are
    routed to outputs
RETURNS
    Pointer to the routes configuration
*/
source_sync_route_t* mixerCoreGetSourceSyncRoutes(connection_type_t connection_type, AUDIO_MUSIC_CHANNEL_MODE_T channel_mode, uint16* num_routes, const uint32 sample_rate);

/****************************************************************************
DESCRIPTION
    Get mono path source sync source routes configuration. This defines how inputs are
    routed to outputs
RETURNS
    Pointer to the mono path routes configuration
*/
source_sync_route_t* mixerCoreGetMonoPathSourceSyncRoutes(connection_type_t connection_type, uint16* num_routes, const uint32 sample_rate);

/****************************************************************************
DESCRIPTION
    Get the number of music inputs provided by the core chain
RETURNS
    Number of music inputs
*/
unsigned mixerCoreGetNumberOfMusicChannels(connection_type_t connection_type, bool mono_path_enabled);

#endif /* _AUDIO_MIXER_CORE_CHAIN_CONFIG_H_ */
