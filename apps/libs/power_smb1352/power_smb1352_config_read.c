/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    power_smb1352_config_read.c

DESCRIPTION
    Read SMB1352 specific config data.
*/

#include <string.h>

#include <file.h>
#include <panic.h>
#include <source.h>
#include <stream.h>

#include "power_smb1352_config_read.h"

#define SMB1352_CONFIG_FILENAME    "quick_charge_config/chargerConfig.hex"

static FILE_INDEX getConfigFileIndex(void)
{
    FILE_INDEX smb1352_config_file_index;

    smb1352_config_file_index = FileFind(FILE_ROOT, SMB1352_CONFIG_FILENAME, (uint16)strlen(SMB1352_CONFIG_FILENAME));

    PanicFalse(smb1352_config_file_index);

    return smb1352_config_file_index;
}

static uint8 readConfigFileInBlocksOfUint8(const uint8 *data, uint16 len, uint16 *index)
{
    uint8 byte;
    uint16 i = *index;

    if (!data)
        Panic();

    PanicFalse(len > *index);
    byte = data[i++];
    *index = i;
    return byte;
}

void powerSmb1352ConfigReadPopulateData(power_smb_1352_config_t * config_data)
{
    FILE_INDEX smb1352_config_file_index;
    Source smb1352_config_file_source;
    uint16 length, index, reg_index;
    const uint8 *data;

    PanicFalse(config_data);

    smb1352_config_file_index = getConfigFileIndex();

    smb1352_config_file_source = StreamFileSource(smb1352_config_file_index);

    PanicFalse(smb1352_config_file_source);

    length = SourceSize(smb1352_config_file_source);
    data = SourceMap(smb1352_config_file_source);

    index = 0;

    for(reg_index = 0; reg_index < SMB1352_NUMBER_OF_CONFIGURATION_REGISTERS; reg_index++)
        config_data->configuration_registers[reg_index] = readConfigFileInBlocksOfUint8(data, length, &index);

    SourceClose(smb1352_config_file_source);
}
