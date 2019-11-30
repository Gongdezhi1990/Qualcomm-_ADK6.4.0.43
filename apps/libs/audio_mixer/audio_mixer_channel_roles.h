/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_channel_roles.h

DESCRIPTION
    Definition of common channel roles, used to connect chains together
*/

#ifndef _AUDIO_MIXER_CHANNEL_ROLES_H_
#define _AUDIO_MIXER_CHANNEL_ROLES_H_

typedef enum
{
    mixer_left_channel,
    mixer_right_channel,
    mixer_secondary_left_channel,
    mixer_secondary_right_channel,
    mixer_wired_sub_channel,
    mixer_voice_channel,
    mixer_music_left_channel,
    mixer_music_right_channel,
    mixer_music_downmix_left_channel,
    mixer_music_downmix_right_channel,
    mixer_prompt_left_channel,
    mixer_prompt_right_channel
} audio_mixer_channel_role_t;

#endif /* _AUDIO_MIXER_CHANNEL_ROLES_H_ */
