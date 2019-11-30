/*
Copyright (c) 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   
*/

/*!

@file   sink_malloc_debug.h
@brief Macros to add trace and debug functionality around memory allocation
        and free.

    Alternative to the memory allocation functions are included here for
    programming convenience and debug facilities.

    Each of malloc, realloc, calloc and free have Debug versions which add
    debug and can be adjusted to Panic/Not Panic
        e.g. mallocDebug can be adjusted to Panic/NotPanic
             mallocDebugPanic will always panic on Null.
             mallocDebugNoPanic will not panic on Null.

    In addition a newDebug() macro is provided which matches the C++ style
    of new. This is passed a type, mallocs the size necessary and casts the
    result to a pointer to the type.

    The allocated memory can be zeroed by using the Z variant of the function.
    Exceptions are calloc, which does this anyway, and realloc which is 
    resizing an existing block.
        e.g. mallocZDebug will malloc a zeroed block. It can be adjusted to panic
             mallocZDebugPanic is the same, but will panic
*/

#ifndef _SINK_MALLOC_DEBUG_H_
#define _SINK_MALLOC_DEBUG_H_

#include <panic.h>
#include <stdlib.h>

#ifdef DEBUG_MALLOC
    #define mallocDebugPanic(_size)     PanicNull(SinkMallocDebug(__FILE__,__LINE__,(_size)))
    #define mallocDebugNoPanic(_size)   SinkMallocDebug(__FILE__,__LINE__,(_size))

    #define mallocZDebugPanic(_size)    PanicNull(SinkMallocZDebug(__FILE__,__LINE__,(_size)))
    #define mallocZDebugNoPanic(_size)  SinkMallocZDebug(__FILE__,__LINE__,(_size))

    #define reallocDebugPanic(_ptr,_size)   PanicNull(SinkReallocDebug(__FILE__,__LINE__,(_ptr),(_size)))
    #define reallocDebugNoPanic(_ptr,_size) SinkReallocDebug(__FILE__,__LINE__,(_ptr),(_size))

    #define callocDebugPanic(_items,_elem_size)     PanicNull(SinkMallocZDebug(__FILE__,__LINE__,(_items)*(_elem_size)))
    #define callocDebugNoPanic(_items,_elem_size)   SinkMallocZDebug(__FILE__,__LINE__,(_items)*(_elem_size))

    #define newDebugPanic(_type)        (_type *)PanicNull(SinkMallocDebug(__FILE__,__LINE__,(sizeof(_type))))
    #define newDebugNoPanic(_type)      (_type *)SinkMallocDebug(__FILE__,__LINE__,(sizeof(_type)))

    #define newZDebugPanic(_type)       (_type *)PanicNull(SinkMallocZDebug(__FILE__,__LINE__,(sizeof(_type))))
    #define newZDebugNoPanic(_type)     (_type *)SinkMallocZDebug(__FILE__,__LINE__,(sizeof(_type)))

    #define freeDebugPanic(_ptr)        do { \
                                                SinkFreeDebug(__FILE__,__LINE__,(_ptr));\
                                                PanicNull(_ptr);\
                                           } while (0)
    #define freeDebugNoPanic(_ptr)      SinkFreeDebug(__FILE__,__LINE__,(_ptr))

    /* Implementing functions */
    void *SinkMallocDebug ( const char file[], int line, size_t pSize ) ;
    void *SinkMallocZDebug ( const char file[], int line, size_t pSize ) ;
    void *SinkReallocDebug(const char file[], int line,void *ptr,size_t size);
    void SinkFreeDebug ( const char file[], int line, void * ptr ) ;

#else   /* !DEBUG_MALLOC */

    #define mallocDebugPanic(_size)     PanicNull(malloc(_size))
    #define mallocDebugNoPanic(_size)   malloc(_size)

    #define mallocZDebugPanic(_size)    PanicNull(SinkMallocZ(_size))
    #define mallocZDebugNoPanic(_size)  SinkMallocZ(_size)

    #define reallocDebugPanic(_ptr,_size)   PanicNull(realloc((_ptr),(_size)))
    #define reallocDebugNoPanic(_ptr,_size) realloc((_ptr),(_size))

    #define callocDebugPanic(_items,_elem_size)     PanicNull(calloc((_items),(_elem_size)))
    #define callocDebugNoPanic(_items,_elem_size)   calloc((_items),(_elem_size))

    #define newDebugPanic(_type)        PanicUnlessNew(_type)
    #define newDebugNoPanic(_type)      (_type *)malloc(sizeof(_type))

    #define newZDebugPanic(_type)       (_type *)PanicNull(SinkMallocZ(sizeof(_type)))
    #define newZDebugNoPanic(_type)     (_type *)SinkMallocZ(sizeof(_type))

    #define freeDebugPanic(_ptr)        free(PanicNull(_ptr))
    #define freeDebugNoPanic(_ptr)      free(_ptr)

    /* Implementing functions */
    void *SinkMallocZ ( size_t pSize ) ;

#endif  /* DEBUG_MALLOC */

    /* Nested macros to form Panic/NoPanic variants based on 
       DEBUG_MALLOC_PANIC_ACTION. Nesting needed due to the workings
       of pre-processor */
#define FUNC_PANIC_ACTION_IMPL2(func,action) func##action
#define FUNC_PANIC_ACTION_IMPL(func,action) FUNC_PANIC_ACTION_IMPL2(func,action) 
#define FUNC_PANIC_ACTION(bare_function) FUNC_PANIC_ACTION_IMPL(bare_function,DEBUG_MALLOC_PANIC_ACTION)


    /* Bare variants of the memory debug functions. 
       Can choose here whether to use the Panic or NoPanic versions
     */
#ifndef DEBUG_MALLOC_PANIC_ACTION
#define DEBUG_MALLOC_PANIC_ACTION NoPanic
#endif


#define mallocDebug(_size)              FUNC_PANIC_ACTION(mallocDebug)(_size)
#define mallocZDebug(_size)             FUNC_PANIC_ACTION(mallocZDebug)(_size)
#define reallocDebug(_ptr,_size)        FUNC_PANIC_ACTION(reallocDebug)(_ptr,_size)
#define callocDebug(_items,_elem_size)  FUNC_PANIC_ACTION(callocDebug)(_items,_elem_size)
#define newDebug(_type)                 FUNC_PANIC_ACTION(newDebug)(_type)
#define newZDebug(_type)                FUNC_PANIC_ACTION(newZDebug)(_type)
#define freeDebug(_ptr)                 FUNC_PANIC_ACTION(freeDebug)(_ptr)

/* Legacy macros. Kept for compatibility */

#define mallocPanic(x) mallocDebugPanic(x)
#define freePanic(x) freeDebugPanic(x)
#define PanicUnlessRealloc(ptr, size) reallocDebugPanic(ptr, size)

#endif /* _SINK_MALLOC_DEBUG_H_ */
