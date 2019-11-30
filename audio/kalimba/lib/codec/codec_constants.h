/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  codec_constants.h
 * \ingroup codec
 *
 * CODEC library C/ASM shared constants header file. <br>
 *
 */

#ifndef CODEC_CONSTANTS_H
#define CODEC_CONSTANTS_H

/****************************************************************************
Public Constant Declarations
*/
#define CODEC_NORMAL_DECODE     0
#define CODEC_NO_OUTPUT         1
#define CODEC_GOBBLE            2

#define CODEC_SUCCESS 0
#define CODEC_NOT_ENOUGH_INPUT_DATA 1
#define CODEC_NOT_ENOUGH_OUTPUT_SPACE 2
#define CODEC_ERROR 3
#define CODEC_EOF 4
#define CODEC_FRAME_CORRUPT 5

#endif /* CODEC_CONSTANTS_H */

