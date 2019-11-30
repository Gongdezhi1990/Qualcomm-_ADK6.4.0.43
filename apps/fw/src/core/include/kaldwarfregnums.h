/*
  kaldwarfregnums.h

Copyright (c) 2008 - 2016 Qualcomm Technologies International, Ltd.
  
*/


#define KalDwarfRegisterNum_VERSION "0.1"
 

/*Dwarf2 register numbers upto 127 encode in a single byte using uleb128 format*/ 

/*general purpose registers*/
#define KalDwarfRegisterNum_Reserved_0 0
#define KalDwarfRegisterNum_RegrMAC 1
#define KalDwarfRegisterNum_RegR0 2
#define KalDwarfRegisterNum_RegR1 3
#define KalDwarfRegisterNum_RegR2 4
#define KalDwarfRegisterNum_RegR3 5
#define KalDwarfRegisterNum_RegR4 6
#define KalDwarfRegisterNum_RegR5 7
#define KalDwarfRegisterNum_RegR6 8
#define KalDwarfRegisterNum_RegR7 9
#define KalDwarfRegisterNum_RegR8 10
#define KalDwarfRegisterNum_RegR9 11
#define KalDwarfRegisterNum_RegR10 12

/* call frame registers*/
#define KalDwarfRegisterNum_RegrLINK 13
#define KalDwarfRegisterNum_RegFP 14
#define KalDwarfRegisterNum_RegSP 15
#define KalDwarfRegisterNum_RegFlags 16
#define KalDwarfRegisterNum_RegrINTLINK 17

/*future*/
#define KalDwarfRegisterNum_Reserved_18 18
#define KalDwarfRegisterNum_Reserved_19 19


/*composite double registers*/
#define KalDwarfRegisterNum_RegD0 20
#define KalDwarfRegisterNum_RegD1 21
#define KalDwarfRegisterNum_RegD2 22
#define KalDwarfRegisterNum_RegD3 23
#define KalDwarfRegisterNum_RegD4 24
#define KalDwarfRegisterNum_RegD5 25

/*future*/
#define KalDwarfRegisterNum_Reserved_26 26
#define KalDwarfRegisterNum_Reserved_27 27
#define KalDwarfRegisterNum_Reserved_28 28
#define KalDwarfRegisterNum_Reserved_29 29


/* composite triple registers */
#define KalDwarfRegisterNum_RegT0 30
#define KalDwarfRegisterNum_RegT1 31
#define KalDwarfRegisterNum_RegT2 32
#define KalDwarfRegisterNum_RegT3 33

/*future*/
#define KalDwarfRegisterNum_Reserved_34 34
#define KalDwarfRegisterNum_Reserved_35 35
#define KalDwarfRegisterNum_Reserved_36 36
#define KalDwarfRegisterNum_Reserved_37 37
#define KalDwarfRegisterNum_Reserved_38 38
#define KalDwarfRegisterNum_Reserved_39 39

/* index registers */
#define KalDwarfRegisterNum_RegI0 40
#define KalDwarfRegisterNum_RegI1 41
#define KalDwarfRegisterNum_RegI2 42
#define KalDwarfRegisterNum_RegI3 43
#define KalDwarfRegisterNum_RegI4 44
#define KalDwarfRegisterNum_RegI5 45
#define KalDwarfRegisterNum_RegI6 46
#define KalDwarfRegisterNum_RegI7 47

/*future*/
#define KalDwarfRegisterNum_Reserved_48 48
#define KalDwarfRegisterNum_Reserved_49 49

/* dummy mappings to satisfy the compiler */
#define KalDwarfRegisterNum_RegNull    50
#define KalDwarfRegisterNum_RegrMAC0   51
#define KalDwarfRegisterNum_RegrMAC1   52
#define KalDwarfRegisterNum_RegrMAC2   53
#define KalDwarfRegisterNum_RegrMAC12  54
#define KalDwarfRegisterNum_RegrMACB0  55
#define KalDwarfRegisterNum_RegrMACB1  56
#define KalDwarfRegisterNum_RegrMACB2  57
#define KalDwarfRegisterNum_RegrMACB12 58

/* reserved for future use */

#define KalDwarfRegisterNum_Reserved_small_low  59
#define KalDwarfRegisterNum_Reserved_small_high 127


/* Dwarf2 register numbers above 128 encode in 2 bytes using uleb128 format*/ 
#define KalDwarfRegisterNum_Reserved_normal_low 128
#define KalDwarfRegisterNum_Reserved_normal_high 16383




/*end kaldwarfregnums.h */


