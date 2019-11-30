// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
// *****************************************************************************



#ifndef FRAME_PROC_ASM_HEADER_INCLUDED
#define FRAME_PROC_ASM_HEADER_INCLUDED

// Frame Buffer for Processing Modules
.CONST	$frmbuffer.FRAME_PTR_FIELD                 0*ADDR_PER_WORD;
.CONST	$frmbuffer.FRAME_SIZE_FIELD                1*ADDR_PER_WORD;
.CONST	$frmbuffer.BUFFER_SIZE_FIELD               2*ADDR_PER_WORD;
.CONST	$frmbuffer.BUFFER_START_ADDRESS_FIELD      3*ADDR_PER_WORD;
// Extended Data Structure
.CONST	$frmbuffer.CBUFFER_PTR_FIELD	           4*ADDR_PER_WORD;
.CONST	$frmbuffer.INITIAL_FRAME_SIZE_FIELD        5*ADDR_PER_WORD;
.CONST   $frmbuffer.DISTRIBUTE_FUNCPTR_FIELD       6*ADDR_PER_WORD;
.CONST   $frmbuffer.UPDATE_FUNCPTR_FIELD           7*ADDR_PER_WORD;
.CONST   $frmbuffer.THRESHOLD_FIELD                8*ADDR_PER_WORD;
.CONST	$frmbuffer.JITTER_FIELD                    9*ADDR_PER_WORD; 
// Statistics
.CONST   $frmbuffer.DROPPED_SAMPLES_FIELD         10*ADDR_PER_WORD;
.CONST   $frmbuffer.INSERTED_SAMPLES_FIELD        11*ADDR_PER_WORD;
// Working Data
.CONST   $frmbuffer.AMOUNT_TO_DROP_FIELD          12*ADDR_PER_WORD;

// Frame Object Size
.CONST   $frmbuffer.STRUC_SIZE                     4;
.CONST   $frmbuffer.EXTENDED_STRUC_SIZE           13;




#endif  // FRAME_PROC_ASM_HEADER_INCLUDED
