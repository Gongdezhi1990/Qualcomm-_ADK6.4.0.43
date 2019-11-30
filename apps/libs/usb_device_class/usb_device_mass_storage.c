/*******************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include <message.h>
#include <panic.h>
#include <print.h>
#include <sink.h>
#include <source.h>
#include <stdlib.h>
#include <string.h>
#include <stream.h>
#include <usb.h>
#include <vmal.h>

#include "usb_device_mass_storage.h"
#include "usb_device_class.h"
#include "usb_device_class_private.h"

#include "usb_device_mass_storage_fat16.h"


#ifndef USB_DEVICE_CLASS_REMOVE_MASS_STORAGE


#ifdef DEBUG_PRINT_ENABLED
#define MS_DEBUG(x) PRINT(x)
#else
#define MS_DEBUG(x)
#endif

static MassStorageType *ms = NULL;
        
static const UsbCodes usb_codes_mass_storage = {B_INTERFACE_CLASS_MASS_STORAGE, /* bInterfaceClass */
                                                B_INTERFACE_SUB_CLASS_MASS_STORAGE, /* bInterfaceSubClass */
                                                B_INTERFACE_PROTOCOL_MASS_STORAGE, /* bInterfaceProtocol */
                                                I_INTERFACE_INDEX, /* iInterface */
                                               };


/* define two endpoints - bulk in, bulk out */
static const EndPointInfo epinfo_mass_storage_bulk[] =
{
    {
        end_point_bulk_in, /* address */
        end_point_attr_bulk, /* attributes */
        64, /* max packet size */
        1, /* poll_interval */
        0, /* data to be appended */
        0, /* length of data appended */
    },
    {
        end_point_bulk_out, /* address */
        end_point_attr_bulk, /* attributes */
        64, /* max packet size */
        1, /* poll_interval */
        0, /* data to be appended */
        0, /* length of data appended */
    }
};


/* Standard SCSI INQUIRY response */
static const InquiryResponseType inquiry_response = {
    {0x00}, /* peripheral direct access device */
    {0x80}, /* removable media */ 
    {0x04}, /* version */
    {0x02}, /* response data format */    
    {0x20}, /* additional length */
    {0x00},    
    {0x00},                    
    {0x00},    
    {' ',' ',' ',' ',' ',' ',' ',' '}, /* vendor ID */
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}, /* product ID */
    {'0','0','0','1'} /* product revision */
};


static void handleMassStorageDefaultRequest(Source req);
static void handleMassStorageBulkRequest(Source req);
static void massStorageDefaultHandler(Task task, MessageId id, Message message);
static void massStorageBulkHandler(Task task, MessageId id, Message message);


static void send_bulk_data(uint8 *data, uint16 size_data)
{
    uint8 *ptr;
    Sink ep_sink = StreamUsbEndPointSink(end_point_bulk_out);
    
    /* wait for free space in Sink */
    Fat16_WaitAvailable(ep_sink, size_data);
    
    if ((ptr = claimSink(ep_sink, size_data)) != 0)
    {
        MS_DEBUG(("USB: sending bulk data\n"));
        memmove(ptr, data, size_data);
        {
            uint16 i;
            for (i = 0; i < size_data; i++)
                MS_DEBUG(("%x ",ptr[i]));
            MS_DEBUG(("\n"));
        }        
        SinkConfigure(ep_sink, VM_SINK_USB_TRANSFER_LENGTH, size_data);
        (void) SinkFlush(ep_sink, size_data);
    }    
}

static uint16 scsi_inquiry(uint8 *data, uint32 dataXferLen)
{ 
    uint16 allocation_length = (data[2] << 8) | data[3];
    
    if ((allocation_length < SIZE_INQUIRY_RESPONSE) || (dataXferLen < SIZE_INQUIRY_RESPONSE))
        send_bulk_data((uint8 *)&inquiry_response, allocation_length < dataXferLen ? allocation_length : dataXferLen);  
    else
        send_bulk_data((uint8 *)&inquiry_response, SIZE_INQUIRY_RESPONSE);
    
    return CSW_STATUS_PASSED;
}

static uint16 scsi_request_response(uint8 *data, uint32 dataXferLen)
{
    uint16 allocation_length = data[3];
    
    if ((allocation_length < SIZE_REQUEST_SENSE_RESPONSE) || (dataXferLen < SIZE_REQUEST_SENSE_RESPONSE))
        send_bulk_data((uint8 *)&ms->req_sense_rsp, allocation_length < dataXferLen ? allocation_length : dataXferLen);  
    else
        send_bulk_data((uint8 *)&ms->req_sense_rsp, SIZE_REQUEST_SENSE_RESPONSE);  
    
    return CSW_STATUS_PASSED;
}

static void initialise_request_sense_response(void)
{
    memset(&ms->req_sense_rsp, 0, SIZE_REQUEST_SENSE_RESPONSE);
    
    ms->req_sense_rsp.Valid_ResponseCode[0] = SENSE_RESPONSE_CURRENT;
    ms->req_sense_rsp.SenseKey[0] = SENSE_ERROR_NO_SENSE;
    ms->req_sense_rsp.AddSenseLen[0] = 0xa; /* n-7 = SIZE_REQUEST_SENSE_RESPONSE - 1 - 7 */
}

static uint16 scsi_read(uint8 *data, uint32 dataXferLen, bytes_scsi version)
{
    uint32 lba = (((uint32)data[1] << 24) & 0xff000000ul) | (((uint32)data[2] << 16) & 0xff0000) | (((uint32)data[3] << 8) & 0xff00) | (data[4] & 0xff);
    uint32 transfer_length;
    uint16 status = CSW_STATUS_PASSED;  
    
    if (version == BYTES_SCSI12)
        transfer_length = (((uint32)data[5] << 24) & 0xff000000ul) | (((uint32)data[6] << 16) & 0xff0000) | (((uint32)data[7] << 8) & 0xff00) | (data[8] & 0xff);
    else
        transfer_length = (((uint16)data[6] << 8) & 0xff00) | (data[7] & 0xff);

    if ((transfer_length * Fat16_GetBlockSize()) > dataXferLen)
    {        
        /* the amount of data that needs to be sent to the host exceeds the amount the host wants to receive, 
            so clip the total read blocks to fit the hosts requirements
        */
        transfer_length = dataXferLen / Fat16_GetBlockSize();
        status = CSW_STATUS_PHASE_ERROR;
    }
    
    if (transfer_length)
    {
        Fat16_Read(ms, lba, transfer_length);
    }
    
    return status;
}

static uint16 scsi_read_capacity10(uint32 dataXferLen)
{
    ReadCapacity10Response response;
    uint32 block_size = Fat16_GetBlockSize();
    uint32 last_lba = Fat16_GetTotalBlocks() - 1;
       
    response.LBA[0] = (last_lba & 0xff000000ul) >> 24;
    response.LBA[1] = (last_lba & 0xff0000) >> 16;
    response.LBA[2] = (last_lba & 0xff00) >> 8;
    response.LBA[3] = last_lba & 0xff;
    
    response.BlockLength[0] = (block_size & 0xff000000ul) >> 24;
    response.BlockLength[1] = (block_size & 0xff0000) >> 16;
    response.BlockLength[2] = (block_size & 0xff00) >> 8;
    response.BlockLength[3] = block_size & 0xff;
    
    if (dataXferLen < SIZE_READ_CAPACITY10_RESPONSE)
        send_bulk_data((uint8 *)&response, dataXferLen); 
    else
        send_bulk_data((uint8 *)&response, SIZE_READ_CAPACITY10_RESPONSE);  
 
    return CSW_STATUS_PASSED;
}

static uint16 scsi_read_capacity16(uint32 dataXferLen)
{
    ReadCapacity16Response response;
    uint32 block_size = Fat16_GetBlockSize();
    uint32 last_lba = Fat16_GetTotalBlocks() - 1;
           
    response.LBA[0] = 0x00;
    response.LBA[1] = 0x00;
    response.LBA[2] = 0x00;
    response.LBA[3] = 0x00;
    response.LBA[4] = (last_lba & 0xff000000ul) >> 24;
    response.LBA[5] = (last_lba & 0xff0000) >> 16;
    response.LBA[6] = (last_lba & 0xff00) >> 8;
    response.LBA[7] = last_lba & 0xff;
    
    response.BlockLength[0] = (block_size & 0xff000000ul) >> 24;
    response.BlockLength[1] = (block_size & 0xff0000) >> 16;
    response.BlockLength[2] = (block_size & 0xff00) >> 8;
    response.BlockLength[3] = block_size & 0xff;
    
    response.ProtPType[0] = 0x00;   /* no protection */
    
    memset(&response.Reserved, 0, sizeof(response.Reserved));
    
    if (dataXferLen < SIZE_READ_CAPACITY16_RESPONSE)
        send_bulk_data((uint8 *)&response, dataXferLen); 
    else
        send_bulk_data((uint8 *)&response, SIZE_READ_CAPACITY16_RESPONSE);  
 
    return CSW_STATUS_PASSED;
}

static uint16 scsi_read_format_capacities(uint32 dataXferLen)
{
    ReadFormatCapacitiesResponse response;
    uint32 block_size = Fat16_GetBlockSize();
    uint32 total_blocks = Fat16_GetTotalBlocks();
    
    /* Capacity List Header */
    response.CapacityListHeader[0] = 0x00;
    response.CapacityListHeader[1] = 0x00;
    response.CapacityListHeader[2] = 0x00;
    response.CapacityListHeader[3] = 0x10; /* capacity list length */
    
    /* Current Capacity Header */
    /* Number of Blocks - 4 bytes */
    response.CurrentMaximumCapacityHeader[0] = (total_blocks & 0xff000000ul) >> 24;
    response.CurrentMaximumCapacityHeader[1] = (total_blocks & 0xff0000) >> 16;
    response.CurrentMaximumCapacityHeader[2] = (total_blocks & 0xff00) >> 8;
    response.CurrentMaximumCapacityHeader[3] = total_blocks & 0xff;
    /* Descriptor Code */
    response.CurrentMaximumCapacityHeader[4] = 0x02; /* Formatted Media - Current media capacity */
    /* Block Length */
    response.CurrentMaximumCapacityHeader[5] = (block_size & 0xff0000) >> 16;
    response.CurrentMaximumCapacityHeader[6] = (block_size & 0xff00) >> 8;
    response.CurrentMaximumCapacityHeader[7] = block_size & 0xff;
    
    /* Formattable Capacity Descriptor */
    /* Number of Blocks - 4 bytes */
    response.FormattableCapacityDescriptor[0] = (total_blocks & 0xff000000ul) >> 24;
    response.FormattableCapacityDescriptor[1] = (total_blocks & 0xff0000) >> 16;
    response.FormattableCapacityDescriptor[2] = (total_blocks & 0xff00) >> 8;
    response.FormattableCapacityDescriptor[3] = total_blocks & 0xff;
    /* Reserved */
    response.FormattableCapacityDescriptor[4] = 0x00;
    /* Block Length */
    response.FormattableCapacityDescriptor[5] = (block_size & 0xff0000) >> 16;
    response.FormattableCapacityDescriptor[6] = (block_size & 0xff00) >> 8;
    response.FormattableCapacityDescriptor[7] = block_size & 0xff;
    
    if (dataXferLen < SIZE_READ_FORMAT_CAPACITIES_RESPONSE)
        send_bulk_data((uint8 *)&response, dataXferLen); 
    else
        send_bulk_data((uint8 *)&response, SIZE_READ_FORMAT_CAPACITIES_RESPONSE); 
    
    return CSW_STATUS_PASSED;
}

static uint16 scsi_mode_sense(uint8 *data, uint32 dataXferLen, bytes_scsi version)
{
    uint8 page_code = data[1] & 0x3f;
    uint8 response[12];
    uint16 data_length = 0;
    uint16 size_header = SIZE_MODE_PARAM_HEADER;
    ModeParameterHeader header;
    uint16 allocation_length;
    
    if (version == BYTES_SCSI10)
        allocation_length = (data[6] << 8) | data[7];
    else
        allocation_length = data[3];
    
    switch (page_code)
    {
        case PAGE_CODE_TIMER_AND_PROTECT_PAGE: /* Timer And Protect Page */
        {            
            PageTimerProtectResponse page;
            
            header.ModeDataLength[0] = SIZE_MODE_PARAM_HEADER + SIZE_PAGE_TIMER_PROTECT_RESPONSE - 1 ;
            header.MediumType[0] = 0x00;
            header.DeviceSpecificParam[0] = 0x80; /* bit 7 marks the media as un-writable */
            header.BlockDescriptorLength[0] = 0x00;
            
            page.PageCode[0] = PAGE_CODE_TIMER_AND_PROTECT_PAGE;
            page.PageLength[0] = 0x06;
            page.Reserved1[0] = 0x00;
            page.InactivityTimeMult[0] = 0x05;
            page.Reserved2[0] = 0x00;
            page.Reserved2[1] = 0x00;
            page.Reserved2[2] = 0x00;
            page.Reserved2[3] = 0x00;
            data_length = SIZE_PAGE_TIMER_PROTECT_RESPONSE;
            
            memcpy(response, (uint8 *)&header, SIZE_MODE_PARAM_HEADER);
            memcpy(response + SIZE_MODE_PARAM_HEADER, (uint8 *)&page, SIZE_PAGE_TIMER_PROTECT_RESPONSE);
            break;
        }
        case PAGE_CODE_CACHING:
        case PAGE_CODE_ALL_PAGES: /* Return all pages */
        {
            header.ModeDataLength[0] = SIZE_MODE_PARAM_HEADER - 1;
            header.MediumType[0] = 0x00;
            header.DeviceSpecificParam[0] = 0x80; /* bit 7 marks the media as un-writable */
            header.BlockDescriptorLength[0] = 0x00;
            
            data_length = 0;
           
            memcpy(response, (uint8 *)&header, SIZE_MODE_PARAM_HEADER);
                       
            break;
        }
        default:
        {
            return CSW_STATUS_FAILED;
        }
    }
    
    if ((allocation_length < (size_header + data_length)) || (dataXferLen < (size_header + data_length)))
        send_bulk_data((uint8 *)&response, allocation_length < dataXferLen ? allocation_length : dataXferLen);
    else
        send_bulk_data((uint8 *)&response, size_header + data_length);
    
    return CSW_STATUS_PASSED;
}


static uint16 scsi_write(uint8 *data, uint32 dataXferLen, bytes_scsi version)
{
    uint32 transfer_length;
    uint16 status = CSW_STATUS_PASSED;  
    
    if (version == BYTES_SCSI12)
        transfer_length = (((uint32)data[5] << 24) & 0xff000000ul) | (((uint32)data[6] << 16) & 0xff0000) | (((uint32)data[7] << 8) & 0xff00) | (data[8] & 0xff);
    else
        transfer_length = (((uint16)data[6] << 8) & 0xff00) | (data[7] & 0xff);
        
    if ((transfer_length * Fat16_GetBlockSize()) > dataXferLen)
        status = CSW_STATUS_FAILED;
        
    return status;    
}


static void process_cbw(Source req, uint16 packet_size)
{
    uint16 i;
    UsbCbwType cbw;
    UsbCswType csw;
    uint8 res0=0;
    uint8 res1=0;
    uint8 res2=0;
    uint8 res3=0;
    const uint8 *ptr = SourceMap(req);
    bool found = FALSE;
    uint16 status_code = CSW_STATUS_PASSED;
    uint32 dataXferLen;
  
    /* check for CBW */
    if (packet_size >= CBW_SIZE)
    {
        for (i = 0; i < packet_size; i++)
        {
            if ((packet_size - i) >= CBW_SIZE)
            {
                if (ptr[i] == CBW_SIGNATURE_B1)
                {                    
                    if ((ptr[i+1] == CBW_SIGNATURE_B2) &&
                        (ptr[i+2] == CBW_SIGNATURE_B3) &&
                        (ptr[i+3] == CBW_SIGNATURE_B4) &&
                        ((ptr[i+12] == 0x00) || (ptr[i+12] == 0x80)) &&
                        (ptr[i+13] < MAX_LUN) &&
                        (ptr[i+14] <= 0x10) &&
                        (ptr[i+14] >= 0x01))
                    {
                        MS_DEBUG(("\n    found CBW, SourceDrop:%d\n", i));
                        SourceDrop(req, i);
                        ptr = SourceMap(req);
                        found = TRUE;
                        break;
                    }
                }
            }            
        }
        
        if (!found)
        {
            SourceDrop(req, packet_size);
            MS_DEBUG(("\n    couldn't find CBW, SourceDrop:%d\n", packet_size));
            return; 
        }
            
        memmove(&cbw, ptr, CBW_SIZE);
        
        /* Bytes 0-3 are dCBWSignature */
        MS_DEBUG(("USB: CBW signature "));
        for (i = 0; i < 4; i++)
            MS_DEBUG(("0x%x ", cbw.dCBWSignature[i]));
        /* Bytes 4-7 are dCBWTag */
        MS_DEBUG(("\nUSB: CBW tag "));
        for (i = 0; i < 4; i++)
            MS_DEBUG(("0x%x ", cbw.dCBWTag[i]));
        /* Bytes 8-11 are dCBWDataTransferLength */
        MS_DEBUG(("\nUSB: CBW transfer length "));
        for (i = 0; i < 4; i++)
            MS_DEBUG(("0x%x ", cbw.dCBWDataTransferLength[i]));
        /* Byte 12 is bmCBWFlags */
        MS_DEBUG(("\nUSB: CBW flags 0x%x\n", cbw.bCBWFlags[0]));
        /* Byte 13 is bCBWLUN */
        MS_DEBUG(("USB: CBW LUN 0x%x\n", cbw.bCBWLUN[0]));
        /* Byte 14 is bCBWCBLength */
        MS_DEBUG(("USB: CBW CB length 0x%x\n", cbw.bCBWCBLength[0]));
        /* Bytes 15-30 are CBWCB */
        MS_DEBUG(("USB: CBW CB "));
        for (i = 0; i < cbw.bCBWCBLength[0]; i++)
        {
            MS_DEBUG(("%x ", cbw.CBWCB[i]));
        }
        MS_DEBUG(("\n"));
    
        /* the CBW has been verified so start building CSW */
        
        dataXferLen = ((uint32)cbw.dCBWDataTransferLength[3] << 24) | 
                        ((uint32)cbw.dCBWDataTransferLength[2] << 16) | 
                        (cbw.dCBWDataTransferLength[1] << 8) | 
                        cbw.dCBWDataTransferLength[0];
        
        MS_DEBUG(("USB: CBW xfer len %ld\n",dataXferLen));
        
        /* write CSW signature */
        csw.dCSWSignature[0] = CSW_SIGNATURE_B1;
        csw.dCSWSignature[1] = CSW_SIGNATURE_B2;
        csw.dCSWSignature[2] = CSW_SIGNATURE_B3;
        csw.dCSWSignature[3] = CSW_SIGNATURE_B4;
        /* copy CBW tag to CSW */
        for (i = 0; i < 4; i++)
        {
            csw.dCSWTag[i] = cbw.dCBWTag[i];
        }
        
        if (dataXferLen != 0)
        {                
            /* the data transfer length is non-zero, so the device can send SCSI data back to host */
            if (cbw.bCBWFlags[0] == 0x80)
            {
                /* data expected device -> host */
                
                /* parse the SCSI commands */
                switch (cbw.CBWCB[0])
                {
                    case SCSI_TEST_UNIT_READY:
                    {
                        MS_DEBUG(("USB: SCSI_TEST_UNIT_READY\n"));
                        /* accept this command */
                        break;
                    }
                    case SCSI_REQUEST_SENSE:
                    {
                        MS_DEBUG(("USB: SCSI_REQUEST_SENSE\n"));
                        status_code = scsi_request_response(&cbw.CBWCB[1], dataXferLen);
                        break;
                    }
                    case SCSI_INQUIRY:
                    {
                        MS_DEBUG(("USB: SCSI_INQUIRY\n"));     
                        status_code = scsi_inquiry(&cbw.CBWCB[1], dataXferLen);
                        break;
                    }         
                    case SCSI_READ10:
                    {
                        MS_DEBUG(("USB: SCSI_READ_10\n"));
                        status_code = scsi_read(&cbw.CBWCB[1], dataXferLen, BYTES_SCSI10);                      
                        break;
                    }  
                    case SCSI_READ12:
                    {
                        MS_DEBUG(("USB: SCSI_READ_12\n"));
                        status_code = scsi_read(&cbw.CBWCB[1], dataXferLen, BYTES_SCSI12);                      
                        break;
                    }  
                    case SCSI_READ_CAPACITY10:
                    {
                        MS_DEBUG(("USB: SCSI_READ_CAPACITY10\n"));
                        status_code = scsi_read_capacity10(dataXferLen);
                        break;
                    }
                    case SCSI_READ_CAPACITY16:
                    {
                        MS_DEBUG(("USB: SCSI_READ_CAPACITY16\n"));
                        status_code = scsi_read_capacity16(dataXferLen);
                        break;
                    }                    
                    case SCSI_READ_FORMAT_CAPACITIES:
                    {
                        MS_DEBUG(("USB: SCSI_READ_FORMAT_CAPACITIES\n"));
                        status_code = scsi_read_format_capacities(dataXferLen);
                        break;
                    }
                    case SCSI_MODE_SENSE6:
                    {
                        MS_DEBUG(("SCSI_MODE_SENSE6\n"));
                        status_code = scsi_mode_sense(&cbw.CBWCB[1], dataXferLen, BYTES_SCSI6);
                        break;
                    }
                    case SCSI_MODE_SENSE10:
                    {
                        MS_DEBUG(("SCSI_MODE_SENSE10\n"));
                        status_code = scsi_mode_sense(&cbw.CBWCB[1], dataXferLen, BYTES_SCSI10);
                        break;
                    }
                    case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
                    {
                        MS_DEBUG(("SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL\n"));
                        /* accept this command */
                        break;
                    }
                    default:
                    {
                        MS_DEBUG(("USB: Unhandled SCSI 0x%x\n", cbw.CBWCB[0]));
                        status_code = CSW_STATUS_FAILED;
                        initialise_request_sense_response();
                        ms->req_sense_rsp.SenseKey[0] = SENSE_ERROR_ILLEGAL_REQUEST; 
                        ms->req_sense_rsp.ASC[0] = SENSE_ASC_INVALID_FIELD_IN_PARAMETER_LIST; 
                        ms->req_sense_rsp.ASCQ[0] = SENSE_ASCQ_INVALID_FIELD_IN_PARAMETER_LIST;
                        res0 = cbw.dCBWDataTransferLength[0];
                        res1 = cbw.dCBWDataTransferLength[1];
                        res2 = cbw.dCBWDataTransferLength[2];
                        res3 = cbw.dCBWDataTransferLength[3];
                        break;
                    }
                }
            }
            else
            {        
                /* data expected host -> device */
                if ((cbw.CBWCB[0] != SCSI_WRITE10) && (cbw.CBWCB[0] != SCSI_WRITE12))
                {
                    /* host is trying to send non-CBW on bulk out endpoint, we should not allow this */
                    status_code = CSW_STATUS_FAILED;
                    res0 = cbw.dCBWDataTransferLength[0];
                    res1 = cbw.dCBWDataTransferLength[1];
                    res2 = cbw.dCBWDataTransferLength[2];
                    res3 = cbw.dCBWDataTransferLength[3];
                    /* update sense data */ 
                    initialise_request_sense_response();
                    ms->req_sense_rsp.SenseKey[0] = SENSE_ERROR_ILLEGAL_REQUEST; 
                    ms->req_sense_rsp.ASC[0] = SENSE_ASC_INVALID_FIELD_IN_PARAMETER_LIST; 
                    ms->req_sense_rsp.ASCQ[0] = SENSE_ASCQ_INVALID_FIELD_IN_PARAMETER_LIST; 
                    MS_DEBUG(("    non-CBW data not allowed SCSI:[%d]\n", cbw.CBWCB[0]));                    
                }
                else
                {
                    /* handle SCSI Write command as data will be actually sent */
                    bytes_scsi version;
                    if (cbw.CBWCB[0] == SCSI_WRITE12)
                        version = BYTES_SCSI12;
                    else
                        version = BYTES_SCSI10;
                    
                    status_code = scsi_write(&cbw.CBWCB[1], dataXferLen, version);
                    if (status_code == CSW_STATUS_FAILED)
                    {
                        res0 = cbw.dCBWDataTransferLength[0];
                        res1 = cbw.dCBWDataTransferLength[1];
                        res2 = cbw.dCBWDataTransferLength[2];
                        res3 = cbw.dCBWDataTransferLength[3];
                        /* update sense data */ 
                        initialise_request_sense_response();
                        ms->req_sense_rsp.SenseKey[0] = SENSE_ERROR_ILLEGAL_REQUEST; 
                        ms->req_sense_rsp.ASC[0] = SENSE_ASC_INVALID_FIELD_IN_PARAMETER_LIST; 
                        ms->req_sense_rsp.ASCQ[0] = SENSE_ASCQ_INVALID_FIELD_IN_PARAMETER_LIST;
                    }
                }
            }
        }
        else
        {
            /* the data transfer length is zero, so the device should only send back the CSW with the correct status code */
            switch (cbw.CBWCB[0])
            {
                case SCSI_REQUEST_SENSE:
                case SCSI_INQUIRY:
                case SCSI_READ10:
                case SCSI_READ12:
                case SCSI_READ_CAPACITY10:
                case SCSI_READ_FORMAT_CAPACITIES:
                case SCSI_MODE_SENSE6:
                case SCSI_MODE_SENSE10:
                case SCSI_WRITE10:
                case SCSI_WRITE12:
                case SCSI_VERIFY10:
                    status_code = CSW_STATUS_PHASE_ERROR;
                    MS_DEBUG(("USB: Data tranfer length zero, Phase Error SCSI:[%d]\n", cbw.CBWCB[0]));
                    break;
                default:
                    MS_DEBUG(("USB: Data tranfer length zero, SCSI:[%d]\n", cbw.CBWCB[0]));
                    break;
             }                        
        }
        
        /* send CSW */
        csw.dCSWDataResidue[0] = res0;
        csw.dCSWDataResidue[1] = res1;
        csw.dCSWDataResidue[2] = res2;
        csw.dCSWDataResidue[3] = res3;
        csw.bCSWStatus[0] = status_code;
        send_bulk_data((uint8 *)&csw, CSW_SIZE);             

        SourceDrop(req, CBW_SIZE);
        MS_DEBUG(("\n    discard CBW, SourceDrop:%d\n", CBW_SIZE));
    }
    else
    {
        SourceDrop(req, packet_size);
        MS_DEBUG(("\n    small packet, SourceDrop:%d\n", packet_size));
    }
}


static void handleMassStorageDefaultRequest(Source req)
{
    uint16 packet_size;
    Sink sink = StreamSinkFromSource(req);

    /* Check for outstanding Class requests */
    while ((packet_size = SourceBoundary(req)) != 0)
    {
        MS_DEBUG(("USB: packet_size = %d\n", packet_size));
        if (SourceSizeHeader(req))
        {
            /*
                This must be a class specific request so build the response. 
                It must contain the original request, so copy from the source header.
            */
            UsbResponse usbresp;
            memcpy(&usbresp.original_request, SourceMapHeader(req), sizeof(UsbRequest));

            /* Set the response fields to default values to make the code below simpler */
            usbresp.success = FALSE;
            usbresp.data_length = 0;
            
            MS_DEBUG(("USB: found header\n"));
    
            if (usbresp.original_request.bRequest == MS_BULK_RESET)
            {
                MS_DEBUG(("USB: BULK_RESET\n"));
                if(usbresp.original_request.wValue == 0)
                {
                    usbresp.success = TRUE;
                }
            }
            else if (usbresp.original_request.bRequest == MS_GET_MAX_LUN)
            {
                MS_DEBUG(("USB: GET_MAX_LUN\n"));
                if(usbresp.original_request.wValue == 0)
                {
                    uint8 *ptr = claimSink(sink, 1);
                    if (ptr != 0)
                    {
                        ptr[0] = MAX_LUN - 1; /* Number of Logical Units supported - 1 */           
                        usbresp.data_length = 1;  
                        usbresp.success = TRUE;
                    }
                }
            }
        

            /* Send response */
            if (usbresp.data_length)
            {
                (void)SinkFlushHeader(sink, usbresp.data_length, (void *)&usbresp, sizeof(UsbResponse));
            }
            else
            {
                /* Sink packets can never be zero-length, so flush a dummy byte */
                (void)SinkClaim(sink, 1);
                (void)SinkFlushHeader(sink, 1, (void *)&usbresp, sizeof(UsbResponse));
            }
        }
       
        /* Discard the original request */
        SourceDrop(req, packet_size);
        MS_DEBUG(("USB: source drop = %d\n", packet_size));
    }  
}


static void handleMassStorageBulkRequest(Source req)
{    
    uint16 packet_size;
  
    /* Check for outstanding Class requests */
    while ((packet_size = SourceBoundary(req)) != 0)
    {
        MS_DEBUG(("USB: packet_size = %d\n", packet_size));       
       
        if (SourceMap(req) != NULL)
        {
            MS_DEBUG(("USB: CBW data?\n"));             
                    
            /* Process data received from host, which should be a CBW */
            process_cbw(req, packet_size);
        }
    }
}


static void massStorageDefaultHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    MS_DEBUG(("USB: default\n"));
    if (id == MESSAGE_MORE_DATA)
    {
        MS_DEBUG(("USB: MESSAGE_MORE_DATA mass storage DEFAULT\n"));
        handleMassStorageDefaultRequest(StreamUsbClassSource(device->usb_interface[usb_interface_mass_storage]));
    }
}


static void massStorageBulkHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    MS_DEBUG(("USB: bulk\n"));
    if (id == MESSAGE_MORE_DATA)
    {
        MS_DEBUG(("USB: MESSAGE_MORE_DATA mass storage BULK\n"));      
        handleMassStorageBulkRequest(StreamUsbEndPointSource(end_point_bulk_in));
    }
}


bool usbEnumerateMassStorage(void)
{
    Sink sink_bulk_out;
    
    device->usb_interface[usb_interface_mass_storage] = UsbAddInterface(&usb_codes_mass_storage, 0, 0, 0);

    if (device->usb_interface[usb_interface_mass_storage] == usb_interface_error)
        return FALSE;

    /* Add required endpoints to the interface */
    if (!UsbAddEndPoints(device->usb_interface[usb_interface_mass_storage], 2, epinfo_mass_storage_bulk))
        return FALSE;
    
    /* message handling for default commands */
    device->usb_task[usb_task_mass_storage_default].handler = massStorageDefaultHandler;    
    MessageStreamTaskFromSink(StreamUsbClassSink(device->usb_interface[usb_interface_mass_storage]), &device->usb_task[usb_task_mass_storage_default]);
    
    /* message handling for bulk commands */
    device->usb_task[usb_task_mass_storage_bulk].handler = massStorageBulkHandler;
    MessageStreamTaskFromSink(StreamUsbEndPointSink(end_point_bulk_in), &device->usb_task[usb_task_mass_storage_bulk]);
    
    ms = PanicUnlessNew(MassStorageType);
    sink_bulk_out = StreamUsbEndPointSink(end_point_bulk_out);

    /* configure Sink for mass storage use (not required for Hydracore but calling SinkConfigure should have no effect) */   
    SinkConfigure(sink_bulk_out, VM_SINK_USB_TERMINATION, 1); /* mass storage needs implicit termination */
    SinkConfigure(sink_bulk_out, VM_SINK_USB_VM_SUPPLIES_LENGTH, 1); /* mass storage needs VM to supply data length before data transfer */
    SinkConfigure(sink_bulk_out, VM_SINK_USB_SUPPORT_LARGE_FILES, 1); /* mass storage needs large file support */    
        
    initialise_request_sense_response();
    Fat16_Initialise(ms);
    
    MS_DEBUG(("USB: Enumerate mass storage\n"));
    
    return TRUE;
}


bool usbConfigureMassStorage(uint16 config, uint16 value_16, uint32 value_32, uint8 *params)
{
    switch (config)
    {
        case USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_DATA_AREA:
        {
            if (ms == NULL)
                return FALSE; /* mass storage must be enumerated first */
            Fat16_ConfigureDataArea(ms, value_16, value_32, params);
            return TRUE;          
        }
        case USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_TABLE:
        {
            if (ms == NULL)
                return FALSE; /* mass storage must be enumerated first */
            Fat16_ConfigureFat(ms, value_16, value_32, params);
            return TRUE;          
        }
        case USB_DEVICE_CLASS_CONFIG_MASS_STORAGE_FAT_ROOT_DIR:
        {
            if (ms == NULL)
                return FALSE; /* mass storage must be enumerated first */
            Fat16_ConfigureRootDir(ms, value_16, value_32, params);
            return TRUE;          
        }
        default:
        {   
            break;
        }
    }
    
    return FALSE;
}


Sink usbMassStorageSink(void)
{
    return StreamUsbEndPointSink(end_point_bulk_out);
}


Source usbMassStorageSource(void)
{
    return StreamUsbEndPointSource(end_point_bulk_in);
}


#endif /* USB_DEVICE_CLASS_REMOVE_MASS_STORAGE */
    
