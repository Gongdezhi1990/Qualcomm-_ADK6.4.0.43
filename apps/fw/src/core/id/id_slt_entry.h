/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef ID_SLT_ENTRY_H
#define ID_SLT_ENTRY_H

#include "id/id.h"

#define CORE_ID_SLT_ENTRY(m)                                                \
    SLT_ENTRY(m, (APPS_SLT_ID_VERSION_STRING_PACKED, build_id_string))   \
    SLT_ENTRY(m, (APPS_SLT_ID_VERSION_INTEGER, &build_id_number))

#endif /* ID_SLT_ENTRY_H */
