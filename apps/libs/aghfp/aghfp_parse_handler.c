/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#include "aghfp.h"
#include "aghfp_ok.h"
#include "aghfp_parse.h"
#include "aghfp_private.h"
#include "aghfp_wbs.h"
#include "aghfp_common.h"

#include <message.h>
#include <panic.h>
#include <string.h>
#include <util.h>


/****************************************************************************
	Called when we receive data that cannot be recognised by the AT cmd
	parser. To stop us from panicking if the AG is spamming us with
	unrecognised data e.g. some out of spec AG is sending us the wrong
	commands, we only send the unrecognised data to the app if we have the
	resources, otherwise we silently ignore it!
*/
void handleUnrecognised(const uint8 *data, uint16 length, Task task)
{
	AGHFP *aghfp = (AGHFP *) task;

	if (!length)
	{
		return;
	}

	/* 	Create a message and send it directly to the app.
		No point going through the state machine because we don't
		know what this data is anyway. */
	{
		MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_UNRECOGNISED_AT_CMD_IND, length);

		if (message)
		{
			message->aghfp = aghfp;
			message->size_data = length;
			memmove(message->data, data, length);
			MessageSend(aghfp->client_task, AGHFP_UNRECOGNISED_AT_CMD_IND, message);
		}
		/* If we didn't alloc the mesage don't panic, just ignore this we only send these up if we have spare resources */
	}
}


/* HS has sent us AT+CKPD=xxx command */
void aghfpHandleCkpdParse(Task task, const struct aghfpHandleCkpdParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_CKPD_CMD);
    message->keycode = data->keycode;
    MessageSend(task, AGHFP_INTERNAL_AT_CKPD_CMD, message);
}


/* HF has sent us AT+BRSF=xxx, informing us of its supported features. Pass this
   message on to the profile handler. */
void aghfpHandleBrsfReqParse(Task task, const struct aghfpHandleBrsfReqParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_BRSF_REQ);
    message->hf_supported_features = data->supportedFeatures;
    MessageSend(task, AGHFP_INTERNAL_AT_BRSF_REQ, message);
}


void aghfpHandleCindSupportReqParse(Task task)
{
    MessageSend(task, AGHFP_INTERNAL_AT_CIND_SUPPORTED_REQ, NULL);
}


void aghfpHandleCindStatusReqParse(Task task)
{
    AGHFP *aghfp = (AGHFP *) task;
    if(aghfp->cind_poll_client)
    {
        MAKE_AGHFP_MESSAGE(AGHFP_CALL_INDICATIONS_STATUS_REQUEST_IND);
        message->aghfp = aghfp;
        MessageSend(aghfp->client_task, AGHFP_CALL_INDICATIONS_STATUS_REQUEST_IND, message);
    }
    else
    {
        MessageSend(task, AGHFP_INTERNAL_AT_CIND_STATUS_REQ, NULL);
    }
}


void aghfpHandleCmerReqParse(Task task, const struct aghfpHandleCmerReqParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_CMER_REQ);
    message->mode = data->mode;
    message->ind = data->ind;
    MessageSend(task, AGHFP_INTERNAL_AT_CMER_REQ, message);
}


void aghfpHandleChldSupportReqParse(Task task)
{
    MessageSend(task, AGHFP_INTERNAL_CALL_HOLD_SUPPORT_REQ, NULL);
}


void aghfpHandleChldParse(Task task, const struct aghfpHandleChldParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CALL_HOLD_REQ);
    message->action = (uint16)(*data->cmd.data - '0');
    if ( data->cmd.length>1 )
    {
        (void)UtilGetNumber(data->cmd.data+1,data->cmd.data+data->cmd.length-1, &message->index);
    }
    else
    {
        message->index = 0;
    }
    MessageSend(task, AGHFP_INTERNAL_CALL_HOLD_REQ, message);
}


void aghfpHandleAtaParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_ANSWER_REQ, NULL);
}


void aghfpHandleClipParse(Task task, const struct aghfpHandleClipParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CALLER_ID_SETUP_REQ);
	message->state = data->state;
	MessageSend(task, AGHFP_INTERNAL_CALLER_ID_SETUP_REQ, message);
}


void aghfpHandleCcwaParse(Task task, const struct aghfpHandleCcwaParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CALL_WAITING_SETUP_REQ);
	message->state = data->state;
	MessageSend(task, AGHFP_INTERNAL_CALL_WAITING_SETUP_REQ, message);
}


void aghfpHandleChupParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_CALL_HANG_UP_REQ, NULL);
}


void aghfpHandleDialParse(Task task, const struct aghfpHandleDialParse *data)
{
	AGHFP *aghfp = (AGHFP *)task;
    MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_DIAL_REQ, data->number.length);

    if (message)
    {
    	message->size_number = data->number.length;
        memmove(message->number, data->number.data, data->number.length * sizeof(uint8));
		MessageSend(task, AGHFP_INTERNAL_DIAL_REQ, message);
    }
    else
    {
    	aghfpSendError(aghfp);
    }
}


void aghfpHandleMemoryDialParse(Task task, const struct aghfpHandleMemoryDialParse *data)
{
	AGHFP *aghfp = (AGHFP *)task;
    MAKE_AGHFP_MESSAGE_WITH_ARRAY(AGHFP_INTERNAL_MEMORY_DIAL_REQ, data->number.length);

    if (message)
    {
    	message->size_number = data->number.length;
    	memmove(message->number, data->number.data, data->number.length * sizeof(uint8));
        MessageSend(task, AGHFP_INTERNAL_MEMORY_DIAL_REQ, message);
    }
    else
    {
    	aghfpSendError(aghfp);
    }
}


void aghfpHandleBldnParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_LAST_NUMBER_REDIAL_REQ, NULL);
}


void aghfpHandleNrecParse(Task task, const struct aghfpHandleNrecParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_NREC_SETUP_REQ);
	message->state = data->state;
	MessageSend(task, AGHFP_INTERNAL_NREC_SETUP_REQ, message);
}


void aghfpHandleBvraParse(Task task, const struct aghfpHandleBvraParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ);
	message->state = data->state;
	MessageSend(task, AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ, message);
}


void aghfpHandleBinpParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_PHONE_NUMBER_REQ, NULL);
}


void aghfpHandleVtsParse(Task task, const struct aghfpHandleVtsParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_TRANSMIT_DTMF_CODE);
	message->code = data->code.data[0];
	MessageSend(task, AGHFP_INTERNAL_TRANSMIT_DTMF_CODE, message);
}


void aghfpHandleVgmParse(Task task, const struct aghfpHandleVgmParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SYNC_MIC_GAIN);
	message->gain = data->gain;
	MessageSend(task, AGHFP_INTERNAL_SYNC_MIC_GAIN, message);
}


void aghfpHandleVgsParse(Task task, const struct aghfpHandleVgsParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME);
	message->volume = data->volume;
	MessageSend(task, AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME, message);
}


void aghfpHandleBtrhStatusRequestParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_RESPONSE_HOLD_STATUS_REQUEST_REQ, NULL);
}


void aghfpHandleBtrhSetStatusParse(Task task, const struct aghfpHandleBtrhSetStatusParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ);
	message->cmd = data->cmd;
	MessageSend(task, AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ, message);
}


void aghfpHandleCnumParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_SUBSCRIBER_NUMBER_REQ, NULL);
}


void aghfpHandleClccParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_CURRENT_CALLS_REQ, NULL);
}


void aghfpHandleCopsFormatParse(Task task, const struct aghfpHandleCopsFormatParse *data)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ);
	message->mode = data->mode;
	message->format = data->format;
	MessageSend(task, AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ, message);
}


void aghfpHandleCopsStatusParse(Task task)
{
	MessageSend(task, AGHFP_INTERNAL_NETWORK_OPERATOR_REQ, NULL);
}


void aghfpHandleBiaParse(Task task, const struct aghfpHandleBiaParse *data)
{
    struct sequence indicators = data->indicators;
    uint16 i, indicator_idx=0;    
    bool success = TRUE;
    
    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ);
    
    /* By default we don't update the indicator state */
    message->service=indicator_ignore;
    message->signal=indicator_ignore;
    message->roam=indicator_ignore;
    message->battchg=indicator_ignore;
    
    AGHFP_DEBUG(("aghfpHandleBiaParse - Parsing\n"));
    
    for(i=0; i<indicators.length; i++)
    {
        AGHFP_DEBUG(("%c", (char)indicators.data[i]));
        if(indicators.data[i] == ',' || indicators.data[i] == ';')
        {
            /* If this is a comma move on (break if more indicators specified than we support) */
            AGHFP_DEBUG(("+ "));
            if(++indicator_idx > 6)
                break;
        }
        else if(indicators.data[i] == ' ' || indicators.data[i] =='\t')
        {
            /* Ignore whitespace */
            AGHFP_DEBUG(("- "));
        }
        else if(indicators.data[i] == '0' || indicators.data[i] == '1')
        {
            AGHFP_DEBUG(("m "));
            
            /* Matched a valid value */
            switch(indicator_idx)
            {
                /* This corresponds to the order we list our indicators to the HF */
                case 0:
                    message->service = indicators.data[i]-0x30;
                break;
                case 4:
                    message->signal = indicators.data[i]-0x30;
                break;
                case 5:
                    message->roam = indicators.data[i]-0x30;
                break;
                case 6:
                    message->battchg = indicators.data[i]-0x30;
                break;
                default:
                    /* Ignore call, call_setup and call_held - always on */
                break;
            }
        }
        else
        {
            /* Improperly formatted command - ERROR */
            success = FALSE;
            break;
        }
    }
    
    if(success)
    {
        AGHFP_DEBUG(("\nOK\n"));
        MessageSend(task, AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ, message);
    }
    else
    {
        AGHFP_DEBUG(("\nERROR\n"));
        free(message);
        aghfpSendError((AGHFP *)task);
    }
}

/****************************************************************************
NAME	
	aghfpHandleBindParse

DESCRIPTION
	Handles reception of the AT+BIND=<a>, <b>...<n> command from the HF. It parses the data
	to find the HF Indicators UUID16s and translates them to a bitmap form for internal use.

RETURNS
	void
*/
void aghfpHandleBindParse(Task task, const struct aghfpHandleBindParse *hf_indicators)
{
    /* Immediately extract the data. Once we return from this
	 function the data is lost, so parse now. */
	uint16 num_of_entries = hf_indicators->indicators.count;
	uint16 counter;
	struct value_aghfpHandleBindParse_indicators value;

	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_BIND_REQ);
	
	AGHFP_DEBUG(("aghfpHandleBindParse\n"));

	if(num_of_entries > AGHFP_MAX_HF_INDICATORS)
	{
		AGHFP_DEBUG(("Warning HF specified more hf indicators than defined in HFP Assigned Number. Only considering the first %d\n", AGHFP_MAX_HF_INDICATORS));
		num_of_entries = AGHFP_MAX_HF_INDICATORS;
	}

	message->num_hf_indicators = num_of_entries;

	for (counter = 0; counter < num_of_entries; counter++)
	{
		/* Get the next HF Indicator ID. */
		value = get_aghfpHandleBindParse_indicators(&hf_indicators->indicators, counter);

		message->hf_indicators_uuids[counter] = value.indicator;
	}

	MessageSend(task, AGHFP_INTERNAL_AT_BIND_REQ, message);
}

/****************************************************************************
NAME	
	aghfpHandleBindSupportRequestParse

DESCRIPTION
	Handles reception of the AT+BIND=? command from the HF. 

RETURNS
	void
*/
void aghfpHandleBindSupportRequestParse(Task task)
{
    MessageSend(task, AGHFP_INTERNAL_AT_BIND_SUPPORTED_REQ, NULL);
}

/****************************************************************************
NAME	
	aghfpHandleBindStatusRequestParse

DESCRIPTION
	Handles reception of the AT+BIND? command from the HF. 

RETURNS
	void
*/
void aghfpHandleBindStatusRequestParse(Task task)
{
     MessageSend(task, AGHFP_INTERNAL_AT_BIND_INDICATOR_STATUS_REQ, NULL);
}

/****************************************************************************
NAME	
	aghfpHandleBievParse

DESCRIPTION
	Handles reception of the AT+BIEV=<assigned num>, <value> from the HF. 

RETURNS
	void
*/
void aghfpHandleBievParse(Task task, const struct aghfpHandleBievParse *feature)
{
    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_BIEV_STATUS_REQ) ;    
  
    message->hf_indicator_assigned_num 	= feature->assignedNum;
    message->value = feature->value;        
    
    MessageSend(task , AGHFP_INTERNAL_AT_BIEV_STATUS_REQ , message ) ;
}

void aghfpHandleWbsCodecNegotiation(Task task, const struct aghfpHandleWbsCodecNegotiation *codec)
{
	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ);
	message->codec_id = codec->codec;
	MessageSend(task, AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ, message);
}

void aghfpQceCodecSelectParse(Task task, const struct aghfpQceCodecSelectParse *codec)
{
    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_QCS_REQ);
    message->codec_mode_id = codec->codec;
    MessageSend(task, AGHFP_INTERNAL_AT_QCS_REQ, message);
}

void aghfpHandleWbsCodecConnection(Task task)
{
	/* We've received an (AT+BCC) from the HF. We need to get audio parameters from the app. */
	MessageSend(task, AGHFP_INTERNAL_WBS_CODEC_CONNECTION_REQ, NULL);
}

/****************************************************************************
NAME	
	aghfpHandleAvailableCodecs

DESCRIPTION
	Handles reception of the AT+BAC= command from the HF. It parses the data
	to find the codec UUID16s and translates them to a bitmap form for internal use.

RETURNS
	void
*/

void aghfpHandleAvailableCodecs(Task task, const struct aghfpHandleAvailableCodecs *availableCodecs)
{
	/* Immediately extract the data. Once we return from this
	   function the data is lost, so parse now. */
	uint16 num_of_entries = availableCodecs->codecs.count;
	uint16 counter1;
	struct value_aghfpHandleAvailableCodecs_codecs value;

	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ);
	
	AGHFP_DEBUG(("aghfpHandleCodecNegotiationReq\n"));

	if(num_of_entries > AGHFP_MAX_NUM_CODECS)
	{
		AGHFP_DEBUG(("Warning HF specified more codecs than AG can handle. Only considering the first %d\n", AGHFP_MAX_NUM_CODECS));
		num_of_entries = AGHFP_MAX_NUM_CODECS;
	}

	message->num_codecs = num_of_entries;

	for (counter1 = 0; counter1 < num_of_entries; counter1++)
	{
		/* Get the next Codec ID. */
		value = get_aghfpHandleAvailableCodecs_codecs(&availableCodecs->codecs, counter1);

		message->codec_uuids[counter1] = value.codec;
	}

	MessageSend(task, AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ, message);
}


/****************************************************************************
NAME	
	aghfpHandleQacReqParse

DESCRIPTION
        Handles reception of the AT+QAC= command from the HF. It parses the data
	to find the codec mode values and translates them to a bitmap form for 
        internal use.

        NOTE: Cut & paste of aghfpHandleAvailableCodecs().
RETURNS
	void
*/

void aghfpHandleQacReqParse(Task task, const struct aghfpHandleQacReqParse *CodecModelist)
{
    /* Immediately extract the data. Once we return from this
       function the data is lost, so parse now. */
    uint16 num_of_entries = CodecModelist->codecs.count;
    uint16 idx;
    struct value_aghfpHandleQacReqParse_codecs codec;

    MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_AT_QAC_REQ);

    AGHFP_DEBUG(("aghfpHandleQacReqParse\n"));

    if(num_of_entries > QCE_MAX_NUM_CODECS)
    {
        AGHFP_DEBUG(("Warning HF specified more QCE codecs than AG can handle. Only considering the first %d\n", QCE_MAX_NUM_CODECS));
        num_of_entries = QCE_MAX_NUM_CODECS;
    }

    /* Convert the Codec Mode IDs (uint16 values) into a bit map for internal 
     * use. 
     */
    message->codec_mode_ids = 0;
    for (idx = 0; idx < num_of_entries; idx++)
    {
        /* Get the next Codec ID. */
        codec = get_aghfpHandleQacReqParse_codecs(&CodecModelist->codecs, idx);

        message->codec_mode_ids |= aghfpConvertCodecModeIdToBit(codec.codec);
    }

    MessageSend(task, AGHFP_INTERNAL_AT_QAC_REQ, message);
}
