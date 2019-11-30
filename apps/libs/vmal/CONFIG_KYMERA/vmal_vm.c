/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    vmal_vm.c

DESCRIPTION
    Create shim version of VM traps
*/

#include <vmal.h>
#include <vm.h>

uint32 VmalVmReadProductId(void)
{
    return 0;
}

bool VmalDspInPatchableRom(void)
{
    return FALSE;
}
