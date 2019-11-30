/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      Header file for logging macro's

    Two styles of logging are supported.

    Both styles can be accessed from the pydbg tool using the 
    log commands. The two forms of logging will be displayed
    in the order they were logged. Although the two styles of logging 
    can be combined, the recommendation is to use the DEBUG_LOG()
    functions.

    All of these macros are enabled by default, but logging can be disabled 
    by use of the define DISABLE_LOG. Defining or undefining this 
    at the top of a particular source file would allow logging on a 
    module basis.

    \note The DEBUG_LOG() macro write condensed information 
    to a logging area and <b>can only be decoded if the original 
    application image file (.elf) is available</b>.

    This macro is quicker and by virtue of being condensed there
    is less chance of losing information if monitoring information
    in real time.

    The DEBUG_PRINT() macros write the complete string to a different 
    logging area, a character buffer, and can be decoded even if the 
    application image file is not available.

    The use of DEBUG_PRINT() is not recommended apart from, for instance,
    printing the contents of a message or buffer. Due to memory
    constraints the available printf buffer is relatively small
    and information can be lost. When paramaters are used the DEBUG_PRINT()
    functions can also use a lot of the software stack which can cause 
    a Panic() due to a stack overflow.
*/


#ifndef LOGGING_H
#define LOGGING_H

#include <hydra_log.h>

#ifndef LOGGING_EXCLUDE_SEQUENCE
#define EXTRA_LOGGING_STRING        "%04X: "
#define EXTRA_LOGGING_NUM_PARAMS    1
#define EXTRA_LOGGING_PARAMS        , globalDebugLineCount++ 
#else
#define EXTRA_LOGGING_STRING
#define EXTRA_LOGGING_NUM_PARAMS
#define EXTRA_LOGGING_PARAMS
#endif /*  LOGGING_EXCLUDE_SEQUENCE */


/*! \cond internals 

    This is some cunning macro magic that is able to count the
    number of arguments supplied to DEBUG_LOG

    If the arguments were "Hello", 123, 456 then VA_NARGS returns 2

                Hello 123 456 16  15  14  13  12  11  10   9   8   7   6   5   4   3  2  1 0 _bonus
    VA_NARGS_IMPL(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l, _m, _n, _o, _p, _q, N, ...) N


 */
#define VA_NARGS_IMPL(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l, _m, _n, _o, _p, _q, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, _bonus_as_no_ellipsis)

    /* Equivalent macro that just indicates if there are SOME arguments 
       after the format string, or NONE 

       if the argument is just "hello" then VA_ANY_ARGS_IMPL returns _NONE, which is 
       used to form a macro REST_OF_ARGS_NONE

                      hello _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _NONE, _bonus
       VA_ANY_ARGS_IMPL(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l, _m, _n, _o, _p, _q, SN, ...) SN

       */

#define VA_ANY_ARGS_IMPL(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l, _m, _n, _o, _p, _q, SN, ...) SN
#define VA_ANY_ARGS(...) VA_ANY_ARGS_IMPL(__VA_ARGS__, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _SOME, _NONE, _bonus_as_no_ellipsis)

    /* Retrieve just the format string */
#define FMT(fmt,...) fmt

    /* macros that return additional parameters past the format string 
        The _NONE variant is needed as (fmt, ...) will not match if just passed "hello"
     */
#define REST_OF_ARGS_NONE(fmt) 
#define REST_OF_ARGS_SOME(fmt,...) ,__VA_ARGS__
#define _MAKE_REST_OF_ARGS(a,b) a##b
#define MAKE_REST_OF_ARGS(_num) _MAKE_REST_OF_ARGS(REST_OF_ARGS,_num)


extern void hydra_log_firm_variadic(const char *event_key, size_t n_args, ...);
/*! \endcond internals */

extern uint16 globalDebugLineCount;

#ifndef DISABLE_LOG

/*! \brief  Display the supplied string in the condensed log

    \param fmt  String to display in the log
 */
#define DEBUG_LOG(...) \
            do { \
                HYDRA_LOG_STRING(log_fmt, EXTRA_LOGGING_STRING FMT(__VA_ARGS__,bonus_arg)); \
                hydra_log_firm_variadic(log_fmt, VA_NARGS(__VA_ARGS__) + EXTRA_LOGGING_NUM_PARAMS EXTRA_LOGGING_PARAMS MAKE_REST_OF_ARGS(VA_ANY_ARGS(__VA_ARGS__))(__VA_ARGS__)); \
            } while (0)



/*! \brief  Include a string, without parameters in the 
        character debug buffer

    See the description in the file as to why the use of this
    function is not recommended.

    \param fmt  String to display in the log
 */
#define DEBUG_PRINT(...) \
        printf(__VA_ARGS__)

#else   /* DISABLE_LOG */

#define UNUSED0(a)                      (void)(a)
#define UNUSED1(a,b)                    (void)(a),UNUSED0(b)
#define UNUSED2(a,b,c)                  (void)(a),UNUSED1(b,c)
#define UNUSED3(a,b,c,d)                (void)(a),UNUSED2(b,c,d)
#define UNUSED4(a,b,c,d,e)              (void)(a),UNUSED3(b,c,d,e)
#define UNUSED5(a,b,c,d,e,f)            (void)(a),UNUSED4(b,c,d,e,f)
#define UNUSED6(a,b,c,d,e,f,g)          (void)(a),UNUSED5(b,c,d,e,f,g)
#define UNUSED7(a,b,c,d,e,f,g,h)        (void)(a),UNUSED6(b,c,d,e,f,g,h)
#define UNUSED8(a,b,c,d,e,f,g,h,i)      (void)(a),UNUSED7(b,c,d,e,f,g,h,i)
#define UNUSED9(a,b,c,d,e,f,g,h,i,j)    (void)(a),UNUSED8(b,c,d,e,f,g,h,i,j)

#define ALL_UNUSED_IMPL_(nargs) UNUSED ## nargs
#define ALL_UNUSED_IMPL(nargs) ALL_UNUSED_IMPL_(nargs)

#define DEBUG_LOG(...) ALL_UNUSED_IMPL(VA_NARGS(__VA_ARGS__)) (__VA_ARGS__ )
#define DEBUG_PRINT(...) ALL_UNUSED_IMPL(VA_NARGS(__VA_ARGS__)) (__VA_ARGS__ )

#endif  /* DISABLE_LOG */

/*! Synonym for DEBUG_LOG.

    \deprecated 

    Originally different debug calls were required depending upon the number of parameters
    
 */
#define DEBUG_LOGF      DEBUG_LOG

/*! Synonym for DEBUG_PRINT.

    \deprecated 

    Originally different debug calls were required depending upon the number of parameters
 */
#define DEBUG_PRINTF    DEBUG_PRINT

#endif /* LOGGING_H */
