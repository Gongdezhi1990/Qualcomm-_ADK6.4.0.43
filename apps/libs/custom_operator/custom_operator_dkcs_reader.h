/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


This code allows the user to read the dkcs file header.
This code may be removed once a suitable trap is added to perform this functionality.
*/

#ifndef LIBS_CUSTOM_OPERATOR_DKCS_READER_H_
#define LIBS_CUSTOM_OPERATOR_DKCS_READER_H_

#include <csrtypes.h>
#include <file.h>

typedef struct
{
    /* The header chip ID field */
    uint16 chip_id;
    /* The header build ID field */
    uint32 build_id;
    /* The number of dkcs in the file */
    uint16 num_dkcs;
    /* The number of capability IDs in all dkcs defined in the header */
    uint32 num_cap_ids;
    /* A list of capability IDs in all dkcs defined in the header */
    uint16 capability_ids[1];
} dkcs_header_t;

/* Read and return the dkcs file's header. Panics if there are any exceptions. */
dkcs_header_t *dkcsHeaderRead(FILE_INDEX file_index);

/* Free the dkcs header. */
void dkcsHeaderFree(dkcs_header_t *header);

#endif /* LIBS_CUSTOM_OPERATOR_DKCS_READER_H_ */
