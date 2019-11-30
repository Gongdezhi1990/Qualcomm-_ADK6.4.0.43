/***************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_development.h
    
DESCRIPTION
    Contains development related macro's useful for debugging.
*/

#ifndef _SINK_DEVELOPMENT_H_
#define _SINK_DEVELOPMENT_H_


#include <panic.h>

#define DEVELOPMENT_BUILDx

#ifdef DEVELOPMENT_BUILD
    
    #include <print.h>
    /* Fatal errors must always cause a Panic; print a useful error statement then Panic */
    #define FATAL_ERROR(x)      {PRINT(x); Panic();}
    /* Safe errors should not be ignored for development builds - print a useful error statement then Panic */
    #define TOLERATED_ERROR(x)  FATAL_ERROR(x)
    
#else
    
    /* Fatal errors must always cause a Panic, even in release builds */
    #define FATAL_ERROR(x)      {Panic();}
    /* Safe errors can be ignored for release builds */
    #define TOLERATED_ERROR(x)
    
#endif


#endif /* _SINK_DEVELOPMENT_H_ */
