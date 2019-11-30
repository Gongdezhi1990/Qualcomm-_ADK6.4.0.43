/****************************************************************************
Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_partition_data.c

DESCRIPTION
    Upgrade file processing state machine.
    It is parsing and validating headers.
    All received data are passed to MD5 validation.
    Partition data are written to SQIF.

NOTES

*/

#include <string.h>
#include <stdlib.h>

#include <byte_utils.h>
#include <panic.h>
#include <print.h>
#include <imageupgrade.h>
#include <rsa_decrypt.h>

#include "upgrade_partition_data.h"
#include "upgrade_partition_data_priv.h"
#include "upgrade_ctx.h"
#include "upgrade_fw_if.h"
#include "upgrade_psstore.h"
#include "upgrade_partitions.h"

#if defined (UPGRADE_RSA_2048)
/*
 * EXPECTED_SIGNATURE_SIZE is the size of an RSA-2048 signature.
 * 2048 bits / 8 bits per byte is 256 bytes.
 */
#define EXPECTED_SIGNATURE_SIZE 256
#elif defined (UPGRADE_RSA_1024)
/*
 * EXPECTED_SIGNATURE_SIZE is the size of an RSA-1024 signature.
 * 1024 bits / 8 bits per byte is 128 bytes.
 */
#define EXPECTED_SIGNATURE_SIZE 128
#else
#error "Neither UPGRADE_RSA_2048 nor UPGRADE_RSA_1024 defined."
#endif

#define FIRST_WORD_SIZE 4

/*
 * A module variable set by the UpgradePartitionDataHandleHeaderState function
 * to the value of the last byte in the data array. Retrieved by the
 * UpgradeFWIFValidateFinalize function using the
 * UpgradePartitionDataGetSigningMode function.
 */
static uint8 SigningMode = 1;
uint8 first_word_size = FIRST_WORD_SIZE;

uint8 UpgradePartitionDataGetSigningMode(void);

/****************************************************************************
NAME
    IsValidPartitionNum

DESCRIPTION
    Validates the partition number

RETURNS
    bool TRUE if the partition number is valid, else FALSE
*/
static bool IsValidPartitionNum(uint16 partNum)
{
    switch (partNum)
    {
    case IMAGE_SECTION_NONCE:
    case IMAGE_SECTION_APPS_P0_HEADER:
    case IMAGE_SECTION_APPS_P1_HEADER:
    case IMAGE_SECTION_AUDIO_HEADER:
    case IMAGE_SECTION_CURATOR_FILESYSTEM:
    case IMAGE_SECTION_APPS_P0_IMAGE:
    case IMAGE_SECTION_APPS_RO_CONFIG_FILESYSTEM:
    case IMAGE_SECTION_APPS_RO_FILESYSTEM:
    case IMAGE_SECTION_APPS_P1_IMAGE:
    case IMAGE_SECTION_APPS_DEVICE_RO_FILESYSTEM:
    case IMAGE_SECTION_AUDIO_IMAGE:
        return TRUE;

    case IMAGE_SECTION_APPS_RW_FILESYSTEM:
    case IMAGE_SECTION_APPS_RW_CONFIG:
    default:
        return FALSE;
    }
}

/****************************************************************************
NAME
    IsValidSqifNum

DESCRIPTION
    Validates the SQIF number for the partition

RETURNS
    bool TRUE if the SQIF and partition numbers are valid, else FALSE
*/
static bool IsValidSqifNum(uint16 sqifNum, uint16 partNum)
{
    UNUSED(partNum);
    /* TODO
     * Until audio is supported, only SQIF zero is valid.
     */
    return (sqifNum == 0);
}

/***************************************************************************
NAME
    PartitionOpen

DESCRIPTION
    Open a write only handle to a physical partition on the external flash.
    For initial testing, the CRC check on the partition is also disabled.

PARAMS
    physPartition Physical partition number in external flash.
    firstWord First word of partition data.

RETURNS
    UpgradeFWIFPartitionHdl A valid handle or NULL if the open failed.
*/
static UpgradeFWIFPartitionHdl PartitionOpen(uint16 physPartition, uint32 firstWord)
{
    Sink sink;
    /** TODO
     * When audio is supported, we can determine the QSPI to use from the partition.
     * Until then only QSPI zero is used.
     */
    uint16 QSPINum = 0;

    PRINT(("UPG: Opening partition %u for resume\n", physPartition));
    sink = ImageUpgradeStreamGetSink(QSPINum, physPartition, firstWord);
    PRINT(("ImageUpgradeStreamGetSink(%d, %d, 0x%08lx) returns %p\n", QSPINum, physPartition, firstWord, sink));
    if (!sink)
    {
        PRINT(("UPG: Failed to open raw partition %u for resume\n", physPartition));
        return (UpgradeFWIFPartitionHdl)NULL;
    }
    SinkConfigure(sink, VM_SINK_MESSAGES, VM_MESSAGES_NONE);

    UpgradeCtxGetFW()->partitionNum = physPartition;

    return (UpgradeFWIFPartitionHdl)sink;
}


/****************************************************************************
NAME
    UpgradePartitionDataInit  -  Initialise.

DESCRIPTION
    Initialises the partition data header handling.

RETURNS
    bool TRUE if OK, FALSE if not.
*/
bool UpgradePartitionDataInit(bool *waitForEraseComplete)
{
    UpgradePartitionDataCtx *ctx;
    *waitForEraseComplete = FALSE;

    ctx = UpgradeCtxGetPartitionData();
    if (!ctx)
    {
        ctx = malloc(sizeof(*ctx));
        if (!ctx)
        {
            PRINT(("\n"));
            return FALSE;
        }
        UpgradeCtxSetPartitionData(ctx);
    }
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = UPGRADE_PARTITION_DATA_STATE_GENERIC_1ST_PART;

    UpgradePartitionDataRequestData(HEADER_FIRST_PART_SIZE);

    UpgradeFWIFInit();
    UpgradeFWIFValidateInit();
    if ((UpgradeCtxGetPSKeys()->upgrade_in_progress_key == UPGRADE_RESUME_POINT_START) &&
        (UpgradeCtxGetPSKeys()->state_of_partitions == UPGRADE_PARTITIONS_UPGRADING) &&
        (UpgradeCtxGetPSKeys()->last_closed_partition > 0))
    {
        /* A partial update has been interrupted. Don't erase. */
        PRINT(("Partial update interrupted. Not erasing.\n"));
        return TRUE;
    }
    /* Ensure the other bank is erased before we start. */
    if (UPGRADE_PARTITIONS_ERASED == UpgradePartitionsEraseAllManaged())
    {
        *waitForEraseComplete = TRUE;
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    UpgradePartitionDataHandleHeaderState  -  Parser for the main header.

DESCRIPTION
    Validates content of the main header.

RETURNS
    Upgrade library error code.

NOTES
    Currently when main header size will grow beyond block size it won't work.
*/
UpgradeHostErrorCode UpgradePartitionDataHandleHeaderState(uint8 *data, uint16 len, bool reqComplete)
{
    UpgradePartitionDataCtx *ctx = UpgradeCtxGetPartitionData();
    UpgradeHostErrorCode rc = UPGRADE_HOST_SUCCESS;
    upgrade_version newVersion;
    upgrade_version currVersion;
    uint16 newPSVersion;
    uint16 currPSVersion;
    uint16 compatibleVersions;
    uint8 *ptr; /* Pointer into variable length portion of header */
    unsigned i;
    UNUSED(len);
    if(!reqComplete)
    {
        /* TODO: Handle a case when header is bigger than blockSize.
         * Currently such situation will cause this error.
         */
        return UPGRADE_HOST_ERROR_INTERNAL_ERROR_2;
    }

    if((strlen(UpgradeFWIFGetDeviceVariant()) > 0) &&
       (strncmp((char *)data, UpgradeFWIFGetDeviceVariant(), ID_FIELD_SIZE)))
    {
        return UPGRADE_HOST_ERROR_WRONG_VARIANT;
    }

    newVersion.major = ByteUtilsGet2BytesFromStream(&data[ID_FIELD_SIZE]);
    newVersion.minor = ByteUtilsGet2BytesFromStream(&data[ID_FIELD_SIZE+2]);
    compatibleVersions = ByteUtilsGet2BytesFromStream(&data[ID_FIELD_SIZE+4]);
    currVersion.major = UpgradeCtxGetPSKeys()->version.major;
    currVersion.minor = UpgradeCtxGetPSKeys()->version.minor;

    PRINT(("Current Version: %d.%d [%d]\n",currVersion.major,currVersion.minor,UpgradeCtxGetPSKeys()->config_version));
    PRINT(("Number compat %d\n",compatibleVersions));

    ptr = &data[ID_FIELD_SIZE+6];
    for (i = 0;i < compatibleVersions;i++)
    {
        upgrade_version version;
        version.major = ByteUtilsGet2BytesFromStream(ptr);
        version.minor = ByteUtilsGet2BytesFromStream(ptr+2);
        PRINT(("Test version: %d.%d\n",version.major,version.minor));
        if (    (version.major == currVersion.major)
            && (    (version.minor == currVersion.minor)
                 || (version.minor == 0xFFFFu)))
        {
            /* Compatible */
            break;
        }
        ptr += 4;
    }

    /* We failed to find a compatibility match */
    if (i == compatibleVersions)
    {
        return UPGRADE_HOST_WARN_APP_CONFIG_VERSION_INCOMPATIBLE;
    }

    ptr = &data[ID_FIELD_SIZE+6+4*compatibleVersions];
    currPSVersion = UpgradeCtxGetPSKeys()->config_version;
    newPSVersion = ByteUtilsGet2BytesFromStream(ptr);
    PRINT(("PS: Curr %d, New %d\n",currPSVersion,newPSVersion));
    if (currPSVersion != newPSVersion)
    {
        compatibleVersions = ByteUtilsGet2BytesFromStream(&ptr[2]);
        ptr+=4;
        PRINT(("Number of compatible PS %d\n",compatibleVersions));
        for (i = 0;i < compatibleVersions;i++)
        {
            uint16 version;
            version = ByteUtilsGet2BytesFromStream(ptr);
            PRINT(("Test PS compatibility %d\n",version));
            if (version == currPSVersion)
            {
                /* Compatible */
                break;
            }
            ptr += 2;
        }
        if (i == compatibleVersions)
        {
            return UPGRADE_HOST_WARN_APP_CONFIG_VERSION_INCOMPATIBLE;
        }
    }

    /* Store the in-progress upgrade version */
    UpgradeCtxGetPSKeys()->version_in_progress.major = newVersion.major;
    UpgradeCtxGetPSKeys()->version_in_progress.minor = newVersion.minor;
    UpgradeCtxGetPSKeys()->config_version_in_progress = newPSVersion;

    PRINT(("Saving versions %d.%d [%d]\n",newVersion.major,newVersion.minor,newPSVersion));

    /* At this point, partitions aren't actually dirty - but want to minimise PSKEYS
     * @todo: Need to check this variable before starting an upgrade
     */
    UpgradeCtxGetPSKeys()->state_of_partitions = UPGRADE_PARTITIONS_UPGRADING;

    /*!
        @todo Need to minimise the number of times that we write to the PS
              so this may not be the optimal place. It will do for now.
    */
    UpgradeSavePSKeys();

    UpgradePartitionDataRequestData(HEADER_FIRST_PART_SIZE);
    ctx->state = UPGRADE_PARTITION_DATA_STATE_GENERIC_1ST_PART;

    /* Set the signing mode to the value of the last byte in the data array */
    SigningMode = data[len-1];
    PRINT(("SigningMode %d\n", SigningMode));

    return rc;
}

/****************************************************************************
NAME
    UpgradePartitionDataHandleDataHeaderState  -  Parser for the partition data header.

DESCRIPTION
    Validates content of the partition data header.

RETURNS
    Upgrade library error code.
*/
UpgradeHostErrorCode UpgradePartitionDataHandleDataHeaderState(uint8 *data, uint16 len, bool reqComplete)
{
    UpgradePartitionDataCtx *ctx = UpgradeCtxGetPartitionData();
    uint16 partNum, sqifNum;
    uint32 firstWord = 0;
    UNUSED(reqComplete);

    if(len < PARTITION_SECOND_HEADER_SIZE + FIRST_WORD_SIZE)
    {
        return UPGRADE_HOST_ERROR_BAD_LENGTH_DATAHDR_RESUME;
    }

    sqifNum = ByteUtilsGet2BytesFromStream(data);
    PRINT(("PART_DATA: SQIF number %u\n", sqifNum));

    partNum = ByteUtilsGet2BytesFromStream(&data[2]);
    PRINT(("PART_DATA: partition number %u\n", partNum));

    if(!IsValidPartitionNum(partNum))
    {
        return UPGRADE_HOST_ERROR_WRONG_PARTITION_NUMBER;
    }

    if(!IsValidSqifNum(sqifNum, partNum))
    {
        return UPGRADE_HOST_ERROR_PARTITION_TYPE_NOT_MATCHING;
    }

     firstWord |= (uint32)data[PARTITION_SECOND_HEADER_SIZE+2]<<16;
     firstWord |= (uint32)data[PARTITION_SECOND_HEADER_SIZE + 3]<<24;
     firstWord |= (uint32)data[PARTITION_SECOND_HEADER_SIZE];
     firstWord |= (uint32)data[PARTITION_SECOND_HEADER_SIZE + 1]<<8;

    PRINT(("PART_DATA: first word is 0x%08lx\n", firstWord));
    

    if(!UpgradeFWIFValidateUpdate(NULL, partNum))
    {
        return UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_PARTITION_HEADER1;
    }

    if (UpgradeCtxGetPSKeys()->last_closed_partition > partNum)
    {
        PRINT(("PART_DATA: already handled partNum %u; skipping\n", partNum));
        ctx->nextOffset = ctx->partitionLength - FIRST_WORD_SIZE;
        UpgradePartitionDataRequestData(HEADER_FIRST_PART_SIZE);
        ctx->state = UPGRADE_PARTITION_DATA_STATE_GENERIC_1ST_PART;
        return UPGRADE_HOST_SUCCESS;
    }
    
    if(ctx->partitionLength > UpgradeFWIFGetPhysPartitionSize(partNum))
    {
        PRINT(("Size mismatch. ctx->partitionLength 0x%08lx\n", ctx->partitionLength));
        return UPGRADE_HOST_ERROR_PARTITION_SIZE_MISMATCH;
    }

    ctx->partitionHdl = PartitionOpen(partNum, firstWord);
    if(!ctx->partitionHdl)
    {
        return UPGRADE_HOST_ERROR_PARTITION_OPEN_FAILED;
    }

    ctx->nextOffset = UpgradeFWIFPartitionGetOffset(ctx->partitionHdl);

    PRINT(("PART_DATA: partition length is %ld and offset is: %ld\n", ctx->partitionLength, ctx->nextOffset));

    if((ctx->nextOffset + FIRST_WORD_SIZE) < ctx->partitionLength)
    {
        /* Get partition data from the offset, but skipping the first word. */
        UpgradePartitionDataRequestData(ctx->partitionLength - ctx->nextOffset - FIRST_WORD_SIZE);
        ctx->state = UPGRADE_PARTITION_DATA_STATE_DATA;
    }
    else if((ctx->nextOffset + FIRST_WORD_SIZE) == ctx->partitionLength)
    {
        /* A case when all data are in but partition is not yet closed */
        UpgradeHostErrorCode closeStatus = UpgradeFWIFPartitionClose(ctx->partitionHdl);
        if(UPGRADE_HOST_SUCCESS != closeStatus)
        {
            return closeStatus;
        }

        ctx->openNextPartition = TRUE;

        ctx->nextOffset -= FIRST_WORD_SIZE;
        UpgradePartitionDataRequestData(HEADER_FIRST_PART_SIZE);
        ctx->state = UPGRADE_PARTITION_DATA_STATE_GENERIC_1ST_PART;
    }
    else
    {
        /* It is considered bad when offset is bigger than partition size */

        return UPGRADE_HOST_ERROR_INTERNAL_ERROR_3;
    }

    PRINT(("PART_DATA: partition length is %ld and offset is: %ld bigreq is %ld, nextReq is %ld\n", ctx->partitionLength, ctx->nextOffset, ctx->bigReqSize, ctx->nextReqSize));

    return UPGRADE_HOST_SUCCESS;
}


/****************************************************************************
NAME
    UpgradePartitionDataHandleGeneric1stPartState  -  Parser for ID & length part of a header.

DESCRIPTION
    Parses common beginning of any header and determines which header it is.
    All headers have the same first two fields which are 'header id' and
    length.

RETURNS
    Upgrade library error code.
*/
UpgradeHostErrorCode UpgradePartitionDataHandleGeneric1stPartState(uint8 *data, uint16 len, bool reqComplete)
{
    UpgradePartitionDataCtx *ctx = UpgradeCtxGetPartitionData();
    uint32 length;
    UNUSED(reqComplete);

    if(len < HEADER_FIRST_PART_SIZE)
    {
        return UPGRADE_HOST_ERROR_BAD_LENGTH_TOO_SHORT;
    }

    length = ByteUtilsGet4BytesFromStream(&data[ID_FIELD_SIZE]);

    PRINT(("1st part header id %c%c%c%c%c%c%c%c len 0x%lx, 0x%x 0x%x 0x%x 0x%x\n", data[0], data[1], data[2], data[3],
            data[4], data[5], data[6], data[7], length, data[8], data[9], data[10], data[11]));

    if(0 == strncmp((char *)data, UpgradeFWIFGetHeaderID(), ID_FIELD_SIZE))
    {
        if(length < UPGRADE_HEADER_MIN_SECOND_PART_SIZE)
        {
            return UPGRADE_HOST_ERROR_BAD_LENGTH_UPGRADE_HEADER;
        }

        UpgradePartitionDataRequestData(length);
        ctx->state = UPGRADE_PARTITION_DATA_STATE_HEADER;
    }
    else if(0 == strncmp((char *)data, UpgradeFWIFGetPartitionID(), ID_FIELD_SIZE))
    {
        if(length < PARTITION_SECOND_HEADER_SIZE + FIRST_WORD_SIZE)
        {
            return UPGRADE_HOST_ERROR_BAD_LENGTH_PARTITION_HEADER;
        }
        UpgradePartitionDataRequestData(PARTITION_SECOND_HEADER_SIZE + FIRST_WORD_SIZE);
        ctx->state = UPGRADE_PARTITION_DATA_STATE_DATA_HEADER;
        ctx->partitionLength = length - PARTITION_SECOND_HEADER_SIZE;
    }
    else if(0 == strncmp((char *)data, UpgradeFWIFGetFooterID(), ID_FIELD_SIZE))
    {
        if(length != EXPECTED_SIGNATURE_SIZE)
        {
            /* The length of signature must match expected length.
             * Otherwise OEM signature checking could be omitted by just
             * setting length to 0 and not sending signature.
             */
            return UPGRADE_HOST_ERROR_BAD_LENGTH_SIGNATURE;
        }

        UpgradePartitionDataRequestData(length);

        ctx->signature = malloc(length);
        if (!ctx->signature)
        {
            return UPGRADE_HOST_ERROR_OEM_VALIDATION_FAILED_MEMORY;
        }

        ctx->state = UPGRADE_PARTITION_DATA_STATE_FOOTER;
    }
    else
    {
        return UPGRADE_HOST_ERROR_UNKNOWN_ID;
    }

    return UPGRADE_HOST_SUCCESS;
}

/****************************************************************************
NAME
    UpgradePartitionDataHandleDataState  -  Partition data handling.

DESCRIPTION
    Writes data to a SQIF and sends it the MD5 validation.

RETURNS
    Upgrade library error code.
*/
UpgradeHostErrorCode UpgradePartitionDataHandleDataState(uint8 *data, uint16 len, bool reqComplete)
{
    UpgradePartitionDataCtx *ctx = UpgradeCtxGetPartitionData();

    if(len != UpgradeFWIFPartitionWrite(ctx->partitionHdl, data, len))
    {
        PRINT(("UpgradeFWIFPartitionWrite(%d) failed. partitionLength %ld, bigReqSize %ld\n",
            len, ctx->partitionLength, ctx->bigReqSize));
        return UPGRADE_HOST_ERROR_PARTITION_WRITE_FAILED_DATA;
    }

    if(reqComplete)
    {
        UpgradeHostErrorCode closeStatus;
        UpgradePartitionDataRequestData(HEADER_FIRST_PART_SIZE);
        ctx->state = UPGRADE_PARTITION_DATA_STATE_GENERIC_1ST_PART;

        closeStatus = UpgradeFWIFPartitionClose(ctx->partitionHdl);
        if(UPGRADE_HOST_SUCCESS != closeStatus)
        {
            return closeStatus;
        }

        ctx->openNextPartition = TRUE;
    }

    return UPGRADE_HOST_SUCCESS;
}


/****************************************************************************
NAME
    UpgradePartitionDataCopyFromStream  -  Copy from stream.

DESCRIPTION
    Accounts for differences in offset value in CONFIG_HYDRACORE.

RETURNS
    Nothing.
*/
void UpgradePartitionDataCopyFromStream(uint8 *signature, uint16 offset, uint8 *data, uint16 len)
{
    ByteUtilsMemCpyFromStream(&signature[offset], data, len);
}

/****************************************************************************
NAME
    UpgradePartitionDataGetSigningMode

DESCRIPTION
    Gets the signing mode value set by the header.

RETURNS
    uint8 signing mode.
*/
uint8 UpgradePartitionDataGetSigningMode(void)
{
    return SigningMode;
}

