/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE

#ifndef _USB_DEVICE_MASS_STORAGE_H
#define _USB_DEVICE_MASS_STORAGE_H


#include <file.h>
#include <sink.h>
#include <source.h>


/* Mass Storage Interface Descriptor */
#define B_INTERFACE_CLASS_MASS_STORAGE 0x08 /* mass storage */
#define B_INTERFACE_SUB_CLASS_MASS_STORAGE 0x06 /* SCSI transparent command set */
#define I_INTERFACE_MASS_STORAGE 0x00 /* no string descriptor */
#define B_INTERFACE_PROTOCOL_MASS_STORAGE 0x50 /* bulk only transport */

/* Class specific requests */
#define MS_BULK_RESET 0xff
#define MS_GET_MAX_LUN 0xfe

/* Number of logical units supported on this device */
#define MAX_LUN 1

/* Command Block Wrapper (CBW) */
#define CBW_SIGNATURE_B1 0x55
#define CBW_SIGNATURE_B2 0x53
#define CBW_SIGNATURE_B3 0x42
#define CBW_SIGNATURE_B4 0x43
#define CBW_SIZE 31 /* CBW is always 31 bytes */

/* Command Status Wrapper (CSW) */
#define CSW_SIGNATURE_B1 0x55
#define CSW_SIGNATURE_B2 0x53
#define CSW_SIGNATURE_B3 0x42
#define CSW_SIGNATURE_B4 0x53
#define CSW_SIZE 13 /* CSW is always 13 bytes */
#define CSW_STATUS_PASSED 0x00
#define CSW_STATUS_FAILED 0x01
#define CSW_STATUS_PHASE_ERROR 0x02

/* SCSI commands */
#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_REQUEST_SENSE 0x03
#define SCSI_INQUIRY 0x12
#define SCSI_MODE_SENSE6 0x1a
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define SCSI_READ_FORMAT_CAPACITIES 0x23
#define SCSI_READ_CAPACITY10 0x25
#define SCSI_READ10 0x28
#define SCSI_WRITE10 0x2a
#define SCSI_VERIFY10 0x2f
#define SCSI_MODE_SENSE10 0x5a
#define SCSI_READ_CAPACITY16 0x9e
#define SCSI_READ12 0xa8
#define SCSI_WRITE12 0xaa

/* Sense Key Error Codes */
#define SENSE_ERROR_NO_SENSE 0x0
#define SENSE_ERROR_RECOVERED_ERROR 0x1
#define SENSE_ERROR_NOT_READY 0x2
#define SENSE_ERROR_MEDIUM_ERROR 0x3
#define SENSE_ERROR_HARDWARE_ERROR 0X4
#define SENSE_ERROR_ILLEGAL_REQUEST 0x5
#define SENSE_ERROR_UNIT_ATTENTION 0x6
#define SENSE_ERROR_DATA_PROTECT 0x7
#define SENSE_ERROR_BLANK_CHECK 0x8
#define SENSE_ERROR_VENDOR_SPECIFIC 0x9
#define SENSE_ERROR_COPY_ABORTED 0xa
#define SENSE_ERROR_ABORTED_COMMAND 0xb
#define SENSE_ERROR_OBSOLETE 0xc
#define SENSE_ERROR_VOLUME_OVERFLOW 0xd
#define SENSE_ERROR_MISCOMPARE 0xe

/* Sense Additional Sense Codes */
#define SENSE_ASC_INVALID_COMMAND_OPCODE 0x20
#define SENSE_ASCQ_INVALID_COMMAND_OPCODE 0x00
#define SENSE_ASC_INVALID_FIELD_IN_PARAMETER_LIST 0x26
#define SENSE_ASCQ_INVALID_FIELD_IN_PARAMETER_LIST 0x00
#define SENSE_ASC_INVALID_FIELD_IN_CDB 0x24
#define SENSE_ASCQ_INVALID_FIELD_IN_CDB 0x00

/* Sense Response Codes */
#define SENSE_RESPONSE_CURRENT 0x70
#define SENSE_RESPONSE_DEFERRED 0x71

/* Page Codes */
#define PAGE_CODE_TIMER_AND_PROTECT_PAGE 0x1c
#define PAGE_FLEXIBLE_DISK 0x05
#define PAGE_CODE_CACHING 0x08
#define PAGE_CODE_ALL_PAGES 0x3f


/* Mass Storage Command Block Wrapper (CBW) */
typedef struct
{
    uint8 dCBWSignature[4];                /* defined value to identify this packet as a CBW */
    uint8 dCBWTag[4];                      /* the tag that should be used in the CSW */
    uint8 dCBWDataTransferLength[4];       /* bytes the host expects to be transferred */
    uint8 bCBWFlags[1];                    /* bit 7 = 0 - data out from host to device; 1 - device to host: other bits = 0 */
    uint8 bCBWLUN[1];                      /* LUN that the command is the target for */
    uint8 bCBWCBLength[1];                 /* length of CBWCB */
    uint8 CBWCB[16];                       /* SCSI command */
} UsbCbwType;


/* Mass Storage Command Status Wrapper (CSW) */
typedef struct
{
    uint8 dCSWSignature[4];                /* defined value to identify this packet as a CSW */
    uint8 dCSWTag[4];                      /* the same tag as used in the CBW */
    uint8 dCSWDataResidue[4];              /* difference between dCBWDataTransferLength and the data actually sent/processed */
    uint8 bCSWStatus[1];                   /* status of the CSW */
} UsbCswType;


/* Inquiry Response structure */
typedef struct 
{
    uint8 Peripheral[1];                   /* b7-b5:Peripheral_Qualifier; b4-b0:Peripheral_DevType */
    uint8 Removble[1];                      /* b7:removable medium; b6-b0:reserved */
    uint8 Version[1];                       /* version*/
    uint8 Response_Data_Format[1];          /* b7-b6:Obsolete; b5:Access control co-ordinator; b4:hierarchical addressing support; 
                                               b3-b0:2 indicates response is in format defined by spec */
    uint8 AdditionalLength[1];              /* length in bytes of remaining in standard inquiry data */
    uint8 Sccstp[1];                        /* b7:SCCS; b6:ACC; b5-b4:TGPS; b3:3PC; b2-b1:Reserved, b0:Protected */
    uint8 bqueetc[1];                       /* b7:bque; b6:EncServ; b5:VS; b4:MultiP; b3:MChngr; b2-b1:Obsolete; b0:Addr16    */
    uint8 CmdQue[1];                        /* b7-b6:Obsolete; b5:WBUS; b4:Sync; b3:Linked; b2:Obsolete; b1:Cmdque; b0:VS */
    uint8 vendorID[8];                      /* vendor ID */
    uint8 productID[16];                    /* product ID */
    uint8 productRev[4];                    /* product revision */
} InquiryResponseType;

#define SIZE_INQUIRY_RESPONSE sizeof(InquiryResponseType)


/* Request Sense Response structure */
typedef struct 
{
    uint8 Valid_ResponseCode[1];            /* b7:Valid; b6-b0:Response Code */
    uint8 Obsolete[1];                      /* always set to 0 */
    uint8 SenseKey[1];                      /* b7:Filemark; b6:EOM; b5:ILI; b4:Reserved; b3-b0:Sense Key */
    uint8 Information[4];                   /* device type or command specific */
    uint8 AddSenseLen[1];                   /* number of additional sense bytes */
    uint8 CmdSpecificInfo[4];               /* command sepcific */
    uint8 ASC[1];                           /* additional sense code */
    uint8 ASCQ[1];                          /* additional sense code qualifier */
    uint8 FRUC[1];                          /* field replaceable unit code */
    uint8 SenseKeySpecific[3];              /* msb is SKSV */    
    /* bytes 18 - n are additional sense bytes, but not defined here */
} RequestSenseResponse;

#define SIZE_REQUEST_SENSE_RESPONSE sizeof(RequestSenseResponse)


/* Read Capacity 10 Response structure */
typedef struct 
{
    uint8 LBA[4];                        /* Last LBA value */
    uint8 BlockLength[4];                /* Block length */ 
} ReadCapacity10Response;

#define SIZE_READ_CAPACITY10_RESPONSE sizeof(ReadCapacity10Response)

/* Read Capacity 16 Response structure */
typedef struct 
{
    uint8 LBA[8];                        /* Last LBA value */
    uint8 BlockLength[4];                /* Block length */     
    uint8 ProtPType[1];                  /* b7-b4 Reserved; b3-b1 P_TYPE; b1 PROT_EN */
    uint8 Reserved[19];                  /* Reserved */   
} ReadCapacity16Response;

#define SIZE_READ_CAPACITY16_RESPONSE sizeof(ReadCapacity16Response)


/* Read Format Capacities Response structure */
typedef struct 
{
    uint8 CapacityListHeader[4];
    uint8 CurrentMaximumCapacityHeader[8];
    uint8 FormattableCapacityDescriptor[8];
} ReadFormatCapacitiesResponse;

#define SIZE_READ_FORMAT_CAPACITIES_RESPONSE sizeof(ReadFormatCapacitiesResponse)


/* Mode Parameter Header */
typedef struct 
{
    uint8 ModeDataLength[1];
    uint8 MediumType[1];
    uint8 DeviceSpecificParam[1];
    uint8 BlockDescriptorLength[1];
} ModeParameterHeader;

#define SIZE_MODE_PARAM_HEADER sizeof(ModeParameterHeader)


/* Timer Protect Page Response */
typedef struct 
{
    uint8 PageCode[1];
    uint8 PageLength[1];
    uint8 Reserved1[1];
    uint8 InactivityTimeMult[1];
    uint8 Reserved2[4];
} PageTimerProtectResponse;

#define SIZE_PAGE_TIMER_PROTECT_RESPONSE sizeof(PageTimerProtectResponse)


typedef struct
{
    uint16 index;
    uint32 size;
    uint8 *params;
    uint32 current_start_sector;
    Source src;
    uint32 end_sector;
} FileInfoType;


typedef enum
{
    FILE_INFO_FAT,
    FILE_INFO_ROOT_DIR,
    FILE_INFO_DATA,
    FILE_INFO_MAX
} file_info_value;


/* Mass Storage Type that holds all relevant data */
typedef struct
{
    RequestSenseResponse req_sense_rsp;    
    bool info_read;
    FileInfoType file_info[FILE_INFO_MAX];
} MassStorageType;


typedef enum
{
    BYTES_SCSI6,
    BYTES_SCSI10,
    BYTES_SCSI12
} bytes_scsi;


/* Functions */
bool usbEnumerateMassStorage(void);   

bool usbConfigureMassStorage(uint16 config, uint16 value_16, uint32 value_32, uint8 *params);

Sink usbMassStorageSink(void);

Source usbMassStorageSource(void);


#endif /* _USB_DEVICE_MASS_STORAGE_H */

#endif /* !USB_DEVICE_CLASS_REMOVE_MASS_STORAGE */
