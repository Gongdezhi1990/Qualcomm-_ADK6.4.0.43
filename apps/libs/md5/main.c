/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */
/* Test harness for MD5 */

#include <stdio.h>
#include <string.h>
#include <vm.h>

#include "md5.h"

/* Length of test block, number of test blocks.  */
#define TEST_BLOCK_LEN   1000
#define TEST_BLOCK_COUNT 1000

static void puthex(unsigned int nibble)
{ putchar("0123456789abcdef"[nibble&15]); }

static void print_digest(unsigned char digest[16])
{
    unsigned int i;
    for(i = 0; i < 16; i++)
    {
        puthex(digest[i]>>4);
        puthex(digest[i]);
    }
}

/* Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte
   blocks.
*/
static void time_trial(void)
{
    MD5_CTX context;
    uint32 endTime, startTime;
    unsigned char block[TEST_BLOCK_LEN], digest[16];
    unsigned int i;


    printf
        ("MD5 time trial. Digesting %d %d-byte blocks ...",
         TEST_BLOCK_LEN, TEST_BLOCK_COUNT);

    /* Initialize block */
    for(i = 0; i < TEST_BLOCK_LEN; i++)
        block[i] =(unsigned char)(i & 0xff);

    /* Start timer */
    startTime = VmGetClock();

    /* Digest blocks */
    MD5Init(&context);
    for(i = 0; i < TEST_BLOCK_COUNT; i++)
        MD5Update(&context, block, TEST_BLOCK_LEN);
    MD5Final(digest, &context);

    /* Stop timer */
    endTime = VmGetClock();

    printf(" done\n");
    printf("Digest = ");
    print_digest(digest);
    printf("\nTime = %ld ms\n",(long)(endTime-startTime));
    printf
        ("Speed = %ld bytes/second\n",
        (long)TEST_BLOCK_LEN *(long)TEST_BLOCK_COUNT *(long) 1000/(endTime-startTime));
}

/* Digests a reference suite of strings and prints the results.
 */

static void verify(const char *input, const unsigned char *expected)
{
    MD5_CTX context;
    unsigned char digest[16];
    unsigned int len = strlen(input);

    MD5Init(&context);
    MD5Update(&context,(unsigned char *) input, len);
    MD5Final(digest, &context);

    printf("MD5(\"%s\") = ", input);
    print_digest(digest);
    if(memcmp(digest, expected, sizeof(digest)) != 0)
        printf(" FAIL!");
    printf("\n");
}

const static struct
{
    const char *input;
    unsigned char output[16];
} test_cases[] =
{
    { "", { 0xd4, 0x1d, 0x8c, 0xd9, 0x8f, 0x00, 0xb2, 0x04, 0xe9, 0x80, 0x09, 0x98, 0xec, 0xf8, 0x42, 0x7e }},
    { "a", { 0x0c, 0xc1, 0x75, 0xb9, 0xc0, 0xf1, 0xb6, 0xa8, 0x31, 0xc3, 0x99, 0xe2, 0x69, 0x77, 0x26, 0x61 }},
    { "abc", { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 }},
    { "message digest", { 0xf9, 0x6b, 0x69, 0x7d, 0x7c, 0xb7, 0x93, 0x8d, 0x52, 0x5a, 0x2f, 0x31, 0xaa, 0xf1, 0x61, 0xd0 }},
    { "abcdefghijklmnopqrstuvwxyz", { 0xc3, 0xfc, 0xd3, 0xd7, 0x61, 0x92, 0xe4, 0x00, 0x7d, 0xfb, 0x49, 0x6c, 0xca, 0x67, 0xe1, 0x3b }},
    { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", { 0xd1, 0x74, 0xab, 0x98, 0xd2, 0x77, 0xd9, 0xf5, 0xa5, 0x61, 0x1c, 0x2c, 0x9f, 0x41, 0x9d, 0x9f }},
    { "12345678901234567890123456789012345678901234567890123456789012345678901234567890", { 0x57, 0xed, 0xf4, 0xa2, 0x2b, 0xe3, 0xc9, 0x55, 0xac, 0x49, 0xda, 0x2e, 0x21, 0x07, 0xb6, 0x7a }}
};

static void test_suite(void)
{
    uint16 i;
    printf("MD5 test suite:\n");
    for(i = 0; i < sizeof(test_cases)/sizeof(*test_cases); ++i)
        verify(test_cases[i].input, test_cases[i].output);
}

int main(void)
{
    test_suite();
    time_trial();
    return 0;
}
