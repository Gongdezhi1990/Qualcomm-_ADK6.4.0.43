/*
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
\file    
\ingroup sink_app
\brief
    Handles AVRCP Browsing Channel features.
*/

#ifndef _SINK_AVRCP_BROWSING_H_
#define _SINK_AVRCP_BROWSING_H_
#ifdef ENABLE_AVRCP


#include <avrcp.h>


#define AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC 3           /* number of media attributes to retrieve information for when only requiring limited track data */
#define AVRCP_NUMBER_MEDIA_ATTRIBUTES_FULL 7            /* number of media attributes to retrieve information for when requiring full track data */


extern const uint8 avrcp_retrieve_media_attributes_basic[AVRCP_NUMBER_MEDIA_ATTRIBUTES_BASIC * 4];
extern const uint8 avrcp_retrieve_media_attributes_full[AVRCP_NUMBER_MEDIA_ATTRIBUTES_FULL * 4];


#ifdef ENABLE_AVRCP_BROWSING


#define AVRCP_MAX_MEDIA_PLAYERS 3                       /* maximum number of media players to retrieve information for */
#define AVRCP_MAX_NOW_PLAYING_TRACKS 3                  /* maximum number of now playing tracks to retrieve information for */
#define AVRCP_BROWSING_PLAYER_FEATURES_INVALID 0xffff   /* media player features have not been retrieved */
#define AVRCP_BROWSING_MEDIA_NOT_PLAYABLE 0             /* media cannot be played - the same value as Is Playable field of Folder Item */
#define AVRCP_BROWSING_MEDIA_PLAYABLE 1                 /* media can be played  - the same value as Is Playable field of Folder Item */

/* defines for stored Octets 7 and 8 of media player features */
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_GROUP_NAVIGATION (1 << 1) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_AVRCP14 (1 << 2) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_BROWSING (1 << 3)
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_SEARCHING (1 << 4) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_ADD_TO_NOW_PLAYING (1 << 5) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_UIDS_UNIQUE (1 << 6) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O7_ONLY_BROWSABLE_IF_ADDRESSED (1 << 7) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O8_ONLY_SEARCHABLE_IF_ADDRESSED (1 << 8) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O8_NOW_PLAYING (1 << 9) 
#define AVRCP_BROWSING_PLAYER_FEATURE_O8_UID_PERSISTENCY (1 << 10) 

/* Get Folder Items types */
#define AVRCP_BROWSABLE_ITEM_TYPE_MEDIA_PLAYER 1
#define AVRCP_BROWSABLE_ITEM_TYPE_FOLDER 2
#define AVRCP_BROWSABLE_ITEM_TYPE_MEDIA_ELEMENT 3

#define AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_HEADER_SIZE 8  /* amount of fixed data in Source of AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_T message before variable length text */
#define AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_HEADER_SIZE 3     /* amount of fixed data in Source of AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T message before item specific data */
#define AVRCP_BROWSE_MEDIA_PLAYER_ITEM_HEADER_SIZE 28       /* amount of fixed data in Source of AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T message when item is media player */
#define AVRCP_BROWSE_MEDIA_ELEMENT_ITEM_HEADER_SIZE 14      /* amount of fixed data in Source of AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T message when item is media element */
#define AVRCP_BROWSE_FOLDER_ITEM_HEADER_SIZE 14             /* amount of fixed data in Source of AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T message when item is folder */

#define AVRCP_BROWSING_DISCONNECT_ON_IDLE_TIMER 5000        /* delay before disconnecting browsing channel in idle state */

#define SCOPE_NON_ZERO(scope) (scope + 1)                    /* scope starts at zero index, convert it to start at index of one */ 

/* queued application browsing commands */
typedef enum
{
    AVRCP_BROWSING_NOW_PLAYING_TRACK,
    AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS,
    AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST,
    AVRCP_BROWSING_PLAY_ITEM,
    AVRCP_BROWSING_RETRIEVE_FILESYSTEM,
    AVRCP_BROWSING_RETRIEVE_NUMBER_OF_ITEMS,
    AVRCP_BROWSING_CHANGE_PATH,
    AVRCP_BROWSING_ADD_TO_NOW_PLAYING,
    AVRCP_BROWSING_SET_PLAYER,
    AVRCP_BROWSING_SEARCH,
    AVRCP_BROWSING_RETRIEVE_SEARCH_LIST,
    AVRCP_BROWSING_DISCONNECT_IDLE
} avrcp_browsing_command;

typedef struct
{    
    uint16 index;    
    uint32 track_index_high;
    uint32 track_index_low;
    bool full_attributes;
} AVRCP_BROWSING_NOW_PLAYING_TRACK_T;

typedef struct
{
    uint16 start_index;
    uint16 end_index;
} AVRCP_BROWSING_RETRIEVE_MEDIA_PLAYERS_T;    

typedef struct
{
    uint16 start_index;
    uint16 end_index;
} AVRCP_BROWSING_RETRIEVE_NOW_PLAYING_LIST_T;

typedef struct
{
    avrcp_browse_uid uid;
    avrcp_browse_scope scope;
} AVRCP_BROWSING_PLAY_ITEM_T;

typedef struct
{
    uint16 start_index;
    uint16 end_index;
} AVRCP_BROWSING_RETRIEVE_FILESYSTEM_T;

typedef struct
{
    avrcp_browse_scope scope;
} AVRCP_BROWSING_RETRIEVE_NUMBER_OF_ITEMS_T;

typedef struct
{
    avrcp_browse_direction direction;
    avrcp_browse_uid folder_uid;
} AVRCP_BROWSING_CHANGE_PATH_T;

typedef struct
{
    avrcp_browse_uid uid;
    avrcp_browse_scope scope;
} AVRCP_BROWSING_ADD_TO_NOW_PLAYING_T;

typedef struct
{
    uint16 player_id;
} AVRCP_BROWSING_SET_PLAYER_T;

typedef struct
{
    uint16 size_string;
    Source string;
} AVRCP_BROWSING_SEARCH_T;

typedef struct
{
    uint16 start_index;
    uint16 end_index;
} AVRCP_BROWSING_RETRIEVE_SEARCH_LIST_T;

typedef struct
{
    AVRCP *avrcp;
} AVRCP_BROWSING_DISCONNECT_IDLE_T;

/* initialization */
void sinkAvrcpBrowsingInit(uint16 link_index);

/* Utility function to check if AVRCP browsing is supported */
bool sinkAvrcpBrowsingIsSupported(uint16 Index);

/* Utility function to check if AVRCP browsing supports Group Navigation feature */
bool sinkAvrcpBrowsingIsGroupNavigationSupported(uint16 Index);

/* message handler */
void avrcpBrowsingHandler(Task task, MessageId id, Message message);

/* Utility function to clear pending messages from the browsing handler */
void sinkAvrcpBrowsingFlushHandlerTask(uint16 index);

void sinkAvrcpBrowsingDisconnectOnIdleRequest(AVRCP *avrcp);

/* Utility function to cancel disconnectOnIdle request */
void sinkAvrcpBrowsingCancelDisconnectOnIdle(uint16 index);

/* get folder items */
void sinkAvrcpBrowsingGetFolderItemsCfm(const AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T *msg);

void sinkAvrcpBrowsingGetNumberOfItemsCfm(const AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM_T *msg);


/* now playing */
void sinkAvrcpBrowsingRetrieveNowPlayingTrackRequest(uint16 Index, uint32 track_index_high, uint32 track_index_low, bool full_attributes);

bool sinkAvrcpBrowsingRetrieveNowPlayingListRequest(uint16 start_index, uint16 end_index);

bool sinkAvrcpBrowsingPlayNowPlayingItemRequest(avrcp_browse_uid uid);

void sinkAvrcpBrowsingGetItemAttributesCfm(const AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_T *msg);

/* multiple media players */
bool sinkAvrcpBrowsingMultipleMediaPlayersIsSupported(uint16 Index);

bool sinkAvrcpBrowsingRetrieveMediaPlayersRequest(uint16 start_index, uint16 end_index);

bool sinkAvrcpBrowsingSetAddressedMediaPlayerRequest(uint16 player_id);

bool sinkAvrcpBrowsingSetBrowsedMediaPlayerRequest(uint16 player_id);

void sinkAvrcpBrowsingSetBrowsedPlayerCfm(const AVRCP_BROWSE_SET_PLAYER_CFM_T *msg);

void sinkAvrcpBrowsingAddressedPlayerChangedInd(const AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND_T *msg);

/* browse filesystem */

void sinkAvrcpBrowsingRegisterChangedUIDNotification(AVRCP *avrcp);

bool sinkAvrcpBrowsingRetrieveFileSystemRequest(uint16 start_index, uint16 end_index);

bool sinkAvrcpBrowsingRetrieveNumberOfItemsRequest(avrcp_browse_scope browsing_scope);

bool sinkAvrcpBrowsingChangePathRequest(avrcp_browse_direction direction, avrcp_browse_uid folder_uid);

bool sinkAvrcpBrowsingPlayBrowsedItemRequest(avrcp_browse_uid uid);

bool sinkAvrcpBrowsingAddBrowsedToNowPlayingRequest(avrcp_browse_uid uid);

void sinkAvrcpBrowsingChangePathCfm(const AVRCP_BROWSE_CHANGE_PATH_CFM_T *msg);

void sinkAvrcpBrowsingUIDsChangedInd(const AVRCP_EVENT_UIDS_CHANGED_IND_T *msg);

void sinkAvrcpBrowsingAvailablePlayersChangedInd(const AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND_T *msg);

/* search */
bool sinkAvrcpSearchIsSupported(uint16 Index);
        
bool sinkAvrcpBrowsingSearchRequest(uint16 size_string, Source string);   

bool sinkAvrcpBrowsingRetrieveSearchItemsRequest(uint16 start_index, uint16 end_index);

bool sinkAvrcpBrowsingPlaySearchItemRequest(avrcp_browse_uid uid);

void sinkAvrcpBrowsingSearchCfm(const AVRCP_BROWSE_SEARCH_CFM_T *msg);

#else /* If Browsing is not enabled */

/* initialisation */
#define sinkAvrcpBrowsingInit(link_index) ((void)0)

#define sinkAvrcpBrowsingIsSupported(Index) (FALSE)

#define sinkAvrcpBrowsingIsGroupNavigationSupported(Index) (FALSE)

#define sinkAvrcpBrowsingCancelDisconnectOnIdle(index) ((void)0)

#define sinkAvrcpBrowsingRegisterChangedUIDNotification(avrcp) ((void)0)

#define sinkAvrcpBrowsingFlushHandlerTask(index) ((void)0)

#define sinkAvrcpBrowsingMultipleMediaPlayersIsSupported(Index) (FALSE)

#define sinkAvrcpBrowsingGetItemAttributesCfm(message) ((void)0)

#define sinkAvrcpBrowsingAddressedPlayerChangedInd(message) ((void)0)

#define sinkAvrcpBrowsingGetFolderItemsCfm(message) ((void)0)

#define sinkAvrcpBrowsingGetNumberOfItemsCfm(message) ((void)0)

#define sinkAvrcpBrowsingSetBrowsedPlayerCfm(message) ((void)0)

#define sinkAvrcpBrowsingChangePathCfm(message) ((void)0)

#define sinkAvrcpBrowsingUIDsChangedInd(message) ((void)0)

#define sinkAvrcpBrowsingAvailablePlayersChangedInd(message) ((void)0)

#define sinkAvrcpBrowsingSearchCfm(message) ((void)0)

#define sinkAvrcpBrowsingRetrieveNowPlayingTrackRequest(Index, track_index_high, track_index_low, full_attributes) ((void)0)

#endif /* ENABLE_AVRCP_BROWSING */

#endif /* ENABLE_AVRCP */

#endif /* _SINK_AVRCP_BROWSING_H_ */


