/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef _XOVER_WRAPPER_DEFS_H_
#define _XOVER_WRAPPER_DEFS_H_

/****************************************************************************
Public Constant Definitions
*/
#define XOVER_VERSION_LENGTH                       2

/** default block size for this operator's terminals */
#define XOVER_CAP_DEFAULT_BLOCK_SIZE                   1
#define XOVER_CAP_MAX_CHANNELS                         8
#define XOVER_PEQ_PARAMETERS_SIZE                     (44*sizeof(unsigned))

/** channel mask values */
#define CHAN_MASK_0                                    1
#define CHAN_MASK_1                                    2
#define CHAN_MASK_2                                    4
#define CHAN_MASK_3                                    8
#define CHAN_MASK_4                                    16
#define CHAN_MASK_5                                    32
#define CHAN_MASK_6                                    64
#define CHAN_MASK_7                                    128

#endif /* _XOVER_WRAPPER_DEFS_H_ */
