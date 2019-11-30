/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#include "hydra/hydra_types.h"
#include "hydra_log/hydra_log.h"

#include "portability/portability.h"

/* Default implementation of _init() which is linked into a library.
 * Can be re-defined by any other implementation in the firmware. */
void _init(void)
{
    L2_DBG_MSG("_init: no USB device descriptors");
}
