/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_downloadable_caps.c

DESCRIPTION
    Initialises the downloadable capabilities configuration data of the file_list and audio_config libraries.

*/

#include <ctype.h>
#include <file.h>
#include <stdlib.h>

#ifdef ENABLE_DOWNLOADABLE_CAPS

#include "sink_led_err.h"
#include "sink_debug.h"
#include "sink_configmanager.h"
#include "file_list.h"
#include "byte_utils.h"
#include "sink_leds.h"
#include "chain.h"
#include "sink_downloadable_caps.h"
#include "sink_downloadable_capabilities_config_def.h"
#include "sink_configure_dsp_capability_ids_config_def.h"
#include "operators.h"

#ifdef DEBUG_DOWNLOADABLE_CAPS
#define DOWNLOADABLE_CAPS_DEBUG(x) DEBUG(x)
#else
#define DOWNLOADABLE_CAPS_DEBUG(x)
#endif

#define PROCESSOR_MAP(x)  ((x == available_to_processor_P0) ? capability_bundle_available_p0 : capability_bundle_available_p0_and_p1)

typedef struct
{
    const uint16* packed_filename;
    capability_bundle_processor_availability_t processor;
} config_bundle_info_t;

static void displayBundleDetailsHelper(const char *filename, FILE_INDEX file_index, capability_bundle_processor_availability_t processor)
{
#ifdef DEBUG_DOWNLOADABLE_CAPS
    const char format_string[] = "DOWNLOADABLE_CAPS: Bundle details: filename = %s, file_index = %u, processor = %s\n";
    char processor_printable[] = "Error: Invalid option for processor";

    switch(processor)
    {
        case capability_bundle_available_p0:
            strcpy(processor_printable, "P0");
            break;
        case capability_bundle_available_p0_and_p1:
            strcpy(processor_printable, "P0 and P1");
            break;
        default:
            break;
    }

    DOWNLOADABLE_CAPS_DEBUG((format_string, filename, file_index, processor_printable));
#else
    UNUSED(filename);
    UNUSED(file_index);
    UNUSED(processor);
#endif
}

static char * getUnpackedFilenameCopy(const uint16 * packed_filename, uint16 max_config_size)
{
    uint8 * filename;
    uint16 filename_length;

    filename_length = ByteUtilsGetPackedStringLen(packed_filename, max_config_size);
    filename = malloc((filename_length + 1U) * sizeof(uint8));

    if(filename == NULL)
    {
        DOWNLOADABLE_CAPS_DEBUG(("DOWNLOADABLE_CAPS: Memory allocation failure in getUnpackedFilenameCopy()\n"));
        LedsIndicateError(led_err_id_enum_downloadable_caps);
    }
    else
    {
        ByteUtilsMemCpyUnpackString(filename, packed_filename, filename_length);
        filename[filename_length]='\0';
    }

    return (char *) filename;
}

static bool isNotEmptyString(const uint16 *packed_string, uint16 max_config_size)
{
    bool not_empty = FALSE;
    char *string = getUnpackedFilenameCopy(packed_string, max_config_size);

    if (string != NULL)
        not_empty = (strcmp(string, "") != 0);

    free(string);

    return not_empty;
}

static void addBundleFileEntryToFileListLib(FILE_INDEX file_index, capability_bundle_processor_availability_t processor)
{
    file_related_data_t  file_meta_data;

    file_meta_data.data = &processor;
    file_meta_data.size_of_data = sizeof(processor);

    if(FileListAddFile(downloadable_capabilities_file_role, file_index, &file_meta_data) == FALSE)
    {
        DOWNLOADABLE_CAPS_DEBUG(("DOWNLOADABLE_CAPS: ERROR :FileListAddFile() failure\n"));
        LedsIndicateError(led_err_id_enum_downloadable_caps);
    }
}

static void processBundleFileEntry(const uint16 * packed_filename, capability_bundle_processor_availability_t processor, uint16 max_config_size)
{
    char *filename = getUnpackedFilenameCopy(packed_filename, max_config_size);

    if(filename != NULL)
    {
        FILE_INDEX file_index = FileFind(FILE_ROOT, filename, (uint16) strlen(filename));

        if(file_index == FILE_NONE)
        {
            DOWNLOADABLE_CAPS_DEBUG(("DOWNLOADABLE_CAPS: ERROR :File not found :%s\n", filename));
            LedsIndicateError(led_err_id_enum_downloadable_caps);
        }
        else
        {
            displayBundleDetailsHelper(filename, file_index, processor);
            addBundleFileEntryToFileListLib(file_index, processor);
        }
    }

    free(filename);
}

static void processAllBundleFileEntries(const downloadable_dsp_caps_config_def_t *config, uint16 max_config_size)
{
    unsigned i;
    const cap_bundle_table_entry_t *entry;

    for(i = 0; i < ARRAY_DIM(config->bundle_file_entry); i++)
    {
        entry = &(config->bundle_file_entry[i]);
        if (isNotEmptyString((const uint16 *) entry->bundle_filename, max_config_size))
            processBundleFileEntry((const uint16 *) entry->bundle_filename, PROCESSOR_MAP(entry->available_on_processor), max_config_size);
    }
}

static void displayCapabilityIds(capability_id_t original_cap_id, capability_id_t new_cap_id)
{
#ifdef DEBUG_DOWNLOADABLE_CAPS
    const char format_string[] = "DOWNLOADABLE_CAPS: Original DSP Cap ID: 0x%X - New DSP Cap ID = 0x%X\n";

    DOWNLOADABLE_CAPS_DEBUG((format_string, original_cap_id, new_cap_id));
#else
    UNUSED(original_cap_id);
    UNUSED(new_cap_id);
#endif
}

static void replaceCapabilityId(capability_id_t original_cap_id, capability_id_t new_cap_id)
{
    displayCapabilityIds(original_cap_id, new_cap_id);
    if (CustomOperatorReplaceCapabilityId(original_cap_id, new_cap_id) == FALSE)
    {
        DOWNLOADABLE_CAPS_DEBUG(("DOWNLOADABLE_CAPS: ERROR: CustomOperatorReplaceCapabilityId() failure\n"));
        LedsIndicateError(led_err_id_enum_downloadable_caps);
    }
}

static void replaceCapabilityIdsWithTheOnesSpecifiedInTheTable(const dsp_capability_ids_table_config_def_t *dsp_cap_ids_table, uint16 config_size)
{
    const dsp_capability_ids_entry_t *caps_entry;
    unsigned number_of_entries = ((config_size * sizeof(uint16)) / sizeof(dsp_cap_ids_table->new_cap_id_entry[0]));

    for(caps_entry = dsp_cap_ids_table->new_cap_id_entry; caps_entry < (dsp_cap_ids_table->new_cap_id_entry + number_of_entries); caps_entry++)
        replaceCapabilityId(caps_entry->original_cap_id, caps_entry->new_cap_id);
}

static void processDownloadableCapsConfigBlock(void)
{
    uint16 config_size;
    const downloadable_dsp_caps_config_def_t *config;

    config_size = configManagerGetReadOnlyConfig(DOWNLOADABLE_DSP_CAPS_CONFIG_BLK_ID, (const void **) &config);

    if(config_size)
        processAllBundleFileEntries(config, config_size);

    configManagerReleaseConfig(DOWNLOADABLE_DSP_CAPS_CONFIG_BLK_ID);
}

static void processDspCapabilityIdsTableConfigBlock(void)
{
    uint16 config_size;
    const dsp_capability_ids_table_config_def_t *dsp_cap_ids_table;

    config_size = configManagerGetReadOnlyConfig(DSP_CAPABILITY_IDS_TABLE_CONFIG_BLK_ID, (const void **) &dsp_cap_ids_table);

    if(config_size)
        replaceCapabilityIdsWithTheOnesSpecifiedInTheTable(dsp_cap_ids_table, config_size);

    configManagerReleaseConfig(DSP_CAPABILITY_IDS_TABLE_CONFIG_BLK_ID);
}

void sinkDownloadableCapsInit(void)
{
    processDownloadableCapsConfigBlock();
    processDspCapabilityIdsTableConfigBlock();
}

#endif /*ENABLE_DOWNLOADABLE_CAPS*/
