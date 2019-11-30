/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_common.h
 * \ingroup ps
 *
 * Persistent Storage common declarations.
 */

#ifndef PS_COMMON_H
#define PS_COMMON_H

#include "types.h"
#include "status_prim.h"
#include "persist_prim.h"

/** Data type of the PS key values and the rank ID for persistence. 
  * Both are aliased from the external definition,
  * so that API specification is respected regardless of tooling conventions.
  */
typedef PERSIST_ID_TYPE PS_KEY_TYPE;
typedef PERSIST PERSISTENCE_RANK;

/**  Callback function types */
typedef bool (*PS_READ_CALLBACK)(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank,
                                 uint16 length, unsigned * data, STATUS_KYMERA status, uint16 extra_status_info);

typedef bool (*PS_WRITE_CALLBACK)(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank,
                                  STATUS_KYMERA status, uint16 extra_status_info);

typedef bool (*PS_ENTRY_DELETE_CALLBACK)(void* instance_data, PS_KEY_TYPE key, STATUS_KYMERA status, uint16 extra_status_info);

typedef bool (*PS_DELETE_CALLBACK)(void* instance_data, PERSISTENCE_RANK rank, STATUS_KYMERA status, int16 extra_status_info);

typedef bool (*PS_SHUTDOWN_CALLBACK)(unsigned conn_id);

#endif /* PS_COMMON_H */
