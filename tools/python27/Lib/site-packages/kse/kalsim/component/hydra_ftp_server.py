'''
Hydra FTP Server
'''

import argparse
import glob
import logging
import os
import struct
import time
from functools import partial

from kats.framework.library.log import log_input
from kats.kalimba.hydra_service.constants import FTP_FW_TO_SERV_CTRL, FTP_FW_TO_SERV_DATA, \
    CCP_SIGNAL_ID_OPERATIONAL_IND
from kats.library.registry import get_instance

CHIP_READY_TIMEOUT = 2

# FTP message ids
FTP_CTRL_SESSION_CREATE_REQ_PDU_ID = 0
FTP_CTRL_SESSION_CREATE_CFM_PDU_ID = 1
FTP_CTRL_SESSION_DESTROY_REQ_PDU_ID = 2
FTP_CTRL_SESSION_DESTROY_CFM_PDU_ID = 3
FTP_CTRL_FILE_OPEN_REQ_PDU_ID = 4
FTP_CTRL_FILE_OPEN_CFM_PDU_ID = 5
FTP_CTRL_FILE_READ_REQ_PDU_ID = 6
FTP_CTRL_FILE_READ_CFM_PDU_ID = 7
FTP_CTRL_FILE_WRITE_REQ_PDU_ID = 8
FTP_CTRL_FILE_WRITE_CFM_PDU_ID = 9
FTP_CTRL_FILE_SEEK_REQ_PDU_ID = 10
FTP_CTRL_FILE_SEEK_CFM_PDU_ID = 11
FTP_CTRL_FILE_CLOSE_REQ_PDU_ID = 12
FTP_CTRL_FILE_CLOSE_CFM_PDU_ID = 13
FTP_CTRL_FILE_DELETE_REQ_PDU_ID = 14
FTP_CTRL_FILE_DELETE_CFM_PDU_ID = 15

# session create request protocol version
CSR_FTP_PROTOCOL_VERSION_LATEST = 0

# session create request flags
CSR_FTP_SERVER_NON_VOLATILE = 1
CSR_FTP_SERVER_VOLATILE = 2
CSR_FTP_SERVER_LOCAL = 4
CSR_FTP_SERVER_HOST = 8

# file open request mode
CSR_FTP_OPEN_MODE_CREATE_ALWAYS = 1
CSR_FTP_OPEN_MODE_APPEND_ALWAYS = 2
CSR_FTP_OPEN_MODE_OPEN_EXISTING = 3

# file open max transfer unit
MTU_DEFAULT = 0x8000

# file open format
FORMAT_PATCH = 0
FORMAT_CONFIG = 1
FORMAT_DATA = 2

FORMAT_STR = {
    FORMAT_PATCH: 'patch',
    FORMAT_CONFIG: 'config',
    FORMAT_DATA: 'data',
}

INVALID_HANDLE_VALUE = 0xFFFFFFFF

# file seek origin
CSR_FTP_SEEK_SET = 0
CSR_FTP_SEEK_CUR = 1
CSR_FTP_SEEK_END = 2

# result
CSR_FTP_RESULT_SUCCESS = 0
CSR_FTP_FILE_OP_FAILURE = 1
CSR_FTP_DIR_OP_FAILURE = 1,
CSR_FTP_DELETE_FAILURE = 1,
CSR_FTP_FILE_OP_EOF = 2
CSR_FTP_DELETE_READ_ONLY = 2,
CSR_FTP_DELETE_NOT_EXIST = 3,
CSR_FTP_FILE_OP_READ_ONLY = 3
CSR_FTP_DELETE_NOT_EMPTY = 4,
CSR_FTP_FILE_OP_NOT_EXIST = 4
CSR_FTP_FILE_OP_NOT_ALLOWED = 5
CSR_FTP_FILE_OP_ALREADY_EXISTS = 5
CSR_FTP_FILE_OP_NO_SPACE = 7

# session create and file open
SERVER_ADDRESS = 0
SERVER_PORT_CONTROL = 1
SERVER_PORT_DATA = 2

FILE_EXTENSION = 'hcf'


class Session(object):
    '''
    Session object
    '''

    def __init__(self, session_id, address, port):
        self._session_id = session_id
        self._address = address
        self._port = port

    @property
    def session_id(self):
        '''
        Get session id

        Returns:
            int: Session id
        '''
        return self._session_id

    @property
    def address(self):
        '''
        Get server address

        Returns:
            int: Server address
        '''
        return self._address

    @property
    def port(self):
        '''
        Get server port

        Returns:
            int: Server port
        '''
        return self._session_id


class Handle(object):
    '''
    Handle object
    '''

    def __init__(self, handle, session_id, mode, filename, file_spec, address, port):
        self._handle = handle
        self._session_id = session_id
        self._mode = mode
        self._filename = filename
        self._file_spec = file_spec
        self._address = address
        self._port = port
        self._pos = 0

    @property
    def handle(self):
        '''
        Get handle id

        Returns:
            int: handle id
        '''
        return self._handle

    @property
    def filename(self):
        '''
        Get filename

        Returns:
            str: Filename
        '''
        return self._filename

    @property
    def pos(self):
        '''
        Get file position

        Returns:
            int: Position
        '''
        return self._pos

    @pos.setter
    def pos(self, val):
        '''
        Set file position

        Args:
            val (int): New file position
        '''
        self._pos = val if val >= 0 else 0


class HydraFtpServer(object):
    '''
    Hydra File transfer protocol server class

    This class provides a ftp server to support kalsim firmware patches.
    On instantiation it registers itself with firmware and listens for requests coming
    (read, write, delete).

    Currently it only supports a read only filesystem without seek support.
    Valid files are those with extension hcf in the selected directory
    Those files come in the format subsysN_configL or subsysN_dataL or subsysN_patchP_fwFFWW unless
    a prefix is selected which is prepended to filenames.

    Args:
        hydra_protocol (kats.kalimba.hydra_service.protocol.HydraProtocol): Protocol handler
        directory (str): Directory containing files
        prefix (str): Prefix to be applied to every file
    '''

    def __init__(self, hydra_protocol, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log

        self.__config = argparse.Namespace()
        self.__config.hydra_protocol = hydra_protocol
        self.__config.uut = get_instance('uut')
        self.__config.directory = kwargs.pop('directory', None)
        self.__config.prefix = kwargs.pop('prefix', '')

        self.__data = argparse.Namespace()
        self.__data.session = []
        self.__data.handle = []
        self.__data.last_handle = 0
        self.__data.file_data = {}
        self.__data.pending = []
        self.__data.chip_ready = False

        # load file contents in a dictionary
        # we are only supporting read access and files are small so this is a simple way to
        # handle our ftp server
        if self.__config.directory is not None:
            wildcard = '%s*.%s' % (self.__config.prefix, FILE_EXTENSION)
            files = glob.glob(os.path.join(self.__config.directory, wildcard))
            for filename in files:
                with open(filename, 'rb') as handler:
                    self.__data.file_data[os.path.basename(filename)] = handler.read()
                    self._log.info('ftp_server found filename:%s', filename)

        # install message handler to receive messages from kymera
        self._msg_handler = self.__config.hydra_protocol.install_message_handler(
            self._message_received)

        # send indication to kymera meaning we are ready to receive requests
        self.__config.hydra_protocol.send_service_advice_indication()

        # wait for kymera to signal it is ready, if it timeouts we just log and continue
        timer0 = time.time()
        while time.time() - timer0 < CHIP_READY_TIMEOUT:
            time.sleep(0.1)
            if self.__data.chip_ready:
                break
        else:
            self._log.warning('chip ready message not received')

    @log_input(logging.DEBUG, formatters={'msg': '0x%04x'})
    def _message_received(self, msg):
        if msg and msg[0] == CCP_SIGNAL_ID_OPERATIONAL_IND:
            self.__data.chip_ready = True
            self._log.info('chip ready message received')

        if msg and msg[0] == FTP_FW_TO_SERV_DATA:
            if self.__data.pending:
                self._send_control(self.__data.pending)
                self.__data.pending = []

        if len(msg) >= 2 and msg[0] == FTP_FW_TO_SERV_CTRL:
            handlers = {
                FTP_CTRL_SESSION_CREATE_REQ_PDU_ID: self._handle_session_create,
                FTP_CTRL_SESSION_DESTROY_REQ_PDU_ID: self._handle_session_destroy,
                FTP_CTRL_FILE_OPEN_REQ_PDU_ID: self._handle_file_open,
                FTP_CTRL_FILE_READ_REQ_PDU_ID: self._handle_file_read,
                FTP_CTRL_FILE_WRITE_REQ_PDU_ID: self._handle_file_write,
                FTP_CTRL_FILE_SEEK_REQ_PDU_ID: self._handle_file_seek,
                FTP_CTRL_FILE_CLOSE_REQ_PDU_ID: self._handle_file_close,
                FTP_CTRL_FILE_DELETE_REQ_PDU_ID: self._handle_file_delete,
            }
            cmd_id = msg[1]
            if cmd_id in handlers:
                handlers[cmd_id](msg[2:])
            else:
                self._log.warning('message received id:0x%04x unknown', cmd_id)

    def _send_control_now(self, payload, timer_id):
        _ = timer_id
        self.__config.hydra_protocol.send_ftp_server_to_firmware_control(payload)

    def _send_data_now(self, payload, timer_id):
        _ = timer_id
        self.__config.hydra_protocol.send_ftp_server_to_firmware_data(payload)

    def _send_control(self, payload):
        # self.__config.hydra_protocol.send_ftp_server_to_firmware_control(payload)
        # FIXME this is a workaround for a bug in current firmware implementation
        self.__config.uut.timer_add_relative(0.200, partial(self._send_control_now, payload))

    def _send_data(self, payload):
        # self.__config.hydra_protocol.send_ftp_server_to_firmware_data(payload)
        # FIXME this is a workaround for a bug in current firmware implementation
        self.__config.uut.timer_add_relative(0.200, partial(self._send_data_now, payload))

    def _add_session(self, address, port):
        '''
        Create a session for certain client.

        Sessions are per client, and the client is identified by the client address and port.
        If requested to create a session that already exists we just return the existing session

        Args:
            address (int): Client address
            port (int): Client port

        Returns:
            Session: Client session
        '''
        session_id = (address << 16) | port
        for sess in self.__data.session:
            if sess.session_id == session_id:
                break
        else:
            sess = Session(session_id, address, port)
            self.__data.session.append(sess)
        return sess

    def _get_session(self, session_id):
        for session in self.__data.session:
            if session.session_id == session_id:
                return session
        return None

    def _close_session(self, session_id):
        for ind, session in enumerate(self.__data.session):
            if session.session_id == session_id:
                del self.__data.session[ind]
                break
        else:
            raise RuntimeError('session_id:%s not found' % (session_id))

    def _add_handle(self, session_id, mode, filename, file_spec, client_address, client_port):
        handle = Handle(self.__data.last_handle + 1, session_id, mode, filename, file_spec,
                        client_address, client_port)
        self.__data.last_handle += 1
        self.__data.handle.append(handle)
        return handle

    def _get_handle(self, handle):
        for hnd in self.__data.handle:
            if hnd.handle == handle:
                return hnd
        return None

    def _close_handle(self, handle):
        for ind, hnd in enumerate(self.__data.handle):
            if hnd.handle == handle:
                del self.__data.handle[ind]
                break
        else:
            raise RuntimeError('handle:%s not found' % (handle))

    def _file_spec_to_filename(self, file_spec):
        '''
        Convert file specification into filename

        Firmware version file_spec[0-1]
        Number file_spec[2]
        Spare file_spec[3][3-0]
        Subsystem file_spec[3][7-4]
        Format file_spec[3][15-8]

        Args:
            file_spec (list[int,int.int.int]): File specification

        Returns:
            str: Filename
        '''
        fw_version = (file_spec[1] << 16) | file_spec[0]
        number = file_spec[2]
        subsystem = (file_spec[3] >> 4) & 0x000F
        fmt = (file_spec[3] >> 8) & 0x00FF
        format_str = FORMAT_STR[fmt]
        if fmt == FORMAT_PATCH:
            filename = '%ssubsys%s_%s%s_fw%08X.%s' % (
                self.__config.prefix, subsystem, format_str, number, fw_version, FILE_EXTENSION)
        else:
            filename = '%ssubsys%s_%s%s.%s' % (
                self.__config.prefix, subsystem, format_str, number, FILE_EXTENSION)
        return filename

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_session_create(self, payload):
        '''
        Process a FTP_CTRL_SESSION_CREATE_REQ_PDU_ID received message

        payload[0] FTP protocol version
        payload[1] Class flags
        payload[2][15:8] Client address
        payload[2][7:0] Client port
        payload[3] eFuse hash
        payload[4][15:8] Chip version
        payload[4][7:4] Chip variant
        payload[4][3:0] Chip revision

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 5:
            version = payload[0]
            flags = payload[1]
            client_address = payload[2] >> 8
            client_port = payload[2] & 0x00FF

            if version != CSR_FTP_PROTOCOL_VERSION_LATEST:
                self._log.warning('received session_create protocol version:%s invalid', version)
                result = CSR_FTP_FILE_OP_FAILURE  # no available result for error
                session_id = 0
            else:
                self._log.info('received session_create client address:%s port:%s', client_address,
                               client_port)
                session_id = self._add_session(client_address, client_port).session_id
                result = CSR_FTP_RESULT_SUCCESS

            spare = 0
            flags = CSR_FTP_SERVER_NON_VOLATILE | CSR_FTP_SERVER_LOCAL | CSR_FTP_SERVER_HOST
            self._send_control([
                FTP_CTRL_SESSION_CREATE_CFM_PDU_ID,
                session_id & 0xFFFF,
                (session_id >> 16) & 0xFFFF,
                flags,
                (SERVER_ADDRESS << 8) | SERVER_PORT_CONTROL,
                (spare << 8) | result])
        else:
            self._log.warning('received session_create length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_session_destroy(self, payload):
        '''
        Process a FTP_CTRL_SESSION_DESTROY_REQ_PDU_ID received message

        payload[0-1] Session id (little endian format)

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 2:
            session_id = payload[0] | (payload[1] << 16)
            self._log.info('received session_destroy session_id:%s', session_id)
            if self._get_session(session_id):
                self._close_session(session_id)

            # we are returning status ok always (as hydra protocol specification states)
            result = CSR_FTP_RESULT_SUCCESS
            spare = 0
            self._send_control([
                FTP_CTRL_SESSION_DESTROY_CFM_PDU_ID,
                session_id & 0xFFFF,
                (session_id >> 16) & 0xFFFF,
                (spare << 8) | result])
        else:
            self._log.warning('received session_destroy length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_file_open(self, payload):
        '''
        Process a FTP_CTRL_FILE_OPEN_REQ_PDU_ID received message

        payload[0-1] Session id (little endian format)
        payload[2] Mode
        payload[3-6] File spec
        payload[7][15:8] Client address
        payload[7][7:0] Client port

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 8:
            session_id = payload[0] | (payload[1] << 16)
            mode = payload[2]
            file_spec = payload[3:7]
            client_port = payload[7] & 0x00FF  # unused there is a special data message
            client_address = (payload[7] >> 8) & 0x00FF  # unused there is a special data message

            session = self._get_session(session_id)
            handle_id = INVALID_HANDLE_VALUE
            spare = 0
            size = 0
            mtu = MTU_DEFAULT
            if not session:  # session does not exist
                result = CSR_FTP_FILE_OP_NOT_ALLOWED
                self._log.warning('received file_open session:%s invalid', session_id)
            elif mode != CSR_FTP_OPEN_MODE_OPEN_EXISTING:  # only read support
                result = CSR_FTP_FILE_OP_READ_ONLY
                self._log.warning('received file_open mode:%s invalid', mode)
            else:
                filename = self._file_spec_to_filename(file_spec)
                if filename in self.__data.file_data:
                    handle_id = self._add_handle(session_id, mode, filename, file_spec,
                                                 client_address, client_port).handle
                    result = CSR_FTP_RESULT_SUCCESS
                    size = len(self.__data.file_data[filename])
                    self._log.info('received file_open filename:%s mode:%s', filename, mode)
                else:
                    self._log.warning('received open file:%s does not exist', filename)
                    result = CSR_FTP_FILE_OP_NOT_EXIST

            self._send_control([
                FTP_CTRL_FILE_OPEN_CFM_PDU_ID,
                (spare << 8) | result,
                handle_id & 0xFFFF,
                (handle_id >> 16) & 0xFFFF,
                size & 0xFFFF,
                (size >> 16) & 0xFFFF,
                mtu,
                (SERVER_ADDRESS << 8) | SERVER_PORT_DATA,
            ])
        else:
            self._log.warning('received file_open length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_file_read(self, payload):
        '''
        Process a FTP_CTRL_FILE_READ_REQ_PDU_ID received message

        payload[0-1] Handle (little endian format)
        payload[2] Bytes to read

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 3:
            handle_id = payload[0] | (payload[1] << 16)
            bytes_to_read = payload[2]

            spare = 0
            length = 0
            handle = self._get_handle(handle_id)
            reply = False
            if not handle:
                self._log.warning('received file_read handle:%s invalid', handle_id)
                result = CSR_FTP_FILE_OP_NOT_EXIST
                reply = True
            else:
                filename = handle.filename
                pos = handle.pos
                self._log.info('received file_read handle:%s length:%s', handle_id, bytes_to_read)

                if (pos + bytes_to_read) > len(self.__data.file_data[filename]):
                    length = len(self.__data.file_data[filename]) - pos
                else:
                    length = bytes_to_read

                if length:
                    # convert to 16-bit payload
                    data = self.__data.file_data[filename][pos:pos + length]
                    elems = int(len(data) / 2)
                    data = list(struct.unpack('<%sH' % elems, bytearray(data)))
                    self._send_data(data)
                    pos += length
                    handle.pos = pos

                    result = CSR_FTP_RESULT_SUCCESS
                    self.__data.pending = [
                        FTP_CTRL_FILE_READ_CFM_PDU_ID,
                        (spare << 8) | result,
                        handle_id & 0xFFFF,
                        (handle_id >> 16) & 0xFFFF,
                        length,
                    ]
                else:
                    result = CSR_FTP_FILE_OP_EOF
                    reply = True

            if reply:
                self._send_control([
                    FTP_CTRL_FILE_READ_CFM_PDU_ID,
                    (spare << 8) | result,
                    handle_id & 0xFFFF,
                    (handle_id >> 16) & 0xFFFF,
                    length,
                ])


        else:
            self._log.warning('received file_read length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_file_write(self, payload):
        '''
        Process a FTP_CTRL_FILE_WRITE_REQ_PDU_ID received message

        payload[0-1] Handle (little endian format)
        payload[2] Bytes to write

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 3:
            handle_id = payload[0] | (payload[1] << 16)
            # bytes_to_write = payload[2]
            self._log.warning('received file_write handle:%s unsupported', handle_id)
            spare = 0
            result = CSR_FTP_FILE_OP_READ_ONLY
            length = 0
            self._send_control([
                FTP_CTRL_FILE_WRITE_CFM_PDU_ID,
                (spare << 8) | result,
                handle_id & 0xFFFF,
                (handle_id >> 16) & 0xFFFF,
                length,
            ])
        else:
            self._log.warning('received file_write length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_file_seek(self, payload):
        '''
        Process a FTP_CTRL_FILE_SEEK_REQ_PDU_ID received message

        payload[0-1] Handle (little endian format)
        payload[2][15:8] Spare
        payload[2][7:0] Origin
        payload[3] Offset

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 4:
            handle_id = payload[0] | (payload[1] << 16)
            # apare = (payload[2] >> 8) & 0x00FF
            # origin = payload[2] & 0x00FF
            # offset = payload[3]
            self._log.warning('received file_seek handle:%s unsupported', handle_id)
            spare = 0
            result = CSR_FTP_FILE_OP_NOT_ALLOWED
            self._send_control([
                FTP_CTRL_FILE_SEEK_CFM_PDU_ID,
                (spare << 8) | result,
                handle_id & 0xFFFF,
                (handle_id >> 16) & 0xFFFF,
            ])
        else:
            self._log.warning('received file_seek length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_file_close(self, payload):
        '''
        Process a FTP_CTRL_FILE_CLOSE_REQ_PDU_ID received message

        payload[0-1] Handle (little endian format)

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 2:
            handle_id = payload[0] | (payload[1] << 16)
            self._log.info('received file_close handle:%s', handle_id)

            if self._get_handle(handle_id):
                self._close_handle(handle_id)

            spare = 0
            result = CSR_FTP_RESULT_SUCCESS
            self._send_control([
                FTP_CTRL_FILE_CLOSE_CFM_PDU_ID,
                (spare << 8) | result,
                handle_id & 0xFFFF,
                (handle_id >> 16) & 0xFFFF
            ])
        else:
            self._log.warning('received file_close length:%s invalid', len(payload))

    @log_input(logging.DEBUG, formatters={'payload': '0x%04x'})
    def _handle_file_delete(self, payload):
        '''
        Process a FTP_CTRL_FILE_DELETE_REQ_PDU_ID received message

        payload[0-1] Session id (little endian format)
        payload[2-5] File spec

        Args:
            payload (list[int]): Message payload words
        '''
        if len(payload) == 6:
            session_id = payload[0] | (payload[1] << 16)
            # file_spec = payload[2:6]
            spare = 0
            self._log.warning('received file_delete unsupported')
            result = CSR_FTP_DELETE_READ_ONLY
            self._send_control([
                FTP_CTRL_FILE_DELETE_CFM_PDU_ID,
                (spare << 8) | result,
                session_id & 0xFFFF,
                (session_id >> 16) & 0xFFFF,
            ])
        else:
            self._log.warning('received file_delete length:%s invalid', len(payload))
