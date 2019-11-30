/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      Implementation for VM adaptation of logging to pydbg
*/

#include <logging.h>

/*! Sequence variable used in debug output so that missing log items can be detected. 

    This sequence can be removed by defining LOGGING_EXCLUDE_SEQUENCE before logging.h
    is included. This has the effect of allowing more debug (as the sequence is itself
    a logging parameter), but loses the ability to detect missing logging.

    Note that the sequence can be excluded on a per file basis.
 */
uint16 globalDebugLineCount = 0;

