/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 *  \file a2dp_decode_cap_asm.h
 */

#ifndef A2DP_DECODE_CAP_ASM_H
#define A2DP_DECODE_CAP_ASM_H

#include "a2dp_header_struct_asm_defs.h"
#include "a2dp_header_defs.h"

.CONST  $a2dp_decode.HEADER_TYPE_FIELD    $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.TYPE_FIELD;
.CONST  $a2dp_decode.HEADER_SIZE_FIELD    $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.HDR_SIZE_FIELD;
.CONST  $a2dp_decode.CID_FIELD            $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.CID_FIELD;
.CONST  $a2dp_decode.BYTES_LEFT           $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.BYTES_LEFT_FIELD;
.CONST  $a2dp_decode.DECODE_FIELD         $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.STRIP_DECODE_FRAME_FIELD;
.CONST  $a2dp_decode.GET_BITS_ADDR_FIELD  $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.GET_BITS_FIELD;

#ifdef INSTALL_OPERATOR_APTX_SHUNT_DECODER
.CONST  $a2dp_decode.ODD_BYTE_POS        $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.ODD_BYTE_POS_FIELD;
.CONST  $a2dp_decode.DATA_ALIGNED        $a2dp_header_struct.A2DP_HEADER_PARAMS_struct.DATA_ALIGNED_FIELD;
#endif /* INSTALL_OPERATOR_APTX_SHUNT_DECODER */

.CONST  $a2dp_decode.BFRAME_TYPE_MASK      A2DP_STRIP_BFRAME;
.CONST  $a2dp_decode.RTP_TYPE_MASK         A2DP_STRIP_RTP;
.CONST  $a2dp_decode.MP_TYPE_MASK          A2DP_STRIP_MP;
.CONST  $a2dp_decode.CP_TYPE_MASK          A2DP_STRIP_CP;

.CONST  $a2dp_decode.BFRAME_HDR_SIZE       A2DP_BFRAME_HDR_SIZE;
.CONST  $a2dp_decode.RTP_HDR_SIZE          A2DP_RTP_HDR_SIZE;
.CONST  $a2dp_decode.MP_HDR_SIZE           A2DP_MP_HDR_SIZE;
.CONST  $a2dp_decode.CP_HDR_SIZE           A2DP_CP_HDR_SIZE;

.CONST  $a2dp_decode.ODD_BYTE_POS_SET      0x100;
.CONST  $a2dp_decode.RTP_HEADER_MASK       0xFF80;
.CONST  $a2dp_decode.RTP_EXPECTED_HEADER   0x8000;

.CONST  $a2dp_decode.OUTPUT_BLOCK_SIZE     128;

 // error codes - extension to codec library error codes
.CONST  $a2dp_decode.SUCCESS               0;
.CONST  $a2dp_decode.HEADER_NOT_FOUND      6;

#endif /* A2DP_DECODE_CAP_ASM_H */

