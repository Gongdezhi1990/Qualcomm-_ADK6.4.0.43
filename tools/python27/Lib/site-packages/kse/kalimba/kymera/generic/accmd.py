'''
Audio Subsystem Command Interpreter Generic/Common library
'''

import logging
from functools import partial

from kats.framework.library.log import log_input, log_output, log_input_output, log_exception

SYSTEM_BUS_HOST_SYS = 1

ACCMD_CMD_ID_REQUEST_RESPONSE = 0x0000
ACCMD_CMD_ID_RESPONSE = 0x2000
ACCMD_CMD_ID_INDICATION = 0x4000
ACCMD_CMD_ID_RESERVED = 0x8000

ACCMD_CMD_ID_NO_CMD_REQ = 0x0000
ACCMD_CMD_ID_STREAM_GET_SOURCE_REQ = 0x0001
ACCMD_CMD_ID_STREAM_GET_SINK_REQ = 0x0002
ACCMD_CMD_ID_STREAM_CLOSE_SOURCE_REQ = 0x0003
ACCMD_CMD_ID_STREAM_CLOSE_SINK_REQ = 0x0004
ACCMD_CMD_ID_STREAM_CONFIGURE_REQ = 0x0005
ACCMD_CMD_ID_STREAM_SYNC_SID_REQ = 0x0006
ACCMD_CMD_ID_STREAM_CONNECT_REQ = 0x0007
ACCMD_CMD_ID_STREAM_TRANSFORM_DISCONNECT_REQ = 0x0008
ACCMD_CMD_ID_GET_FIRMWARE_VERSION_REQ = 0x0009
ACCMD_CMD_ID_CREATE_OPERATOR_REQ = 0x000a
ACCMD_CMD_ID_DESTROY_OPERATORS_REQ = 0x000b
ACCMD_CMD_ID_OPERATOR_MESSAGE_REQ = 0x000c
ACCMD_CMD_ID_START_OPERATORS_REQ = 0x000d
ACCMD_CMD_ID_STOP_OPERATORS_REQ = 0x000e
ACCMD_CMD_ID_RESET_OPERATORS_REQ = 0x000f
ACCMD_CMD_ID_MESSAGE_FROM_OPERATOR_REQ = 0x0010
ACCMD_CMD_ID_GET_FIRMWARE_ID_STRING_REQ = 0x0011
ACCMD_CMD_ID_PRODTEST_REQ = 0x0012
ACCMD_CMD_ID_STREAM_GET_INFO_REQ = 0x0013
ACCMD_CMD_ID_STREAM_GET_CAPABILITIES_REQ = 0x0014
ACCMD_CMD_ID_STREAM_EXTERNAL_DETACH_REQ = 0x0015
ACCMD_CMD_ID_STREAM_SOURCE_DRAINED_REQ = 0x0016
ACCMD_CMD_ID_CREATE_OPERATOR_EX_REQ = 0x0017
ACCMD_CMD_ID_GET_MEM_USAGE_REQ = 0x0018
ACCMD_CMD_ID_CLEAR_MEM_WATERMARKS_REQ = 0x0019
ACCMD_CMD_ID_GET_MIPS_USAGE_REQ = 0x001a
# old command ACCMD_CMD_ID_SET_SYSTEM_STREAM_RATE_REQ = 0x001b
# old command ACCMD_CMD_ID_GET_SYSTEM_STREAM_RATE_REQ = 0x001c
ACCMD_CMD_ID_STREAM_DISCONNECT_REQ = 0x001d
ACCMD_CMD_ID_STREAM_GET_SINK_FROM_SOURCE_REQ = 0x001e
ACCMD_CMD_ID_STREAM_GET_SOURCE_FROM_SINK_REQ = 0x001f
ACCMD_CMD_ID_CAP_DOWNLOAD_REMOVE_KCS_REQ = 0x0020
ACCMD_CMD_ID_SET_SYSTEM_FRAMEWORK_REQ = 0x0021
ACCMD_CMD_ID_GET_SYSTEM_FRAMEWORK_REQ = 0x0022
ACCMD_CMD_ID_UNSOLICITED_SYSTEM_FRAMEWORK_REQ = 0x0023
ACCMD_CMD_ID_MIC_BIAS_CONFIGURE_REQ = 0x0024
ACCMD_CMD_ID_CODEC_SET_IIR_FILTER_REQ = 0x0025
ACCMD_CMD_ID_CODEC_SET_IIR_FILTER_16BIT_REQ = 0x0026
ACCMD_CMD_ID_AUX_PROCESSOR_LOAD_REQ = 0x0027
ACCMD_CMD_ID_AUX_PROCESSOR_UNLOAD_REQ = 0x0028
ACCMD_CMD_ID_AUX_PROCESSOR_START_REQ = 0x0029
ACCMD_CMD_ID_AUX_PROCESSOR_STOP_REQ = 0x002a
ACCMD_CMD_ID_STREAM_MCLK_ACTIVATE_REQ = 0x002b
ACCMD_CMD_ID_SET_MCLK_SOURCE_TYPE_REQ = 0x002c
ACCMD_CMD_ID_GET_OPERATORS_REQ = 0x002d
ACCMD_CMD_ID_PS_SHUTDOWN_REQ = 0x002e
ACCMD_CMD_ID_PS_READ_REQ = 0x002f
ACCMD_CMD_ID_PS_WRITE_REQ = 0x0030
ACCMD_CMD_ID_PS_DELETE_REQ = 0x0031
ACCMD_CMD_ID_PS_SHUTDOWN_COMPLETE_REQ = 0x0032
ACCMD_CMD_ID_PS_REGISTER_REQ = 0x0033
ACCMD_CMD_ID_SET_PROFILER_REQ = 0x0034
ACCMD_CMD_ID_CLOCK_CONFIGURE_REQ = 0x0035
ACCMD_CMD_ID_SET_ANC_IIR_FILTER_REQ = 0x0036
ACCMD_CMD_ID_SET_ANC_LPF_FILTER_REQ = 0x0037
ACCMD_CMD_ID_STREAM_ANC_ENABLE_REQ = 0x0038
ACCMD_CMD_ID_STREAM_SIDETONE_EN_REQ = 0x0039
ACCMD_CMD_ID_DATA_FILE_ALLOCATE_REQ = 0x003a
ACCMD_CMD_ID_DATA_FILE_DEALLOCATE_REQ = 0x003b
ACCMD_CMD_ID_CLOCK_POWER_SAVE_MODE_REQ = 0x003c
ACCMD_CMD_ID_CAP_DOWNLOAD_ADD_KCS_REQ = 0x003d
ACCMD_CMD_ID_CAP_DOWNLOAD_COMPLETE_KCS_REQ = 0x003e
ACCMD_CMD_ID_AUX_PROCESSOR_DISABLE_REQ = 0x003f
ACCMD_CMD_ID_TRANSFORM_FROM_STREAM_REQ = 0x0040
ACCMD_CMD_ID_FILE_DETAILS_REQ = 0x0041
ACCMD_CMD_ID_ACQUIRE_FLASH_LOCK_REQ = 0x0042
ACCMD_CMD_ID_RELEASE_FLASH_LOCK_REQ = 0x0043

ACCMD_CMD_ID_NO_CMD_RESP = 0x4000
ACCMD_CMD_ID_STREAM_GET_SOURCE_RESP = 0x4001
ACCMD_CMD_ID_STREAM_GET_SINK_RESP = 0x4002
ACCMD_CMD_ID_STANDARD_RESP = 0x4003
ACCMD_CMD_ID_STREAM_CONNECT_RESP = 0x4004
ACCMD_CMD_ID_GET_FIRMWARE_VERSION_RESP = 0x4005
ACCMD_CMD_ID_CREATE_OPERATOR_RESP = 0x4006
ACCMD_CMD_ID_DESTROY_OPERATORS_RESP = 0x4007
ACCMD_CMD_ID_OPERATOR_MESSAGE_RESP = 0x4008
ACCMD_CMD_ID_START_OPERATORS_RESP = 0x4009
ACCMD_CMD_ID_STOP_OPERATORS_RESP = 0x400a
ACCMD_CMD_ID_RESET_OPERATORS_RESP = 0x400b
ACCMD_CMD_ID_GET_FIRMWARE_ID_STRING_RESP = 0x400c
ACCMD_CMD_ID_PRODTEST_RESP = 0x400d
ACCMD_CMD_ID_STREAM_GET_INFO_RESP = 0x400e
ACCMD_CMD_ID_STREAM_GET_CAPABILITIES_RESP = 0x400f
ACCMD_CMD_ID_CREATE_OPERATOR_EX_RESP = 0x4010
ACCMD_CMD_ID_GET_MEM_USAGE_RESP = 0x4011
ACCMD_CMD_ID_CLEAR_MEM_WATERMARKS_RESP = 0x4012
ACCMD_CMD_ID_GET_MIPS_USAGE_RESP = 0x4013
# old command ACCMD_CMD_ID_SET_SYSTEM_STREAM_RATE_RESP = 0x4014
# old command ACCMD_CMD_ID_GET_SYSTEM_STREAM_RATE_RESP = 0x4015
ACCMD_CMD_ID_STREAM_DISCONNECT_RESP = 0x4016
ACCMD_CMD_ID_STREAM_GET_SINK_FROM_SOURCE_RESP = 0x4017
ACCMD_CMD_ID_STREAM_GET_SOURCE_FROM_SINK_RESP = 0x4018
ACCMD_CMD_ID_SET_SYSTEM_FRAMEWORK_RESP = 0x4019
ACCMD_CMD_ID_GET_SYSTEM_FRAMEWORK_RESP = 0x401a
ACCMD_CMD_ID_GET_IPC_INTERFACE_TEST_RESP = 0x401b
ACCMD_CMD_ID_GET_OPERATORS_RESP = 0x401c
ACCMD_CMD_ID_PS_SHUTDOWN_RESP = 0x401d  # apparently not used
ACCMD_CMD_ID_PS_READ_RESP = 0x401e
ACCMD_CMD_ID_PS_WRITE_RESP = 0x401f
ACCMD_CMD_ID_PS_DELETE_RESP = 0x4020
ACCMD_CMD_ID_PS_SHUTDOWN_COMPLETE_RESP = 0x4021
ACCMD_CMD_ID_TRANSFORM_FROM_STREAM_RESP = 0x4022

ACCMD_CMD_ID_SET_PROFILER_RESP = 0x4034
ACCMD_CMD_ID_CLOCK_CONFIGURE_RESP = 0x4035

ACCMD_CMD_ID_DATA_FILE_ALLOCATE_RESP = 0x403a

ACCMD_CMD_ID_CLOCK_POWER_SAVE_MODE_RESP = 0x403c
ACCMD_CMD_ID_FILE_DETAILS_RESP = 0x403d
ACCMD_CMD_ID_FLASH_LOCK_RESP = 0x403e

ACCMD_RESP_STATUS_OK = 0x0000
ACCMD_RESP_STATUS_NO_SUCH_CMDID = 0x0001
ACCMD_RESP_STATUS_BAD_REQ = 0x0002
ACCMD_RESP_STATUS_ACCMD_QUEUE_FULL = 0x0003
ACCMD_RESP_STATUS_ACCMD_CMD_PENDING = 0x0004
ACCMD_RESP_STATUS_CMD_FAILED = 0x0005
ACCMD_RESP_STATUS_NO_SUCH_SUB_CMD = 0x0006

ACCMD_RESP_STATUS_STR = {
    ACCMD_RESP_STATUS_OK: 'ok',
    ACCMD_RESP_STATUS_NO_SUCH_CMDID: 'no such command id',
    ACCMD_RESP_STATUS_BAD_REQ: 'bad request',
    ACCMD_RESP_STATUS_ACCMD_QUEUE_FULL: 'accmd queue full',
    ACCMD_RESP_STATUS_ACCMD_CMD_PENDING: 'command pending',
    ACCMD_RESP_STATUS_CMD_FAILED: 'command failed',
    ACCMD_RESP_STATUS_NO_SUCH_SUB_CMD: 'no such subcommand'
}

ENDPOINT_TYPE_DICT = {
    'pcm': 0x0001,
    'i2s': 0x0002,
    'codec': 0x0003,
    'fm': 0x0004,
    'spdif': 0x0005,
    'digital mic': 0x0006,
    'shunt': 0x0007,
    'sco': 0x0009,
    'operator': 0x001e,
}

STREAM_KEY_TYPE_DICT = {
    'pcm_sync_rate': 0x0100,
    'pcm_master_clock_rate': 0x0101,
    'pcm_master_mode': 0x0102,
    'pcm_slot_count': 0x0103,
    'pcm_manchester_mode': 0x0104,
    'pcm_short_sync_mode': 0x0105,
    'pcm_manchester_slave_mode': 0x0106,
    'pcm_sign_extend_mode': 0x0107,
    'pcm_lsb_first_mode': 0x0108,
    'pcm_tx_tristate_mode': 0x0109,
    'pcm_tx_tristate_rising_edge_mode': 0x010a,
    'pcm_sync_suppress_enable': 0x010b,
    'pcm_gci_mode': 0x010c,
    'pcm_mute_enable': 0x010d,
    'pcm_long_length_sync': 0x010e,
    'pcm_sample_rising_edge': 0x010f,
    'pcm_rx_rate_delay': 0x0113,
    'pcm_sample_format': 0x0114,
    'pcm_manchester_mode_rx_offset': 0x0115,
    'pcm_audio_gain': 0x0116,

    'i2s_sync_rate': 0x0200,
    'i2s_master_clock_rate': 0x0201,
    'i2s_master_mode': 0x0202,
    'i2s_justify_mode': 0x0203,
    'i2s_left_justify_delay': 0x0204,
    'i2s_channel_polarity': 0x0205,
    'i2s_audio_attenuation_enable': 0x0206,
    'i2s_audio_attenuation': 0x0207,
    'i2s_justify_resolution': 0x0208,
    'i2s_crop_enable': 0x0209,
    'i2s_bits_per_sample': 0x020a,
    'i2s_tx_start_sample': 0x020b,
    'i2s_rx_start_sample': 0x020c,

    # available to all interfaces
    'audio_channel_mute_enable': 0x0700,
    'audio_sample_size': 0x0701,

    'shunt_l2cap_atu': 0x0800,  # sink
    'shunt_autoflush_enable': 0x0801,  # sink
    'shunt_tokens_allocated': 0x0802,  # sink
    'shunt_max_pdu_length': 0x0803,  # source
    'shunt_notify_when_drained': 0x0804,  # source
}

STREAM_INFO_DICT = {
    'audio_sample_rate': 0x0000,
}

CREATE_OPERATOR_EX_DICT = {
    'priority': 0x0001,
    'processor_id': 0x0002,
}

FRAMEWORK_KEY_DICT = {
    # acknowledgement of an imminent shutdown
    'shutdown_prepare': 0x0001,
    # getting the build ID integer.
    'build_id': 0x0002,
    # getting the build ID string.
    'build_id_string': 0x0003,
    # getting and setting the stream rate.
    'stream_rate': 0x0004,
    # When set to TRUE, the firmware calls panic() rather than fault(). See PanicOnFault MIB entry.
    'panic_on_fault': 0x0005,
    # Enables/Disables the built-in profiler.
    'profiler_enable': 0x0006,
    # Used for getting and setting system kick period. This should only be set on system startup.
    # Changing the kick period with a running processing graph may have unpredictable results
    'kick_period': 0x0007,
}


class Accmd(object):
    '''
    Accmd generic protocol

    This class handles all accmd messages

    Args:
        uut (kats.kalimba.uut.uut_base.UutBase): Uut instance
    '''

    def __init__(self, uut, subsystem=SYSTEM_BUS_HOST_SYS):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.debug('init')

        self._subsystem = subsystem
        self._sequence_num = 0
        self._uut = uut
        self._cb = []

        header = [subsystem]  # FIXME specific for hydra
        self._header = self._uut.message_register_handler(header, self._message_received)

    def _message_received(self, msg):
        try:
            unsolicited_msgs = [ACCMD_CMD_ID_MESSAGE_FROM_OPERATOR_REQ,
                                ACCMD_CMD_ID_UNSOLICITED_SYSTEM_FRAMEWORK_REQ]

            cmd_id, seq_no, payload = self.receive(msg)
            if cmd_id in unsolicited_msgs:
                self.send(ACCMD_CMD_ID_STANDARD_RESP, [ACCMD_RESP_STATUS_OK], sequence_num=seq_no)
        except Exception:  # pylint: disable=broad-except
            pass

        for cb in self._cb:
            cb(msg)

    def _get_sequence_num(self):
        self._sequence_num = self._sequence_num + 1 \
            if self._sequence_num < 255 else 0
        return self._sequence_num

    def _format_message(self, cmd_id, payload=None, sequence_num=None):
        self._log.debug('format ACCMD with id:%s and payload:%s', id, payload)
        if sequence_num is None:
            sequence_num = self._get_sequence_num()
        if not payload:
            payload = []
        payload_list = [cmd_id, len(payload) * 2, sequence_num] + payload
        return payload_list

    @staticmethod
    def _check_value(dict_data, value):
        return value in dict_data or value in dict_data.values()

    @staticmethod
    def _get_value(dict_data, value):
        if value in dict_data:
            return dict_data[value]
        else:
            return value

    @staticmethod
    def _get_status_descr(status):
        return ACCMD_RESP_STATUS_STR.get(status, 'unknown')

    @staticmethod
    def _check_standard_resp(command, cmd_id, payload):
        if cmd_id != ACCMD_CMD_ID_STANDARD_RESP:
            raise RuntimeError('%s response cmd_id:%04x invalid' % (command, cmd_id))
        if len(payload) != 0:
            raise RuntimeError('%s response length:%s invalid' % (command, len(payload)))

    @log_input_output(logging.DEBUG, logging.DEBUG, formatters={'payload': '0x%04x'})
    @log_exception
    def receive(self, payload):
        if len(payload) < 3:
            raise ValueError('response size:%s less than minimum' % (len(payload)))
        cmd_id = payload[0]
        length = payload[1]
        seq_no = payload[2]
        if length % 2:
            raise ValueError('header length:%s should be even' % (length))
        if length != 2 * (len(payload) - 3):
            raise ValueError('header length:%s and length:%s do not match' % (length, len(payload)))
        return cmd_id, seq_no, payload[3:]

    @log_input(logging.INFO)
    def register_receive_callback(self, func):
        self._cb.append(func)

    @log_input(logging.INFO)
    def unregister_receive_callback(self, func):
        self._cb.remove(func)

    @log_input(logging.DEBUG, formatters={'cmd_id': '0x%04x', 'payload': '0x%04x'})
    def send(self, cmd_id, payload, sequence_num=None):
        '''
        Send command message with optional payload

        Args:
            cmd_id (int): Command id
            payload (list[int]): Message parameters
            sequence_num (int): Sequence number or None for next
        '''
        payload = self._format_message(cmd_id, payload if payload else [],
                                       sequence_num=sequence_num)
        self._uut.message_send(self._header, payload)

    @log_input_output(logging.DEBUG, logging.DEBUG,
                      formatters={'cmd_id': '0x%04x', 'payload': '0x%04x'})
    @log_exception
    def send_recv(self, cmd_id, payload=None):
        '''
        Send command message with optional payload and receive response

        Args:
            cmd_id (int): Command id
            payload (list[int]): Message parameters

        Returns:
            tuple[int, list[int]]: Response command (response) id, Response message
        '''

        def filter_by_sequence(seq, payload):
            if not (len(payload) >= 3 and seq == payload[2]):
                self._log.info('filtering out message seq:%s payload:%s', seq, str(payload))
            return len(payload) >= 3 and seq == payload[2]

        payload = self._format_message(cmd_id, payload if payload else [])
        payload = self._uut.message_send_recv(self._header, payload,
                                              flt=partial(filter_by_sequence, self._sequence_num))
        cmd_id, seq_no, payload = self.receive(payload)
        if seq_no != self._sequence_num:
            raise ValueError('response seq_no:0x%02x does not match command:0x%02x' %
                             (seq_no, self._sequence_num))
        if len(payload) == 0:
            raise RuntimeError('response empty response received')
        if payload[0] != ACCMD_RESP_STATUS_OK:
            raise RuntimeError('response status:0x%02x invalid (%s)' %
                               (payload[0], self._get_status_descr(payload[0])))
        return cmd_id, payload[1:]

    # ==============================================================================================
    # audio stream commands
    # ==============================================================================================

    @log_input(logging.INFO)
    @log_exception
    def no_cmd(self):
        '''
        Standard ACCMD that sends no parameters and performs no actions.
        This command is provided to enable verification of the ACCMD service
        '''
        # note status is not coming in this response
        payload = self._format_message(ACCMD_CMD_ID_NO_CMD_REQ, [])
        payload = self._uut.message_send_recv(self._header, payload)
        cmd_id, seq_no, payload = self.receive(payload)
        if seq_no != self._sequence_num:
            raise ValueError('no_cmd_req seq_no:0x%02x does not match command:0x%02x' %
                             (seq_no, self._sequence_num))
        if len(payload) != 0:
            raise RuntimeError('no_cmd_req empty response received')
        if cmd_id != ACCMD_CMD_ID_NO_CMD_RESP:
            raise RuntimeError('no_cmd_req response cmd_id:%04x invalid' % (cmd_id))

    @log_input_output(logging.INFO, logging.INFO, formatters={'return': '0x%04x'})
    @log_exception
    def stream_get_source(self, endpoint_type, opt1, opt2):
        '''
        Obtain use of the specified resource

        If an attempt is made to reserve a resource that has already been successfully reserved
            - The request will succeed but return the source id supplied in response to the
              original request.
            - In the case of Operator; The command is used to obtain the source id for use in
              stream_connect_req.

        Attempting to reserve a resource may fail if:
            - The resource does not exist.
            - The resource cannot be reserved because another resource
              (that shares some aspect of its hardware), has already been reserved.
            - There are insufficient internal resources to support the requested resource

        Args:
            endpoint_type (str or int): Endpoint type, see (ENDPOINT_TYPE_DICT)
            opt1 (int): Optional parameter 1, its meaning depends on endpoint_type
            opt2 (int): Optional parameter 2, its meaning depends on endpoint_type

        Returns:
            int: source endpoint id
        '''
        endpoint = self._get_value(ENDPOINT_TYPE_DICT, endpoint_type)
        if not isinstance(endpoint, int):
            raise ValueError('stream_get_source endpoint_type:%s invalid' % (endpoint_type))
        if not isinstance(opt1, int):
            raise ValueError('stream_get_source opt1:%s invalid' % (opt1))
        if not isinstance(opt2, int):
            raise ValueError('stream_get_source opt2:%s invalid' % (opt2))
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_GET_SOURCE_REQ, [endpoint, opt1, opt2])
        if cmd_id != ACCMD_CMD_ID_STREAM_GET_SOURCE_RESP:
            raise RuntimeError('stream_get_source response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('stream_get_source response length:%s invalid' % (len(payload)))
        return payload[0]

    @log_input_output(logging.INFO, logging.INFO, formatters={'return': '0x%04x'})
    @log_exception
    def stream_get_sink(self, endpoint_type, opt1, opt2):
        '''
        Obtain use of the specified resource

        If an attempt is made to reserve a resource that has already been successfully reserved
            - The request will succeed but return the sink id supplied in response to the
              original request.
            - In the case of Operator; The command is used to obtain the sink id for use in
              stream_connect_req.

        Attempting to reserve a resource may fail if:
            - The resource does not exist.
            - The resource cannot be reserved because another resource
              (that shares some aspect of its hardware), has already been reserved.
            - There are insufficient internal resources to support the requested resource

        Args:
            endpoint_type (str or int): Endpoint type, see (ENDPOINT_TYPE_DICT)
            opt1 (int): Optional parameter 1, its meaning depends on endpoint_type
            opt2 (int): Optional parameter 2, its meaning depends on endpoint_type

        Returns:
            int: Source endpoint id
        '''
        endpoint = self._get_value(ENDPOINT_TYPE_DICT, endpoint_type)
        if not isinstance(endpoint, int):
            raise ValueError('stream_get_sink endpoint_type:%s invalid' % (endpoint_type))
        if not isinstance(opt1, int):
            raise ValueError('stream_get_sink opt1:%s invalid' % (opt1))
        if not isinstance(opt2, int):
            raise ValueError('stream_get_sink opt2:%s invalid' % (opt2))
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_GET_SINK_REQ, [endpoint, opt1, opt2])
        if cmd_id != ACCMD_CMD_ID_STREAM_GET_SINK_RESP:
            raise RuntimeError('stream_get_sink response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('stream_get_sink response length:%s invalid' % (len(payload)))
        return payload[0]

    @log_input(logging.INFO, formatters={'source_id': '0x%04x'})
    @log_exception
    def stream_close_source(self, source_id):
        '''
        Release the resource currently held by the specified source id

        Once released, the associated resource id becomes invalid and should be discarded.
        If the command is issued for a source id active in a transform, the transform is
        automatically disconnected, as part of the command.

        The command will fail if:
            - An unrecognised source id is specified, or
            - An attempt is made to close a source that cannot be closed in this way; such as an
              SCO source (Which is closed automatically when the SCO link is closed over Bluetooth
              HCI or by the remote peer.).

        Args:
            source_id (int): Source id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_CLOSE_SOURCE_REQ, [source_id])
        self._check_standard_resp('stream_close_source', cmd_id, payload)

    @log_input(logging.INFO, formatters={'sink_id': '0x%04x'})
    @log_exception
    def stream_close_sink(self, sink_id):
        '''
        Release the resource currently held by the specified sink id

        Once released, the associated resource id becomes invalid and should be discarded.
        If the command is issued for a source id active in a transform, the transform is
        automatically disconnected, as part of the command.

        The command will fail if:
            - An unrecognised source id is specified, or
            - An attempt is made to close a source that cannot be closed in this way; such as an
              SCO source (Which is closed automatically when the SCO link is closed over Bluetooth
              HCI or by the remote peer.).

        Args:
            sink_id (int): Sink id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_CLOSE_SINK_REQ, [sink_id])
        self._check_standard_resp('stream_close_sink', cmd_id, payload)

    @log_input(logging.INFO, formatters={'sid': '0x%04x'})
    @log_exception
    def stream_configure(self, sid, key, value):
        '''
        Configure a single property of the specified source or sink id

        Any key value configured using stream_configure_req will persist for as long as the service
        instance exists.

        It is not neccessary to apply multiple key values for digital interfaces (PCM, I2s or SPDIF)
        Only apply Key values to a single source or sink id for each interface.

        Args:
            sid (int): source or sink id
            key (str or int): Property to be configured
            value (int): Value to be assigned to the key
        '''
        key_val = self._get_value(STREAM_KEY_TYPE_DICT, key)
        if not isinstance(key_val, int):
            raise ValueError('stream_configure key:%s invalid' % (key))
        if not isinstance(value, int):
            raise ValueError('stream_configure key:%s val:%s invalid' % (key, value))
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_CONFIGURE_REQ,
                                         [sid,
                                          key_val,
                                          value & 0xFFFF,
                                          (value >> 16) & 0xFFFF])
        self._check_standard_resp('stream_configure', cmd_id, payload)

    @log_input(logging.INFO, formatters={'sid1': '0x%04x', 'sid2': '0x%04x'})
    @log_exception
    def stream_sync_sid(self, sid1, sid2):
        '''
        Request that to specified endpoints to be synchronised, such as between the two channels
        of a stereo source. Synchronisation is used to prevent timing mis-match and drift between
        related sources or sinks.

        Sync group
        If two Endpoints are synchronised using stream_sync_sid_req they are considered to be in the
        same "Sync Group".
        In this case, all source or sink ids within a particular Sync Group are enabled
        simultaneously (All stream_connect_req commands are deferred until the last has been
        issued.)

        A Sync Group containing more than two source or sink ids can be created using multiple
        stream_sync_sid ACCMDs.
        For example; the following sequence will create a Sync Group consisting of the four source
        or sink ids; a, b, c and d:

        - stream_sync_sid_req a b
        - stream_sync_sid_req c d
        - stream_sync_sid_req a c

        Source or sink ids closed using stream_close_source or stream_close_sink are automatically
        removed from any existing Sync Group.

        Args:
            sid1 (int): source or sink id
            sid2 (int): source or sink id, if 0 sid1 will be removed frm any existing Sync Group
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_SYNC_SID_REQ, [sid1, sid2])
        self._check_standard_resp('stream_sync_sid', cmd_id, payload)

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'source_id': '0x%04x', 'sink_id': '0x%04x', 'return': '0x%04x'})
    @log_exception
    def stream_connect(self, source_id, sink_id):
        '''
        Connect the specified source and sind id to create a transform

        Source and sink ids are obtained using:
        - stream_get_source command
        - stream_get_sink command

        Operator source and sink ids are derived from:
        - The Operator ID
        - The Connection Number on the Operator (Which starts from 0)

        Values for Source and Sink ID may be calculated using:
        - Source = op id + connection + 0x2000
        - Sink = op id + connection + 0xA000

        Args:
            source_id (int): Source id
            sink_id (int): Sink id

        Returns:
            int: Transform id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_CONNECT_REQ, [source_id, sink_id])
        if cmd_id != ACCMD_CMD_ID_STREAM_CONNECT_RESP:
            raise RuntimeError('stream_connect response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('stream_connect response length:%s invalid' % (len(payload)))
        return payload[0]

    @log_input(logging.INFO, formatters={'transform_id': '0x%04x'})
    @log_exception
    def stream_transform_disconnect(self, transform_id):
        '''
        Disconnect an existing transform formed using stream_connect

        Args:
            transform_id (int): Transform id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_TRANSFORM_DISCONNECT_REQ,
                                         [transform_id])
        self._check_standard_resp('stream_transform_disconnect', cmd_id, payload)

    @log_output(logging.INFO, formatters={'return': '0x%04x'})
    @log_exception
    def get_firmware_version(self):
        '''
        Request firmware Build id from the audio sub-system

        Returns:
            (int) Firmware ID
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_GET_FIRMWARE_VERSION_REQ)
        if cmd_id != ACCMD_CMD_ID_GET_FIRMWARE_VERSION_RESP:
            raise RuntimeError('get_firmware_version response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('get_firmware_version length:%s invalid' % (len(payload)))
        return payload[0]

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'cap_id': '0x%04x', 'return': '0x%04x'})
    @log_exception
    def create_operator(self, cap_id):
        '''
        Create an operator (an instance of a capability)

        Args:
            cap_id (int): Capability id

        Returns:
            int: Operator id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_CREATE_OPERATOR_REQ, [cap_id])
        if cmd_id != ACCMD_CMD_ID_CREATE_OPERATOR_RESP:
            raise RuntimeError('create_operator response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('create_operator response length:%s invalid' % (len(payload)))
        return payload[0]

    @log_input(logging.INFO, formatters={'operators': '0x%04x'})
    @log_exception
    def destroy_operators(self, operators):
        '''
        Destroy one or more existing operators and free any associated resources

        Args:
            operators (int): List of operator ids
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_DESTROY_OPERATORS_REQ,
                                         [len(operators)] + operators)
        if cmd_id != ACCMD_CMD_ID_DESTROY_OPERATORS_RESP:
            raise RuntimeError('destroy_operators response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 2:
            raise RuntimeError('destroy_operators length:%s invalid' % (len(payload)))

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'op_id': '0x%04x', 'msg': '0x%04x', 'return': '0x%04x'})
    @log_exception
    def operator_message(self, op_id, msg):
        '''
        Send an arbitrary message to a specific operator.

        The messages available and their meaning will depend upon the specific capability that
        the operator instantiates.

        Args:
            op_id (int): Operator id
            msg (list[int]): Message to be sent

        Returns:
            list[int]: Message received in response from the operator
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_OPERATOR_MESSAGE_REQ, [op_id] + msg)
        if cmd_id != ACCMD_CMD_ID_OPERATOR_MESSAGE_RESP:
            raise RuntimeError('operator_message response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) < 1:
            raise RuntimeError('operator_message response length:%s invalid' % (len(payload)))
        if payload[0] != op_id:
            raise RuntimeError('operator_message response op_id:%04x expecting:%04x' %
                               (payload[0], op_id))
        return payload[1:]

    @log_input(logging.INFO, formatters={'operators': '0x%04x'})
    @log_exception
    def start_operators(self, operators):
        '''
        Starts a set of operators processing data at the same time (or as close as it is possible)

        It is permitted to start an operator that has already been started.
        This command will have no effect and succeed.

        Attempting to start an operator that has nothing connected to it,
        or has insufficient connections, may fail (depending on the capability).

        Args:
            operators (list[int]): List of operator ids
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_START_OPERATORS_REQ,
                                         [len(operators)] + operators)
        if cmd_id != ACCMD_CMD_ID_START_OPERATORS_RESP:
            raise RuntimeError('start_operators response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 2:
            raise RuntimeError('start_operators length:%s invalid' % (len(payload)))

    @log_input(logging.INFO, formatters={'operators': '0x%04x'})
    @log_exception
    def stop_operators(self, operators):
        '''
        Stops a set of operators at the same time (or as close as it is possible)

        It is permitted to stop an operator that has never been started or has already been stopped.
        No attempt is made to stop subsequent operators on the list

        Args:
            operators (list[int]): List of operator ids
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STOP_OPERATORS_REQ,
                                         [len(operators)] + operators)
        if cmd_id != ACCMD_CMD_ID_STOP_OPERATORS_RESP:
            raise RuntimeError('stop_operators response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 2:
            raise RuntimeError('stop_operators length:%s invalid' % (len(payload)))

    @log_input(logging.INFO, formatters={'operators': '0x%04x'})
    @log_exception
    def reset_operators(self, operators):
        '''
        Reset a set of operators at the same time (or as close as it is possible)

        The exact meaning of reset is capability-dependent, but will always leave the operator
        stopped.

        An operator may be reset whether it is running or stopped

        Args:
            operators (list[int]): List of operator ids
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_RESET_OPERATORS_REQ,
                                         [len(operators)] + operators)
        if cmd_id != ACCMD_CMD_ID_RESET_OPERATORS_RESP:
            raise RuntimeError('reset_operators response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 2:
            raise RuntimeError('reset_operators length:%s invalid' % (len(payload)))

    @log_output(logging.INFO)
    @log_exception
    def get_firmware_id_string(self):
        '''
        Request firmware Build id string from the audio sub-system

        Returns:
            (str) Firmware ID
        '''
        # FIXME cs-308458-sp ACCMD Commands and Protocols (internal use only) is confusing
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_GET_FIRMWARE_ID_STRING_REQ)
        if cmd_id != ACCMD_CMD_ID_GET_FIRMWARE_ID_STRING_RESP:
            raise RuntimeError('get_firmware_id_string response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) < 1:
            raise RuntimeError('get_firmware_id_string length:%s invalid' % (len(payload)))
        if payload[0] != len(payload[1:]):
            raise RuntimeError('get_firmware_id_string response length:%s does not match message' %
                               (payload[0]))
        ret = ''
        for entry in payload[1:]:
            val = entry & 0xFF
            if val == 0:
                break
            ret += chr(val)
            val = (entry >> 8) & 0xFF
            if val == 0:
                break
            ret += chr(val)
        return ret

    # TODO ACCMD_CMD_ID_PRODTEST_REQ

    @log_input_output(logging.INFO, logging.INFO, formatters={'sid': '0x%04x'})
    @log_exception
    def stream_get_info(self, sid, key):
        '''
        Get a single property of the specified source or sink id

        Args:
            sid (int): Source or sink id
            key (str): Property

        Returns:
            int: Property value
        '''
        key_val = self._get_value(STREAM_INFO_DICT, key)
        if not isinstance(key_val, int):
            raise ValueError('stream_get_info key:%s invalid' % (key))
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_GET_INFO_REQ, [sid, key_val])
        if cmd_id != ACCMD_CMD_ID_STREAM_GET_INFO_RESP:
            raise RuntimeError('stream_get_info response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 2:
            raise RuntimeError('stream_get_info response length:%s invalid' % (len(payload)))
        return (payload[1] << 16) | payload[0]

    @log_input_output(logging.INFO, logging.INFO)
    @log_exception
    def get_capabilities(self, start=0):
        '''
        Request a list of ids for the available capabilities

        The request will include any in-built capabilities, as well as all those currently available
        for download, whether or not any operator instances currently exist.
        At most 40 capabilities will be reported

        Args:
            start (int): Start index

        Returns:
            tuple[int, list[int]]: Total number of available capabilities, capabilities
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_GET_CAPABILITIES_REQ, [start])
        if cmd_id != ACCMD_CMD_ID_STREAM_GET_CAPABILITIES_RESP:
            raise RuntimeError('get_capabilities response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) < 1:
            raise RuntimeError('get_capabilities response length:%s invalid' % (len(payload)))
        if payload[1] != len(payload[2:]):
            raise RuntimeError('get_capabilities response count:%d does not match data' %
                               (payload[1]))
        return payload[0], payload[2:]

    @log_input(logging.INFO, formatters={'source_id': '0x%04x'})
    @log_exception
    def stream_external_detach(self, source_id):
        '''
        Detach a shunt stream from its external source

        Once detached any further data received on the ACL handle and channel id are not forwarded
        on the shunt, however data already received remains accessinle.
        The shunt source must be explicitly closed using stream_close_source

        An l2cap (source) stream is automatically detached when the corresponding ACL link is
        detached or disconnected

        Args:
            source_id (int): Source id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_EXTERNAL_DETACH_REQ, [source_id])
        self._check_standard_resp('stream_external_detach', cmd_id, payload)

    @log_input(logging.INFO, formatters={'source_id': '0x%04x'})
    @log_exception
    def stream_source_drained(self, source_id):
        '''
        Is sent by the shunt endpoint whem the shunt buffer is rained of all data (currently only
        supported for shunt source stream).

        This event is generated if the shunt source has been configured to notify when the shunt
        buffer is fully drained and either:

        - A stream_external_detach command is use on the stream, but there is no data waiting
            in the stream
        - The stream is already detached and all data has been drained from the stream

        Args:
            source_id (int): Shunt source id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_SOURCE_DRAINED_REQ, [source_id])
        self._check_standard_resp('stream_source_drained', cmd_id, payload)

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'cap_id': '0x%04x', 'return': '0x%04x'})
    @log_exception
    def create_operator_ex(self, cap_id, *args, **kwargs):
        '''
        Create an operator, parameterised version (an instance of a capability)

        .. code-block:: python

            # Create operator from capability 1 with priority 1 in processor 0
            # All the following commands do the same thing
            create_operator_ex(1, [0x0001, 1], [0x0002, 0])
            create_operator_ex(1, [0x0001, 1], ['processor_id', 0])
            create_operator_ex(1, [0x0001, 1], processor_id=0)
            create_operator_ex(1, [0x0002, 0], priority=1)
            create_operator_ex(1, priority=1, processor_id=0)

        Args:
            cap_id (int): Capability id
            args (list[list[int, int]]): List of key value pairs
            kwargs: Key value pairs

        Returns:
            int: Operator id
        '''
        params = []
        for entry in args:
            if not isinstance(entry, list):
                raise RuntimeError('arg %s invalid should be a list' % (entry))
            elif len(entry) != 2:
                raise RuntimeError('arg %s invalid should be list of 2 elements' % (entry))
            elif not isinstance(entry[1], int):
                raise ValueError('create_operator_ex key:%s val:%s invalid' % (entry[0], entry[1]))

            key_val = self._get_value(CREATE_OPERATOR_EX_DICT, entry[0])
            if not isinstance(key_val, int):
                raise ValueError('create_operator_ex key:%s invalid' % (entry[0]))
            elif not isinstance(entry[1], int):
                raise ValueError('create_operator_ex key:%s invalid' % (entry[0]))
            params.append([key_val, entry[1]])

        for entry in kwargs:
            key_val = self._get_value(CREATE_OPERATOR_EX_DICT, entry)
            if not isinstance(key_val, int):
                raise ValueError('create_operator_ex key:%s invalid' % (entry))
            elif not isinstance(kwargs[entry], int):
                raise ValueError(
                    'create_operator_ex key:%s val:%s invalid' % (entry, kwargs[entry]))
            params.append([key_val, kwargs[entry]])

        payload = [len(params)] + [val for entry in params
                                   for val in [entry[0], entry[1] & 0xFFFF, entry[1] >> 16]]
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_CREATE_OPERATOR_EX_REQ, [cap_id] + payload)
        if cmd_id != ACCMD_CMD_ID_CREATE_OPERATOR_EX_RESP:
            raise RuntimeError('create_operator_ex response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('create_operator_ex response length:%s invalid' % (len(payload)))
        return payload[0]

    @log_output(logging.INFO)
    @log_exception
    def get_mem_usage(self, processor_id=0):
        '''
        Request processor memory usage

        Args:
            processor_id (int): Processor, 0 is main

        Returns:
            (dict)
                heap_size (int) Heap size in words
                heap_current (int) Heap current in words
                heap_min (int) Heap min in words
                pool_size (int) Pool size in words
                pool_min (int) Pool current in words
                pool_current (int) Pool min in words
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_GET_MEM_USAGE_REQ, [processor_id])
        if cmd_id != ACCMD_CMD_ID_GET_MEM_USAGE_RESP:
            raise RuntimeError('get_mem_usage response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 12:
            raise RuntimeError('get_mem_usage length:%s invalid' % (len(payload)))

        return {
            'heap_size': (payload[1] << 16) | payload[0],
            'heap_current': (payload[3] << 16) | payload[2],
            'heap_min': (payload[5] << 16) | payload[4],
            'pool_size': (payload[7] << 16) | payload[6],
            'pool_current': (payload[9] << 16) | payload[8],
            'pool_min': (payload[11]) << 16 | payload[10],
        }

    @log_output(logging.INFO)
    @log_exception
    def clear_mem_watermarks(self):
        '''
        Clear memory watermarks
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_CLEAR_MEM_WATERMARKS_REQ)
        if cmd_id != ACCMD_CMD_ID_CLEAR_MEM_WATERMARKS_RESP:
            raise RuntimeError('clear_mem_watermarks response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 0:
            raise RuntimeError('clear_mem_watermarks length:%s invalid' % (len(payload)))

    @log_output(logging.INFO, formatters={'operators': '0x%04x'})
    @log_exception
    def get_mips_usage(self, operators):
        '''
        Request processor mips usage

        Args:
            operators (list[int]): List of operator ids

        Returns:
            tuple[int, dict] Sleep in thousandths and dict with operator_id as key and operator mips
                in thousandths
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_GET_MIPS_USAGE_REQ,
                                         [len(operators)] + operators)
        if cmd_id != ACCMD_CMD_ID_GET_MIPS_USAGE_RESP:
            raise RuntimeError('get_mips_usage response cmd_id:%04x invalid' % (cmd_id))
        # if len(payload) < 2 or (len(payload) - 2) < 2 * payload[1]:
        if len(payload) < 2 or 2 * payload[1] != (len(payload) - 2):
            raise RuntimeError('get_mips_usage length:%s invalid' % (len(payload)))

        ops = payload[1]
        ret = {}
        for ind in range(ops):
            ret[payload[2 * ind + 2]] = payload[2 * ind + 3]
        return payload[0], ret

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'source_id': '0x%04x', 'sink_id': '0x%04x'})
    @log_exception
    def stream_disconnect(self, source_id, sink_id):
        '''
        Disconnect a connection made between a specified source and sink

        Args:
            source_id (int): Source id
            sink_id (int): Sink id

        Returns:
            tuple[int]: Transform id 1 and transform id 2
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_DISCONNECT_REQ,
                                         [source_id, sink_id])
        if cmd_id != ACCMD_CMD_ID_STREAM_DISCONNECT_RESP:
            raise RuntimeError('stream_disconnect response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) != 2:
            raise RuntimeError('stream_disconnect response length:%s invalid' % (len(payload)))
        return payload[0], payload[1]

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'source_id': '0x%04x', 'return': '0x%04x'})
    @log_exception
    def stream_get_sink_from_source(self, source_id):
        '''
        Returns the sink paired with the specified source

        Args:
            source_id (int): Source id

        Returns:
            int: Sink id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_GET_SINK_FROM_SOURCE_REQ, [source_id])
        if cmd_id != ACCMD_CMD_ID_STREAM_GET_SINK_FROM_SOURCE_RESP:
            raise RuntimeError('stream_get_sink_from_source response cmd_id:%04x invalid' %
                               (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('stream_get_sink_from_source response length:%s invalid' %
                               (len(payload)))
        return payload[0]

    @log_input_output(logging.INFO, logging.INFO,
                      formatters={'sink_id': '0x%04x', 'return': '0x%04x'})
    @log_exception
    def stream_get_source_from_sink(self, sink_id):
        '''
        Returns the source paired with the specified sink

        Args:
            sink_id (int): Sink id

        Returns:
            int: Source id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_GET_SOURCE_FROM_SINK_REQ, [sink_id])
        if cmd_id != ACCMD_CMD_ID_STREAM_GET_SOURCE_FROM_SINK_RESP:
            raise RuntimeError('stream_get_source_from_sink response cmd_id:%04x invalid' %
                               (cmd_id))
        if len(payload) != 1:
            raise RuntimeError('stream_get_source_from_sink response length:%s invalid' %
                               (len(payload)))
        return payload[0]

    @log_input(logging.INFO)
    @log_exception
    def cap_download_remove_kcs(self, kcs_id):
        '''
        Remove downloadable capability bundle

        Args:
            kcs_id (int): Bundle id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_CAP_DOWNLOAD_REMOVE_KCS_REQ, [kcs_id])
        self._check_standard_resp('cap_download_remove_kcs', cmd_id, payload)

    @log_input(logging.INFO, formatters={'message': '0x%04x'})
    @log_exception
    def set_system_framework(self, key, message):
        '''
        Set System Framework request

        Args:
            key (int): Key id
            message (list[int]): Message
        '''
        key_val = self._get_value(FRAMEWORK_KEY_DICT, key)
        if not isinstance(key_val, int):
            raise ValueError('set_system_framework key:%s invalid' % (key))
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_SET_SYSTEM_FRAMEWORK_REQ, [key_val] + message)
        if len(payload):
            raise RuntimeError('set_system_framework_req length:%s invalid' % (len(payload)))

    @log_output(logging.INFO, formatters={'message': '0x%04x'})
    @log_exception
    def get_system_framework(self, key, message=None):
        '''
        Get System Framework request

        Args:
            key (int): Key id
            message (list[int]): Message

        Returns:
            list[int]: Reply
        '''
        message = message if message else []
        key_val = self._get_value(FRAMEWORK_KEY_DICT, key)
        if not isinstance(key_val, int):
            raise ValueError('get_system_framework key:%s invalid' % (key))
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_GET_SYSTEM_FRAMEWORK_REQ, [key_val] + message)
        return payload

    # TODO ACCMD_CMD_ID_MIC_BIAS_CONFIGURE_REQ
    # TODO ACCMD_CMD_ID_CODEC_SET_IIR_FILTER_REQ
    # TODO ACCMD_CMD_ID_CODEC_SET_IIR_FILTER_16BIT_REQ
    # TODO ACCMD_CMD_ID_AUX_PROCESSOR_LOAD_REQ
    # TODO ACCMD_CMD_ID_AUX_PROCESSOR_UNLOAD_REQ
    # TODO ACCMD_CMD_ID_AUX_PROCESSOR_START_REQ
    # TODO ACCMD_CMD_ID_AUX_PROCESSOR_STOP_REQ
    # TODO ACCMD_CMD_ID_STREAM_MCLK_ACTIVATE_REQ
    # TODO ACCMD_CMD_ID_SET_MCLK_SOURCE_TYPE_REQ

    @log_input_output(logging.INFO, logging.INFO)
    @log_exception
    def get_operators(self, flt=0, start=0):
        '''
        Request a list of ids for the available operators

        The request will include all operators instantiated matching the filter.
        At most 20 operators will be reported

        Args:
            flt (int): cap_id information is requested for or 0 for all capabilities
            start (int): Start index

        Returns:
            tuple[int, dict]: Total number of available operators for this filter, operators
                dict with key being the operator id and value the capability id
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_GET_OPERATORS_REQ, [flt, start])
        if cmd_id != ACCMD_CMD_ID_GET_OPERATORS_RESP:
            raise RuntimeError('get_operators response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) < 2:
            raise RuntimeError('get_operators response length:%s invalid' % (len(payload)))
        if 2 * payload[1] != len(payload[2:]):
            raise RuntimeError('get_operators response count:%d does not match data' %
                               (payload[1]))
        op_ret = {}
        for ind in range(int(len(payload[2:]) / 2)):
            op_ret[payload[2 + 2 * ind]] = payload[2 + 2 * ind + 1]
        return payload[0], op_ret

    @log_input(logging.INFO)
    @log_exception
    def ps_shutdown(self):
        '''
        Flush PS Keys to PS Store
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_PS_SHUTDOWN_REQ)
        self._check_standard_resp('ps_shutdown', cmd_id, payload)

    @log_input(logging.INFO)
    @log_exception
    def ps_register(self):
        '''
        Register PS Store
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_PS_REGISTER_REQ)
        self._check_standard_resp('ps_register', cmd_id, payload)

    # TODO ACCMD_CMD_ID_SET_PROFILER_REQ
    # TODO ACCMD_CMD_ID_CLOCK_CONFIGURE_REQ
    # TODO ACCMD_CMD_ID_SET_ANC_IIR_FILTER_REQ
    # TODO ACCMD_CMD_ID_SET_ANC_LPF_FILTER_REQ
    # TODO ACCMD_CMD_ID_STREAM_ANC_ENABLE_REQ

    @log_input(logging.INFO)
    @log_exception
    def stream_sidetone_enable(self, enable):
        '''
        Enable stream uninterruptable sidetone

        Args:
            enable (int): Enable or disable the tone
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_STREAM_SIDETONE_EN_REQ, [enable])
        self._check_standard_resp('stream_sidetone_enable', cmd_id, payload)

    # TODO ACCMD_CMD_ID_DATA_FILE_ALLOCATE_REQ
    # TODO ACCMD_CMD_ID_DATA_FILE_DEALLOCATE_REQ
    # TODO ACCMD_CMD_ID_CLOCK_POWER_SAVE_MODE_REQ

    @log_input(logging.INFO)
    @log_exception
    def cap_download_add_kcs(self, processor_id=0):
        '''
        Get the transform currently connected to a source or sink

        Args:
            processor_id (int): Processor id

        Returns:
            int: Transform if exists, else 0
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_CAP_DOWNLOAD_ADD_KCS_REQ, [processor_id])
        self._check_standard_resp('cap_download_add_kcs', cmd_id, payload)

    # FIXME handle ACCMD_CMD_ID_CAP_DOWNLOAD_COMPLETE_KCS_REQ received message
    # TODO ACCMD_CMD_ID_AUX_PROCESSOR_DISABLE_REQ

    @log_output(logging.INFO, formatters={'sid': '0x%04x'})
    @log_exception
    def transform_from_stream(self, sid):
        '''
        Get the transform currently connected to a source or sink

        Args:
            sid (int): Source or sink id

        Returns:
            int: Transform if exists, else 0
        '''
        cmd_id, payload = self.send_recv(ACCMD_CMD_ID_TRANSFORM_FROM_STREAM_REQ, [sid])
        if cmd_id != ACCMD_CMD_ID_TRANSFORM_FROM_STREAM_RESP:
            raise RuntimeError('transform_from_stream response cmd_id:%04x invalid' % (cmd_id))
        if len(payload) < 1:
            raise RuntimeError('transform_from_stream response length:%s invalid' % (len(payload)))
        return payload[0]

    # TODO ACCMD_CMD_ID_FILE_DETAILS_REQ
    # TODO ACCMD_CMD_ID_ACQUIRE_FLASH_LOCK_REQ
    # TODO ACCMD_CMD_ID_RELEASE_FLASH_LOCK_REQ
