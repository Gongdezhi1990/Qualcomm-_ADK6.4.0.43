/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

DESCRIPTION
    Chain library bundle management implementation.
*/

#include "chain.h"
#include "file_list.h"

#include <panic.h>
#include <print.h>

static FILE_INDEX getFileIndex(const char *filename)
{
    FILE_INDEX file_index;

    file_index = FileFind(FILE_ROOT, filename, (uint16) strlen(filename));
    if (file_index == FILE_NONE)
    {
        PRINT(("Chain: Panic: Downloadable capabilities file not found = %s\n", filename));
        Panic();
    }

    return file_index;
}

static void addBundleInfoInFileListLib(const capability_bundle_t bundle)
{
    file_related_data_t related_data;

    related_data.data = &(bundle.processors);
    related_data.size_of_data = sizeof(bundle.processors);

    if (!FileListAddFile(downloadable_capabilities_file_role, getFileIndex(bundle.file_name), &related_data))
    {
        PRINT(("Chain: Panic: Failed to add file in the file_list library, file = %s\n", bundle.file_name));
        Panic();
    }
}

void ChainSetDownloadableCapabilityBundleConfig(const capability_bundle_config_t *config)
{
    FileListRemoveFiles(downloadable_capabilities_file_role);

    if ((config != NULL) && (config->number_of_capability_bundles > 0))
    {
        unsigned i, number_of_bundles;

        number_of_bundles = config->number_of_capability_bundles;
        for(i = 0; i < number_of_bundles; i++)
            addBundleInfoInFileListLib(config->capability_bundles[i]);
    }
}
