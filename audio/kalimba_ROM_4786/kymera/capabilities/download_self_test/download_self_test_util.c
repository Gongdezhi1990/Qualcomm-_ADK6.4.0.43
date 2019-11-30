/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  download_self_test_util.c
 * \ingroup  capabilities
 *
 *  Basic download self test capability
 *
 */
/****************************************************************************
Include Files
*/
#include "download_self_test_private.h"
#include "const_data/const_data.h"

/*****************************************************************************
Private Function Definitions
*/
typedef unsigned int(*test_func_ptr)(void);

#ifdef CONST_TEST
static unsigned int func_returns_good(void);
static unsigned int func_returns_bad(void);
#endif

/*****************************************************************************
Private Variable Declarations
*/
/* Duplicated name in download_self_test.c but static to this module, build system must be able
 * to deal with this */
static unsigned int my_static_array[] = {0x600D, 1, 2};
/* Helper variable for recursion test */
static int a = 1;
#ifdef CONST_TEST
/* CONST tables with literals */
CONST16 uint16 const_table_16[]   =   {0x1111U,   0x2222U,   0x3333U,   0x4444U,   0x5555U,   0x6666U, 0x7777U};
CONST   uint24 const_table_24[]   =   {0xAAAAAAU, 0xBBBBBBU, 0xCCCCCCU, 0xDDDDDDU, 0xEEEEEEU, 0xFFFFFFU};
/* 0xD9D9 = 0xBEEF + 0xB00 + 0xFEA */
CONST16 uint16 const_table_16_2[] =   {0xBEEFU   ,0xB00U    ,0xFEAU    ,0xD9D9U};
CONST static uint24 const_table_24_2[] =   {0xDADB0DU ,0x0B00B5U ,0x0B00DAU      };
DMCONST uint24 const_table_dm[]   =   {0xAADEADU ,0xDEADAAU};

/* CONST tables with function pointers */
CONST   test_func_ptr const_table_func_ptr[]     =   {func_returns_bad ,func_returns_bad ,func_returns_good };
DMCONST test_func_ptr const_table_func_ptr_dm[]  =   {func_returns_bad ,func_returns_good};

/* Descriptors used to access the tables defined above */
const_data_descriptor const_table_16_dsc   = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,  FORMAT_16BIT_ZERO_PAD, const_table_16);
const_data_descriptor const_table_16_dsc_2 = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,  FORMAT_16BIT_ZERO_PAD, const_table_16_2);
const_data_descriptor const_table_24_dsc   = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST, FORMAT_DSP_NATIVE, const_table_24);
const_data_descriptor const_table_24_dsc_2 = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST, FORMAT_DSP_NATIVE, const_table_24_2);
const_data_descriptor const_table_dm_dsc   = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_DMCONST, FORMAT_DSP_NATIVE, const_table_dm);

const_data_descriptor const_table_func_ptr_dm_dsc    = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_DMCONST, FORMAT_DSP_NATIVE, const_table_func_ptr_dm);
const_data_descriptor const_table_func_ptr_const_dsc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST, FORMAT_DSP_NATIVE, const_table_func_ptr);

/* CONST tables with pointers to other tables using descriptors */
CONST   const_data_descriptor* const_table_desc_ptr[] =   {&const_table_24_dsc   ,&const_table_16_dsc   ,&const_table_dm_dsc };
DMCONST const_data_descriptor* const_table_desc_dm[]  =   {&const_table_16_dsc_2 ,&const_table_24_dsc_2                      };

const_data_descriptor const_table_desc_ptr_const_dsc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST, FORMAT_DSP_NATIVE, const_table_desc_ptr);
const_data_descriptor const_table_desc_ptr_dm_dsc    = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_DMCONST, FORMAT_DSP_NATIVE, const_table_desc_dm);

/*****************************************************************************
Private Function Declarations
*/
/* In a downloadable capability we cannot have two functions with the same name
 * even if they are static. Build system would complain if this occurs. */

static unsigned int func_returns_good(void)
{
    return 0x600D;
}

static unsigned int func_returns_bad(void)
{
    return 0xBAD;
}
#endif

static unsigned int download_self_test_opmsg_start_self_test2(void)
{
    /* This will create an anonymous variable */
    unsigned int* my_local_array[] = {&my_static_array[0], &my_static_array[1], &my_static_array[2]};
    int i = 0;
    /* This loop forces the creation of anonymous initialisation tables L_lcx */
    for (i = 0; i < ARRAY_SIZE(my_static_array); i++)
    {
        my_static_array[i] = *(my_local_array[i]);
    }
    return my_static_array[0];
}

static void func_decrease(unsigned int num)
{
    if (num > 0)
    {
        /* Exercise recursion */
        func_decrease(num - 1);
        a--;
    }
}

static void func_increase_one(void)
{
    a++;
}

#ifdef CONST_TEST

static unsigned int test_dyn_descriptor(void* dsc)
{
    uint16 *wbuf16;
    const_data_descriptor const_table_16_dsc_dyn = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,  FORMAT_16BIT_ZERO_PAD, dsc);
    wbuf16 = (uint16 *) const_data_access ( &const_table_16_dsc_dyn, 1*sizeof(uint16), NULL, 6*sizeof(uint16) );
    if ((wbuf16[3] != 0x5555U))
    {
        return 0xBAD;
    }
    return 0x600D;
}
/***********************************************************
 * download_self_test_const_data_test
 *
 * Test const_data API for a download capability
 *
 * */
static unsigned int download_self_test_const_data_test(void)
{
    uint16 *wbuf16;
    unsigned *wbuf;
    /* Test external const memory access in download */
    wbuf16 = (uint16 *) const_data_access ( &const_table_16_dsc, 1*sizeof(uint16), NULL, 6*sizeof(uint16) );
    wbuf = (unsigned *) const_data_access ( &const_table_24_dsc, 1*sizeof(unsigned), NULL, 5*sizeof(unsigned) );
    if ((wbuf16[5] != 0x7777U) || (wbuf[1] != 0xCCCCCCU))
    {
        return 0xBAD;
    }
    wbuf = const_data_access ( &const_table_24_dsc_2, 1*sizeof(unsigned), wbuf, 1*sizeof(unsigned) );
    if (wbuf[0] != 0x0B00B5U)
    {
        return 0xBAD;
    }
    const_data_release(wbuf);
    /* Access a DMCONST variable which in download gets placed in DM */
    wbuf = const_data_access ( &const_table_dm_dsc, 1*sizeof(unsigned), NULL, 1*sizeof(unsigned) );
    if (wbuf[0] != 0xDEADAAU)
    {
        return 0xBAD;
    }
    wbuf16 = const_data_access ( &const_table_16_dsc_2, 0*sizeof(uint16), wbuf16, 4*sizeof(uint16));
    /* Sum of three fields must be equal to last one */
    if (wbuf16[0] + wbuf16[1] + wbuf16[2] != wbuf16[3] )
    {
        return 0xBAD;
    }
    const_data_release(wbuf16);
    const_data_release(wbuf);

    test_func_ptr* func_ptr_buf;
    /* Test const data access with references into downloaded capability DM and PM */
    /* Using DMCONST */
    func_ptr_buf = (test_func_ptr *) const_data_access ( &const_table_func_ptr_dm_dsc, 0*sizeof(test_func_ptr), NULL, 2*sizeof(test_func_ptr));
    if (func_ptr_buf[1]() != 0x600D)
    {
        return 0xBAD;
    }
    const_data_release(func_ptr_buf);
    /* Repeat previous test with an offset */
    func_ptr_buf = (test_func_ptr *) const_data_access ( &const_table_func_ptr_dm_dsc, 1*sizeof(test_func_ptr), NULL, 1*sizeof(test_func_ptr) );
    if (func_ptr_buf[0]() != 0x600D)
    {
        return 0xBAD;
    }
    const_data_release(func_ptr_buf);
    /* Using CONST */
    func_ptr_buf = (test_func_ptr *) const_data_access ( &const_table_func_ptr_const_dsc, 0*sizeof(test_func_ptr), NULL, 3*sizeof(test_func_ptr) );
    if (func_ptr_buf[2]() != 0x600D)
    {
        return 0xBAD;
    }
    const_data_release(func_ptr_buf);
    /* Repeat previous test with an offset */
    func_ptr_buf = (test_func_ptr *) const_data_access ( &const_table_func_ptr_const_dsc, 1*sizeof(test_func_ptr), NULL, 2*sizeof(test_func_ptr) );
    if (func_ptr_buf[1]() != 0x600D)
    {
        return 0xBAD;
    }
    const_data_release(func_ptr_buf);

    /* Use a CONST table to access another CONST table */
    /* Using DMCONST*/
    const_data_descriptor **desc = (const_data_descriptor **)const_data_access ( &const_table_desc_ptr_dm_dsc, 1*sizeof(const_data_descriptor *), NULL, 1*sizeof(const_data_descriptor *) );
    /* desc[0] should be descriptor const_table_24_dsc_2 */
    wbuf = (unsigned*)const_data_access ( desc[0], 1*sizeof(unsigned), NULL, 2*sizeof(unsigned) );
    if (wbuf[1] != 0x0B00DAU)
    {
        return 0xBAD;
    }
    const_data_release(desc);
    const_data_release(wbuf);
    /* Using CONST*/
    desc = (const_data_descriptor **)const_data_access ( &const_table_desc_ptr_const_dsc, 1*sizeof(const_data_descriptor *), NULL, 2*sizeof(const_data_descriptor *) );
    /* desc[0] should be descriptor const_table_16_dsc */
    wbuf16 = (uint16*)const_data_access ( desc[0], 1*sizeof(uint16), NULL, 4*sizeof(uint16) );
    if (wbuf16[2] != 0x4444U)
    {
        return 0xBAD;
    }
    const_data_release(desc);
    const_data_release(wbuf16);
    /* Test "dynamic" descriptors to access CONST tables */
    const_data_descriptor const_table_16_dsc_stack = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,  FORMAT_16BIT_ZERO_PAD, const_table_16);
    wbuf16 = (uint16 *) const_data_access (&const_table_16_dsc_stack, 1*sizeof(uint16), NULL, 6*sizeof(uint16) );
    if (wbuf16[4] != 0x6666U)
    {
        return 0xBAD;
    }
    const_data_release(wbuf16);
    return test_dyn_descriptor((void*)const_table_16);
}
#endif
/*****************************************************************************
Public Function Declarations
*/

unsigned int func_increase_one_with_param_c(unsigned int a)
{
    return a + 1;
}

void func_decrease_one(void)
{
    func_increase_one();
    func_decrease(2);
}

void set_a(int n)
{
    a = n;
}

unsigned int download_self_test_util_helper(void)
{
    void (*func_ptr)(void);
    /* Force a reference to a local C function */
    func_ptr = &func_increase_one;
    func_ptr();
    /* Force a reference to a global C function */
    func_ptr = &func_decrease_one;
    func_ptr();
    func_ptr();
    if (a == 0)
    {
        if (download_self_test_opmsg_start_self_test2() != 0x600D)
        {
            return 0xBAD;
        }
        #ifdef CONST_TEST
        if (download_self_test_const_data_test() != 0x600D)
        {
            return 0xBAD;
        }
        #endif
        return 0x600D;
    }
    else
    {
        return 0xBAD;
    }
}
