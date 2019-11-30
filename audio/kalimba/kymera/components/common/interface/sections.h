/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 ************************************************************************//**
 * \file sections.h
 * Sections definitions for Kalimba
 *
 * MODULE : Sections
 *
 ****************************************************************************/

#ifndef SECTIONS_H
#define SECTIONS_H

/* INLINE_SECTION is used by capability download feature;
   Force inline functions in header files to be placed in PM
   so that they get garbage collected when not used.
 */
#ifdef __GNUC__
#define INLINE_SECTION
#else
#define INLINE_SECTION   _Pragma("codesection PM")
#endif

/* RUN_FROM_PM_RAM leading a function definition
   instructs the compiler/linker (kcc) to place the code in RAM.
 */
#if defined(CRESCENDO_HOIST_CODE_TO_RAM) && !defined(__GNUC__)
#define RUN_FROM_PM_RAM  _Pragma("codesection PM_RAM")
#else
#define RUN_FROM_PM_RAM
#endif

#endif /* SECTIONS_H */
