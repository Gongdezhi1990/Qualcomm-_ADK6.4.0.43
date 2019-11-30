'''
Capability Base class
'''

import logging
from abc import ABCMeta, abstractmethod, abstractproperty

from six import add_metaclass

from kats.framework.library.log import log_input, log_output, log_exception

CAPABILITY_VERSION_REQUEST = 0x1000
ENABLE_FADE_OUT = 0x2000
DISABLE_FADE_OUT = 0x2001
SET_CONTROL = 0x2002
GET_PARAMS = 0x2003
GET_DEFAULTS = 0x2004
SET_PARAMS = 0x2005
GET_STATUS = 0x2006
SET_UCID = 0x2007
GET_LOGICAL_PS_ID = 0x2008
SET_BUFFER_SIZE = 0x200c
SET_TERMINAL_BUFFER_SIZE = 0x200d
SET_SAMPLE_RATE = 0x200e
SET_DATA_STREAM_BASED = 0x200f

STATUS_NORMAL_STATE = 0x0
STATUS_MESSAGE_TOO_BIG = 0x1
STATUS_PARAMETER_STATE_NOT_READY = 0x2
STATUS_INVALID_PARAMETER = 0x3
STATUS_UNSUPPORTED_CONTROL = 0x4
STATUS_INVALID_CONTROL_VALUE = 0x5
STATUS_UNSUPPORTED_ENCODING = 0x6

STATUS_DESCR = {
    STATUS_NORMAL_STATE: 'normal state',
    STATUS_MESSAGE_TOO_BIG: 'message too big',
    STATUS_PARAMETER_STATE_NOT_READY: 'parameter state not ready',
    STATUS_INVALID_PARAMETER: 'invalid parameter',
    STATUS_UNSUPPORTED_CONTROL: 'unsupported control',
    STATUS_INVALID_CONTROL_VALUE: 'invalid control value',
    STATUS_UNSUPPORTED_ENCODING: 'unsupported encoding'
}

STATE_NOT_CREATED = 'not created'
STATE_CREATED = 'created'
STATE_STARTED = 'started'
STATE_STOPPED = 'stopped'

CONTROL_ID_MODE = 0x0001
CONTROL_ID_MUTE = 0x0002

BLOCK_LOAD_DEFAULT_MASK = 0x8000

ENCODING_24B = 0
ENCODING_32B = 1
ENCODING_DEFAULT = ENCODING_32B
ENCODING_LIST = [ENCODING_24B, ENCODING_32B]


@add_metaclass(ABCMeta)
class CapabilityBase(object):
    '''
    Capability base class. Should be subclassed by every capability extension.

    This builds an interface on top of kymera to handle capabilities and operators.
    kymera interface is a single interface of multiple operators while CapabilityBase is
    an instance that handle a single operator.

    .. code-block:: python

        # PassThrough is subclassing CapabilityBase
        from kats.kalimba.capability.common.pass_through import PassThroughCapability
        op = PassThroughCapability('pass_through', kymera, input_data_type=1, output_data_type=1)
        op.create() # create operator in uut
        op.config() # send all configuration messages
        op.start()
        ...
        op.stop()
        op.destroy()
        del op

    Args:
        cap (str): Name of the capability
        kymera (kats.kalimba.kymera.kymera_base.KymeraBase): Instance of class Kymera
    '''

    def __init__(self, cap, kymera, *_, **__):
        self.capability = cap
        self._kymera = kymera
        self._op_id = None
        self.__parameters = {}
        self.__state = STATE_NOT_CREATED

    @abstractproperty
    def platform(self):
        '''
        str: Platform name
        '''
        pass

    @abstractproperty
    def interface(self):
        '''
        str: Interface name
        '''
        pass

    @abstractproperty
    def cap_id(self):
        '''
        int: Capability id
        '''
        pass

    @staticmethod
    def _encode_params(data, encoding=ENCODING_DEFAULT):
        enc_params = []
        if encoding == ENCODING_24B:
            for ind, entry in enumerate(data):
                if not ind % 2:
                    enc_params.append((entry >> 8) & 0xFFFF)
                    pend = entry & 0xFF
                    if ind == len(data) - 1:
                        enc_params.append((pend & 0xFF) << 8)
                else:
                    enc_params.append(((pend & 0xFF) << 8) | ((entry >> 16) & 0xFF))
                    enc_params.append(entry & 0xFFFF)
        else:  # 32b encoding
            for entry in data:
                enc_params.append((entry >> 16) & 0xFFFF)
                enc_params.append(entry & 0xFFFF)
        return enc_params

    @staticmethod
    def _decode_params(data, encoding=ENCODING_DEFAULT):
        dec_params = []
        if encoding == ENCODING_24B:
            ind = 0
            while ind < len(data):
                val = data[ind] << 8
                ind += 1
                val |= (data[ind] >> 8) & 0xFF
                dec_params.append(val)
                pend = (data[ind] & 0xFF) << 16
                ind += 1
                if len(data) > ind:
                    val = pend | (data[ind] & 0xFFFF)
                    dec_params.append(val)
                    ind += 1
                else:
                    break
        else:
            ind = 0
            while ind < len(data):
                val = (data[ind] & 0xFFFF) << 16
                ind += 1
                val |= (data[ind] & 0xFFFF)
                ind += 1
                dec_params.append(val)
        return dec_params

    def get_id(self):
        '''
        Get operator id

        Returns:
            int: Operator id
        '''
        return self._op_id

    @log_input(logging.INFO)
    @abstractmethod
    def create(self, *args, **kwargs):
        '''
        Create new operator
        '''
        if self._op_id:
            raise RuntimeError('operator already created')
        else:
            if kwargs:
                self._op_id = self._kymera.opmgr_create_operator_ex(self.cap_id, *args, **kwargs)
            else:
                self._op_id = self._kymera.opmgr_create_operator(self.cap_id)
            self.__state = STATE_CREATED

    @log_input(logging.INFO)
    @log_exception
    def destroy(self):
        '''
        Destroy operator

        Raises:
            RuntimeError: If the operator has not been previously created
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            self._kymera.opmgr_destroy_operators([self._op_id])
            self._op_id = None
            self.__state = STATE_NOT_CREATED

    @abstractmethod
    def config(self):
        '''
        Configure operator
        '''
        pass

    @log_input(logging.INFO)
    def start(self):
        '''
        Start operator
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            self._kymera.opmgr_start_operators([self._op_id])
            self.__state = STATE_STARTED

    @log_input(logging.INFO)
    def stop(self):
        '''
        Stop operator
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            self._kymera.opmgr_stop_operators([self._op_id])
            self.__state = STATE_STOPPED

    @log_input(logging.INFO)
    def reset(self):
        '''
        Reset operator
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            self._kymera.opmgr_reset_operators([self._op_id])
            self.__state = STATE_CREATED

    def send_recv_operator_message(self, msg):
        '''
        Send message to uut operator

        Args:
            list[int]: Message to be sent

        Returns:
            list[int]: Response
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            return self._kymera.opmgr_operator_message(self._op_id, msg)

    def get_source_endpoint(self, num):
        '''
        Get source endpoint id

        Args:
            num (int): Zero-based index of the source endpoint in the operator

        Returns:
            int: Source endpoint id
        '''
        return 0x2000 | self._op_id | num

    def get_source_terminal(self, num):
        '''
        Get source terminal id

        Args:
            num (int): Zero-based index of the source terminal in the operator

        Returns:
            int: Source terminal id
        '''
        return 0x6000 | self._op_id | num

    def get_sink_endpoint(self, num):
        '''
        Get sink endpoint id

        Args:
            num (int): Zero-based index of the sink endpoint in the operator

        Returns:
            int: Sink endpoint id
        '''
        return 0xA000 | self._op_id | num

    def get_sink_terminal(self, num):
        '''
        Get sink terminal id

        Args:
            num (int): Zero-based index of the sink terminal in the operator

        Returns:
            int: Sink endpoint id
        '''
        return 0xE000 | self._op_id | num

    def get_state(self):
        '''
        Get capability state

        Returns:
            str: Capability state
        '''
        return self.__state

    # Compulsory operator messages
    # Should be supported by every operator

    @log_output(logging.INFO)
    def version_request(self):
        '''
        Get capability version

        Returns:
            str: Capability version

        Raises:
            RuntimeError: if the operator has not been created of the uut responds incorrectly
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            ret = self.send_recv_operator_message([CAPABILITY_VERSION_REQUEST])
            if len(ret) != 3:
                raise RuntimeError('version_request response length:%s invalid' % len(ret))
            elif ret[0] != CAPABILITY_VERSION_REQUEST:
                raise RuntimeError('version_request response cmd:%04x invalid' % (ret[0]))
            else:
                return str(ret[1]) + '.' + str(ret[2])

    # obpm messages

    @log_input(logging.INFO, formatters={'control_id': '0x%04x', 'params': '0x%04x'})
    @log_exception
    def set_control(self, control_id, params):
        '''
        Send set control message

        Args:
            control_id (int): Specific control to set
            params (list[int]): Control values, 24 or 32 bits, to be sent
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            enc_params = self._encode_params(params, encoding=ENCODING_32B)
            ret = self.send_recv_operator_message(
                [SET_CONTROL, len(params) / 2, control_id] + enc_params)
            if len(ret) != 2:
                raise RuntimeError('set_control response length:%s invalid' % len(ret))
            elif ret[0] != SET_CONTROL:
                raise RuntimeError('set_control response cmd:%04x invalid' % (ret[0]))
            elif ret[1] != STATUS_NORMAL_STATE and ret[1] != STATUS_PARAMETER_STATE_NOT_READY:
                raise RuntimeError('set control status:%s (%s) invalid' %
                                   (ret[1], STATUS_DESCR.get(ret[1], 'unknown')))

    @log_output(logging.INFO, formatters={'return': '0x%04x'})
    @log_exception
    def get_params(self, offset, length=1, encoding=ENCODING_DEFAULT):
        '''
        Send get params message

        Note it only supports requesting one block

        Args:
            offset (int): Parameter offset to read
            length (int): Number of parameters to read, each parameters will could be 24 or 32 bits
            encoding (int): Params encoding, 0 for 24b, 1 for 32b

        Returns:
            list[int]: Parameters read
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        elif encoding not in ENCODING_LIST:
            raise RuntimeError('encoding %s unknown' % (encoding))
        else:
            blocks = 1 + (encoding << 12)
            ret = self.send_recv_operator_message([GET_PARAMS, blocks, offset, length])
            if len(ret) < 7:
                raise RuntimeError('get_params response length:%s invalid' % (len(ret)))
            if ret[0] != GET_PARAMS:
                raise RuntimeError('get_params response cmd:%04x invalid' % (ret[0]))
            elif ret[1] != STATUS_NORMAL_STATE and ret[1] != STATUS_PARAMETER_STATE_NOT_READY:
                raise RuntimeError('get_params status:%s (%s) invalid' %
                                   (ret[1], STATUS_DESCR.get(ret[1], 'unknown')))
            elif ret[2] != blocks:
                raise RuntimeError('get_params response blocks invalid sent %s received %s' %
                                   (blocks, ret[2]))
            elif ret[3] != offset:
                raise RuntimeError('get_params response offset invalid sent %s received %s' %
                                   (offset, ret[3]))
            elif ret[4] != length:
                raise RuntimeError('get_params response range invalid sent %s received %s' %
                                   (length, ret[4]))
            return self._decode_params(ret[5:], encoding=encoding)

    @log_output(logging.INFO, formatters={'return': '0x%04x'})
    @log_exception
    def get_defaults(self, offset, length=1, encoding=ENCODING_DEFAULT):
        '''
        Send get default params message

        Note it only supports requesting one block

        Args:
            offset (int): Parameter offset to read
            length (int): Number of parameters to read, each parameters will could be 24 or 32 bits
            encoding (int): Params encoding, 0 for 24b, 1 for 32b

        Returns:
            list[int]: Parameters read
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        elif encoding not in ENCODING_LIST:
            raise RuntimeError('encoding %s unknown' % (encoding))
        else:
            blocks = 1 + (encoding << 12)
            ret = self.send_recv_operator_message([GET_DEFAULTS, blocks, offset, length])
            if len(ret) < 7:
                raise RuntimeError('get_defaults length:%s invalid' % (len(ret)))
            elif ret[0] != GET_DEFAULTS:
                raise RuntimeError('get_defaults cmd:%04x invalid' % (ret[0]))
            elif ret[1] != STATUS_NORMAL_STATE and ret[1] != STATUS_PARAMETER_STATE_NOT_READY:
                raise RuntimeError('get_defaults status:%s (%s) invalid' %
                                   (ret[1], STATUS_DESCR.get(ret[1], 'unknown')))
            elif ret[2] != blocks:
                raise RuntimeError('get_defaults blocks invalid sent %s received %s' %
                                   (blocks, ret[2]))
            elif ret[3] != offset:
                raise RuntimeError('get_defaults offset invalid sent %s received %s' %
                                   (offset, ret[3]))
            elif ret[4] != length:
                raise RuntimeError('get_defaults range invalid sent %s received %s' %
                                   (length, ret[4]))
            return self._decode_params(ret[5:], encoding=encoding)

    @log_input(logging.INFO, formatters={'params': '0x%04x'})
    @log_exception
    def set_params(self, offset, params, load_defaults=False, encoding=ENCODING_DEFAULT):
        '''
        Send set params message

        Note it only supports requesting one block

        Args:
            offset (int): Parameter offset to read
            params (list[int]): Parameters to write, should come in 24 or 32 bits data per parameter
            load_defaults (bool): Load defaults prior to execute commands
            encoding (int): Params encoding, 0 for 24b, 1 for 32b
        '''
        if not self._op_id:
            raise RuntimeError('operator not created')
        elif encoding not in ENCODING_LIST:
            raise RuntimeError('encoding %s unknown' % (encoding))
        else:
            blocks = 1 + (encoding << 12)
            if load_defaults:
                blocks += BLOCK_LOAD_DEFAULT_MASK
            enc_params = self._encode_params(params, encoding=encoding)

            ret = self.send_recv_operator_message([SET_PARAMS, blocks, offset, len(params)] +
                                                  enc_params)
            if len(ret) != 2:
                raise RuntimeError('set_params length:%s invalid' % (len(ret)))
            elif ret[0] != SET_PARAMS:
                raise RuntimeError('set_params cmd:%04x invalid' % (ret[0]))
            elif ret[1] != STATUS_NORMAL_STATE and ret[1] != STATUS_PARAMETER_STATE_NOT_READY:
                raise RuntimeError('set params status:%s (%s) invalid' %
                                   (ret[1], STATUS_DESCR.get(ret[1], 'unknown')))

    @log_output(logging.INFO)
    @log_exception
    def get_status(self, encoding=ENCODING_DEFAULT):
        '''
        Send get status

        Returns:
            list[int]: Operator specific status data
            encoding (int): Params encoding, 0 for 24b, 1 for 32b
        '''
        # FIXME this does not work
        if not self._op_id:
            raise RuntimeError('operator not created')
        elif encoding not in ENCODING_LIST:
            raise RuntimeError('encoding %s unknown' % (encoding))
        else:
            config = (encoding << 12)
            ret = self.send_recv_operator_message([GET_STATUS, config])
            if len(ret) < 2:
                raise RuntimeError('get_status response length:%s invalid' % len(ret))
            elif ret[0] != GET_STATUS:
                raise RuntimeError('get_status response cmd:%04x invalid' % (ret[0]))
            elif ret[1] != STATUS_NORMAL_STATE and ret[1] != STATUS_PARAMETER_STATE_NOT_READY:
                raise RuntimeError('get status status:%s (%s) invalid' %
                                   (ret[1], STATUS_DESCR.get(ret[1], 'unknown')))
            return ret[2:]

    # Standard operator messages
    # May or may not be included in every capability.
    # Should be subclassed by each capability that suports them and refer to base class
    # if they exist

    @log_input(logging.INFO)
    @log_exception
    def _enable_fade_out(self):
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            ret = self.send_recv_operator_message([ENABLE_FADE_OUT])
            if len(ret) != 1:
                raise RuntimeError('enable_fade_out response length:%s invalid' % len(ret))
            elif ret[0] != ENABLE_FADE_OUT:
                raise RuntimeError('enable_fade_out response cmd:%04x invalid' % (ret[0]))

    @log_input(logging.INFO)
    @log_exception
    def _disable_fade_out(self):
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            ret = self.send_recv_operator_message([DISABLE_FADE_OUT])
            if len(ret) != 1:
                raise RuntimeError('disable_fade_out response length:%s invalid' % len(ret))
            elif ret[0] != DISABLE_FADE_OUT:
                raise RuntimeError('disable_fade_out response cmd:%04x invalid' % (ret[0]))

    @log_input(logging.INFO)
    @log_exception
    def _set_ucid(self, ucid):
        raise NotImplementedError('command not implemented')

    @log_output(logging.INFO)
    @log_exception
    def _get_logical_ps_id(self):
        raise NotImplementedError('command not implemented')
        # if not self._op_id:
        #    raise RuntimeError('operator not created')
        # else:
        #    ret = self.send_recv_operator_message([GET_LOGICAL_PS_ID])
        #    if len(ret) != 1:
        #        raise RuntimeError('get_logical_ps_id response length:%s invalid' % len(ret))
        #    elif ret[0] != GET_LOGICAL_PS_ID:
        #        raise RuntimeError('get_logical_ps_id response cmd:%04x invalid' % (ret[0]))
        #    return ret[1]

    @log_input(logging.INFO)
    @log_exception
    def _set_buffer_size(self, size):
        raise NotImplementedError('command not implemented')

    @log_input(logging.INFO)
    @log_exception
    def _set_terminal_buffer_size(self, size, sinks, sources):
        raise NotImplementedError('command not implemented')

    @log_input(logging.INFO)
    @log_exception
    def _set_sample_rate(self, sample_rate):
        raise NotImplementedError('command not implemented')

    @log_input(logging.INFO)
    @log_exception
    def _set_data_stream_based(self, enable):
        if not self._op_id:
            raise RuntimeError('operator not created')
        else:
            ret = self.send_recv_operator_message([SET_DATA_STREAM_BASED, enable])
            if len(ret) != 1:
                raise RuntimeError('set_data_stream_based response length:%s invalid' % len(ret))
            elif ret[0] != SET_DATA_STREAM_BASED:
                raise RuntimeError('set_data_stream_based response cmd:%04x invalid' % (ret[0]))
