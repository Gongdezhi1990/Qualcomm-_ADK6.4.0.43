/*******************************************************************************
Copyright (c) 2013 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

/* 
    Library allowing user to add debug printfs to their code and be able to
    enable / disable them as required using a single switch.
*/

/*!
\defgroup print print
\ingroup vm_libs

\brief Debug print functions.

\section print_intro INTRODUCTION
  This library allows applications to contain debug printfs in their code and
  to be able to enable / disable them as required by defining (or not) the
  following 'switch'.

  @#define DEBUG_PRINT_ENABLED

  PRINT and CPRINT take the same arguments as printf and cprintf.

@{

*/

#ifdef PRINT
#undef PRINT
#endif
#ifdef CPRINT
#undef CPRINT
#endif

#ifdef DEBUG_PRINT_ENABLED
#include <stdio.h>
#define PRINT(x) printf x
#define CPRINT(x) cprintf x
#else
#define PRINT(x)  {}
#define CPRINT(x) {}
#endif

/** @} */
