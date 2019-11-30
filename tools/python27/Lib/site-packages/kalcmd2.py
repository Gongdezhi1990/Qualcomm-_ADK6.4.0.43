#/bin/python
# Copyright Qualcomm Technologies International Ltd. 2015-2017

from __future__ import print_function

import socket
import array
import struct


class kalcmd2_version(object):
    '''
        Describes the Major and minor version of Kalcmd2 supported
        by this version of the Kalcmd2 interface. Can be used
        to compare to version returned by Kalsim.
    '''
    MAJOR = 2
    MINOR = 8


class kalmsg_connection(object):
    '''
    A kalmsg_connection represents to connection to Kalsim. This class
    specifically handles the framing. In the 2.4 world this class only
    implements the framing. It now depends on subclass if kalsim or kalcmd2
    is the server.
    '''

    def __init__(self):
        None

    def send(self, msg):
        '''
        Send a single framed message to Kalsim.
        See http://wiki/Kalcmd2/protocol for a description of the protocol used.
        '''
        if (len(msg) / 256) > 255:
            raise ValueError('Trying to send something to big for Kalcmd2 protocol')
        write_array = bytearray([int(len(msg) / 256), len(msg) % 256]) + bytearray(msg)
        # This will always succeed in normal code flow, it will throw an
        # exception if something fishy happens
        self.socket.sendall(write_array)

    def receive(self):
        '''
        Receive a single framed message from Kalsim.
        See http://wiki/Kalcmd2/protocol for a description of the protocol used.
        '''
        # algorithm should be the same as the C-code
        # receive first two bytes, which are the framing header
        header = bytearray([0] * 2)
        self.socket.recv_into(header, 2)
        multiples = header[0]
        remainder = header[1]
        # receive the entire buffer
        read_length = (multiples * 256) + remainder
        to_read = read_length
        data = bytearray([0] * read_length)

        read = 0
        while to_read > 0:
            tmp = self.socket.recv(to_read)
            data[read:] = bytearray(tmp)
            num_bytes = len(tmp)
            if num_bytes == 0:
                return None
            to_read = to_read - num_bytes
            read += num_bytes
        cad = 'RX '
        for x in data:
            cad += '%02x ' % (x)
        import logging
        logging.getLogger(__name__).debug(cad)
        return data

    def receive2(self, command):
        data = self.receive()

        # Check if we are following the command pattern or the event pattern
        if data is None or len(data) < 2:
            raise Exception("Kalcmd2 ordering issue detected")
        # Check if the command received matches the command send.
        if data[1] != command:
            raise Exception("Command received doesn't match command send.")
        return data

    def close(self):
        '''
        Close the connection to Kalsim.
        '''
        self.socket.close()


class kalmsg_server_connection(kalmsg_connection):

    def __init__(self, accepted_socket):
        self.socket = accepted_socket


class kalmsg_client_connection(kalmsg_connection):
    '''
    A kalmsg_connection represents to connection to Kalsim. This class
    specifically handles the framing.
    '''

    def __init__(self, host="localhost", port=7522):
        '''
        Construct a new kalmsg_connection
        '''
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((host, port))


class kalcmd_events(object):
    '''
    The possible events that Kalsim can raise with Kalcmd2.
    '''
    NO_EVENT = 0
    INIT_EVENT = 1
    TIMER_EVENT = 2
    WATCHDOG_EVENT = 3
    MESSAGE_EVENT = 4
    STREAM_DONE_EVENT = 5
    TERMINATE_EVENT = 6
    STREAM_EOF_EVENT = 7
    STREAM_DATA_EVENT = 8
    KALSIM_ERROR_EVENT = 9
    KALIMBA_WANTS_ATTENTION_EVENT = 10


class kalcmd_cmds(object):
    '''
    Possible commands that Kalcmd2 can send to Kalsim.
    '''
    NO_CMD = 0
    NEXT_EVENT_CMD = 1
    SET_TIMER_REL_CMD = 2
    SET_TIMER_ABS_CMD = 3
    PEEK_CMD = 4
    POKE_CMD = 5
    INTERRUPT_CMD = 6
    SEND_MESSAGE_CMD = 7
    GET_TIME_CMD = 8
    TERMINATE_CMD = 9
    STREAM_CHANGE_CMD = 10
    STREAM_GET_INFO_CMD = 11
    GET_MESSAGE_CMD = 12
    GET_KAL_ARCH = 13
    GET_KALIMBA_NAME = 14
    GET_VERSION = 15
    KALCMD_FLAG_CMD = 16
    GET_LAST_TIMER_COOKIE = 17
    # 2.1 New api's
    STREAM_CREATE_CMD = 18
    STREAM_DESTROY_CMD = 19
    STREAM_LIST_CMD = 20
    STREAM_QUERY_PROPERTY_CMD = 21
    STREAM_CHANGE_PROPERTY_CMD = 22
    STREAM_SUBMIT_CHANGES_CMD = 23
    STREAM_INJECT_CMD = 24
    STREAM_INTERCEPT_CMD = 25
    STREAM_INDUCE_CMD = 26
    STREAM_EOF_EVENT_QUERY_CMD = 27
    STREAM_DATA_EVENT_QUERY_CMD = 28
    STREAM_REWIND_CMD = 29
    GET_KALCMD_FLAG_CMD = 30
    SAME_BUFFER_CMD = 31
    GET_BUFFER_SIZE_CMD = 32
    GET_BUFFER_OFFSET_CMD = 33
    GET_BUFFER_SAMPLE_SIZE_CMD = 34
    # 2.3 New api's
    QUERY_STATS_CMD = 35
    CHANGE_MAX_CLOCK_SPEED = 36
    QUERY_MAX_CLOCK_SPEED = 37
    # 2.4 New api's
    QUERY_DSP_REG_CMD = 38
    QUERY_KALSIM_VERSION_CMD = 39
    # 2.5 New api's
    BLOCK_MEM_WRITE_CMD = 40
    BLOCK_MEM_READ_CMD = 41
    # 2.6 New api's
    PRINT_CMD = 42
    # 2.8 New api's
    STREAM_FLUSH = 43

class kalmem_spaces(object):
    '''
    Possible memory spaces that Kalcmd2 can use when peeking and poking Kalsim.
    There is some overlap when describing different memory types.
    '''
    PM = 0
    DM = 1
    CRESCENDO_ROM = 2
    CRESCENDO_SQIF = 3
    CRESCENDO_SRAM = 4
    A7DA_AXI = 2
    NAPIER_AHB = 5
    # BAC windows for Crescendo derivatives
    BAC_WINDOW_0 = 6
    BAC_WINDOW_1 = 7
    BAC_WINDOW_2 = 8
    BAC_WINDOW_3 = 9


class kalcmd_response(object):
    '''
    Possible responses Kalsim can send to Kalcmd2.
    '''
    KALCMD_OK = 0
    KALCMD_ERROR = 1
    KALCMD_UNSUPPORTED = 2
    KALCMD_INVALID_KEY = 3
    KALCMD_INVALID_VALUE = 4
    KALCMD_STREAM_FILE_ISSUE = 5


class kal_widths(object):
    '''
    Possible widths to use when peeking and poking.
    '''
    EIGHT_BIT = 0
    SIXTEEN_BIT = 1
    TWENTYFOUR_BIT = 2
    THIRTYTWO_BIT = 3


class kalcmd_stat(object):
    '''
    Possible stat event horizons
    '''
    WATCHDOG = 0
    TOTAL = 1


class kal_regs(object):
    REG_B0 = 0
    REG_B1 = 1
    REG_B4 = 2
    REG_B5 = 3
    REG_FP = 4
    REG_I0 = 5
    REG_I1 = 6
    REG_I2 = 7
    REG_I3 = 8
    REG_I4 = 9
    REG_I5 = 10
    REG_I6 = 11
    REG_I7 = 12
    REG_L0 = 13
    REG_L1 = 14
    REG_L4 = 15
    REG_L5 = 16
    REG_M0 = 17
    REG_M1 = 18
    REG_M2 = 19
    REG_M3 = 20
    REG_NUM_INSTRS = 21
    REG_NUM_RUNCLKS = 22
    REG_NUM_STALLS = 23
    REG_R0 = 24
    REG_R1 = 25
    REG_R10 = 26
    REG_R2 = 27
    REG_R3 = 28
    REG_R4 = 29
    REG_R5 = 30
    REG_R6 = 31
    REG_R7 = 32
    REG_R8 = 33
    REG_R9 = 34
    REG_RFLAGS = 35
    REG_RLINK = 36
    REG_RMAC0 = 37
    REG_RMAC1 = 38
    REG_RMAC2 = 39
    REG_RMAC24 = 40
    REG_RMACB0 = 41
    REG_RMACB1 = 42
    REG_RMACB2 = 43
    REG_RMACB24 = 44
    REG_SP = 45
    REG_PC = 46


class kalcmd_msg_header(object):
    '''
    Base class for the platform specific header. In principle any class that
    implements this needs to provide a way to generate a header in octets and
    provide the length of the header.
    '''

    def __init__(self):
        pass

    def length(self):
        '''
        Returns the length of the header in octets when implemented.
        '''
        raise NotImplementedError

    def header(self):
        '''
        Returns the actual header when implemented.
        '''
        raise NotImplementedError


class kalcmd_hydra_msg_header(kalcmd_msg_header):
    '''
    Specific implementation for Hydra simplified messaging. We only need to
    provide the subsystem as metadata.
    '''

    def __init__(self, subsystem):
        '''
        Creates a hydra message header
        '''
        kalcmd_msg_header.__init__(self)
        self.subsystem = subsystem

    def length(self):
        '''
        Returns the length of the header in octets.
        '''
        return 1

    def header(self):
        '''
        Returns the header in serialized form.
        '''
        header = [self.subsystem]
        return header


class kalcmd_bluecore_msg_header(kalcmd_msg_header):
    '''
    Specific implementation for BlueCore messaging. At the moment it represent
    the message id sent along with the actual message.
    '''

    def __init__(self, msg_id):
        '''
        Construct a BlueCore message header.
        '''
        kalcmd_msg_header.__init__(self)
        self.msg_id = msg_id

    def length(self):
        '''
        Returns the length of the header in octets.
        '''
        return 2

    def header(self):
        '''
        Returns the header.
        '''
        header = [self.msg_id & 0xFF, (self.msg_id & 0xFF00) >> 8]
        return header

class kalcmd_napier_msg_header(kalcmd_msg_header):
    '''
    Specific implementation for Napier messaging. At the moment it represent 
    the message id sent along with the actual message.
    '''
    def __init__(self, msg_id):
        '''
        Construct a Napier message header. 
        '''
        kalcmd_msg_header.__init__(self)
        self.msg_id = msg_id;
        
    def length(self):
        '''
        Returns the length of the header in octets.
        '''
        return 2
        
    def header(self):
        '''
        Returns the header.
        '''
        header = [self.msg_id & 0xFF, (self.msg_id & 0xFF00 >> 8)]
        return header

class kalcmd(object):
    '''
    The kalcmd class provides the methods to talk to Kalsim. It only requires
    knowledge of the connection.

    All functions will return a dict that at least contains a return_code.
    For queries it will return the data along the return_code.
    '''

    def __init__(self, connection):
        '''
        Create an instance of the Kalcmd using a connection instance.
        '''
        self.first_run = True
        self.terminated = False
        self.con = connection

    def __receive_event_response(self):
        data = self.con.receive()

        if data is None or len(data) > 1:
            raise Exception("Kalcmd2 ordering issue detected, while receiving an event.")
        return data

    def __receive_cmd_response(self, command):
        data = self.con.receive()

        # Check if we are following the command pattern or the event pattern
        if data is None or len(data) < 2:
            raise Exception("Kalcmd2 ordering issue detected, while sending a command.")
        # Check if the command received matches the command sent.
        if data[1] != command:
            raise Exception("Command received doesn't match command sent.")
        return data

    def get_next_event(self):
        '''
         Returns the next event, block until event has been produced by
         Kalsim. Event_code can be found in the event key of the dict.
        '''
        if (self.first_run):
            data = self.__receive_event_response()
            self.first_run = False
        else:
            send_buf = bytearray([kalcmd_cmds.NEXT_EVENT_CMD])
            self.con.send(send_buf)
            data = None
            try:
                data = self.__receive_event_response()
            except socket.error:
                data = [kalcmd_events.KALSIM_ERROR_EVENT]

        # FIXME mapd replaced if data is None:
        if not data:
            return {'return_code': 1}
        else:
            return {'return_code': kalcmd_response.KALCMD_OK, 'event': data[0]}

    def get_received_message(self):
        '''
        If the previous event was a message, this method will return the message
        sent. If no message was sent, this method will return an error.

        Message is returned as a dict, which contains the error code, the
        platform specific header and the the payload.

        return_type: Type of message to return. "list" and "string" are
                     available.
        returns:  A dict with the message in the desired form under payload with
                  the platform specific header under the key header.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_MESSAGE_CMD])
        self.con.send(send_buf)
        temp_buf = self.__receive_cmd_response(kalcmd_cmds.GET_MESSAGE_CMD)
        if temp_buf is None:
            return {'return_code': 1}

        if len(temp_buf) > 2:
            plat_header_length = temp_buf[2]
            header = []
            for i in range(0, plat_header_length):
                header.insert(i, temp_buf[4 + i])
            payload_length = temp_buf[3]
            payload = None
            payload = []
            for i in range(0, payload_length):
                payload.insert(i, (temp_buf[4 + plat_header_length + (2 * i)]) |
                                   (temp_buf[5 + plat_header_length + (2 * i)]) << 8)
            return {'return_code': kalcmd_response.KALCMD_OK, 'header': header, 'payload': payload}
        else:
            return {'return_code': temp_buf[0]}

    def get_time(self):
        '''
        Returns the current time simulated time.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_TIME_CMD])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_TIME_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}

        time = struct.unpack('q', recv_buffer[2:])
        return {'return_code': recv_buffer[0], 'time': time[0]}

    def terminate(self, return_code):
        '''
        Terminates Kalsim.
        '''
        send_buf = bytearray([kalcmd_cmds.TERMINATE_CMD, return_code])
        self.con.send(send_buf)
        return {'return_code': kalcmd_response.KALCMD_OK}

    def send_message(self, platform_header, data):
        '''
        Send a message to the Kalsim. Only returns the response. Accepts both
        lists and strings. Octet strings are packed into 16 bit words, while
        only the lower 16 bits of list items are sent.
        '''
        header_length = platform_header.length() + 3
        header = bytearray([0] * header_length)
        header[0] = kalcmd_cmds.SEND_MESSAGE_CMD
        header[1] = platform_header.length()
        header[2] = 0xCB  # To be filled in later
        # Copy platform specific header
        temp_plat_header = platform_header.header()
        for i in range(0, platform_header.length()):
            header[3 + i] = temp_plat_header[i]

        temp_buf = bytearray([0] * len(data) * 2)
        if isinstance(data, str):
            for i in range(0, len(data)):
                temp_buf[2 * i] = data[i]
                temp_buf[2 * i + 1] = 0
            header[2] = len(data)
        elif isinstance(data, list) or isinstance(data, bytearray):
            for i in range(0, len(data)):
                temp_buf[2 * i] = data[i] & 0xFF
                temp_buf[2 * i + 1] = (data[i] & 0xFF00) >> 8
            header[2] = len(data)
        else:
            return {'return_code': 1}
        payload_length = header[2] * 2
        total_length = header_length + payload_length
        send_buffer = bytearray([0] * total_length)
        # copy header
        for i in range(0, header_length):
            send_buffer[i] = header[i]
        # copy payload
        for i in range(0, payload_length):
            send_buffer[header_length + i] = temp_buf[i]
        rc = self.con.send(send_buffer)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.SEND_MESSAGE_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def set_timer_abs(self, timer, cookie=0):
        '''
        Set up an absolute timer in simulated time. By default the cookie is 0
        unless otherwise specified.
        '''
        send_buf = bytearray([kalcmd_cmds.SET_TIMER_ABS_CMD])
        send_buf += bytearray(struct.pack('qI', timer, cookie))
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.SET_TIMER_ABS_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def set_timer_rel(self, timer, cookie=0):
        '''
        Set up a relative timer to the current simulated time. By default the
        cookie is 0 unless otherwise specified.
        '''
        send_buf = bytearray([kalcmd_cmds.SET_TIMER_REL_CMD])
        send_buf += bytearray(struct.pack('qI', timer, cookie))
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.SET_TIMER_REL_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def peek(self, memory_space, address, size):
        '''
        Peek at a memory location in a memory space.
        '''
        send_buf = bytearray([0] * 7)
        send_buf[0] = kalcmd_cmds.PEEK_CMD
        send_buf[1] = size
        send_buf[2] = memory_space
        send_buf[3] = address & 0xFF
        send_buf[4] = (address >> 8) & 0xFF
        send_buf[5] = (address >> 16) & 0xFF
        send_buf[6] = (address >> 24) & 0xFF

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.PEEK_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            value = (recv_buffer[2] | (recv_buffer[3] << 8) |
                     (recv_buffer[4] << 16) | (recv_buffer[5] << 24))
            return {'return_code': recv_buffer[0], 'value': value}

    def poke(self, memory_space, address, size, value):
        '''
        Poke a memory location in a memory space.
        '''
        send_buf = bytearray([0] * 11)
        send_buf[0] = kalcmd_cmds.POKE_CMD
        send_buf[1] = size
        send_buf[2] = memory_space
        send_buf[3] = address & 0xFF
        send_buf[4] = (address >> 8) & 0xFF
        send_buf[5] = (address >> 16) & 0xFF
        send_buf[6] = (address >> 24) & 0xFF
        send_buf[7] = value & 0xFF
        send_buf[8] = (value >> 8) & 0xFF
        send_buf[9] = (value >> 16) & 0xFF
        send_buf[10] = (value >> 24) & 0xFF
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.POKE_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def interrupt(self, interrupt):
        '''
        Trigger an interrupt on the Kalimba.
        '''
        send_buf = bytearray([kalcmd_cmds.INTERRUPT_CMD, interrupt])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.INTERRUPT_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def get_kal_arch(self):
        '''
        Get the architecture number of the simulated Kalimba.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_KAL_ARCH])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_KAL_ARCH)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0], 'kal_arch': recv_buffer[2]}

    def get_kalimba_name(self):
        '''
        Returns the name of the Kalimba currently being simulated.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_KALIMBA_NAME])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_KALIMBA_NAME)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            kalimba_name = str(recv_buffer[2:].decode('utf-8'))
            return {'return_code': recv_buffer[0], 'name': kalimba_name}

    def get_version(self):
        '''
        Returns the major and minor version of the current version of Kalcmd2.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_VERSION])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_VERSION)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            major = recv_buffer[2]
            minor = recv_buffer[3]
            return {'return_code': recv_buffer[0], 'major': major, 'minor': minor}

    def get_last_timer_cookie(self):
        '''
        Returns the last cookie attached to a timer.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_LAST_TIMER_COOKIE])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_LAST_TIMER_COOKIE)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            cookie = struct.unpack('I', recv_buffer[2:])
            return {'return_code': recv_buffer[0], 'cookie': cookie[0]}

    def stream_create(self):
        '''
        Create a new stream instance inside of Kalsim.
        '''
        send_buf = bytearray([kalcmd_cmds.STREAM_CREATE_CMD])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_CREATE_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0], 'stream_id': recv_buffer[2]}

    def streams_list(self):
        '''
        List all the streams present in the Kalsim at the current time.
        '''
        send_buf = bytearray([kalcmd_cmds.STREAM_LIST_CMD])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_LIST_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            # print recv_buffer
            stream_ids = []
            for i in range(0, recv_buffer[2]):
                stream_ids.insert(i, recv_buffer[3 + i])
            return {'return_code': recv_buffer[0], 'stream_ids': stream_ids}

    def stream_destroy(self, stream_id):
        '''
        Destroy a stream instance inside of Kalsim.
        '''
        send_buf = bytearray([kalcmd_cmds.STREAM_DESTROY_CMD, stream_id])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_DESTROY_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def stream_query_property(self, stream_id, key):
        '''
        Query a stream propery when provided with a key.(Key is a string)
        '''
        send_buf = bytearray([0] * (len(key) + 3))
        send_buf[0] = kalcmd_cmds.STREAM_QUERY_PROPERTY_CMD
        send_buf[1] = stream_id
        send_buf[2] = len(key)
        for i in range(0, len(key)):
            send_buf[3 + i] = ord(key[i])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_QUERY_PROPERTY_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            value = str(recv_buffer[3:].decode('utf-8'))
            return {'return_code': recv_buffer[0], 'key': key, 'value': value}

    def stream_change_property(self, stream_id, key, value):
        '''
        Change a stream property of a given key to given value.
        '''
        send_buf = bytearray([0] * (len(key) + len(value) + 4))
        send_buf[0] = kalcmd_cmds.STREAM_CHANGE_PROPERTY_CMD
        send_buf[1] = stream_id
        send_buf[2] = len(key)
        send_buf[3] = len(value)
        for i in range(0, len(key)):
            send_buf[4 + i] = ord(key[i])

        for i in range(0, len(value)):
            send_buf[4 + len(key) + i] = ord(value[i])

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_CHANGE_PROPERTY_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def stream_commit_changes(self, stream_id):
        '''
        Submit all the changes currently pending.
        '''
        send_buf = bytearray([kalcmd_cmds.STREAM_SUBMIT_CHANGES_CMD, stream_id])

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_SUBMIT_CHANGES_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0], "num_of_warnings": recv_buffer[2]}

    '''
        Insert samples into a stream. With type being the width of the samples
        to insert and the samples being a Python list.
    '''

    def stream_insert(self, stream_id, type, samples):
        bufsize = 0
        if type == "8":
            bufsize = len(samples) + 2
        elif type == "16":
            bufsize = 2 * len(samples) + 2
        elif type == "24":
            bufsize = 3 * len(samples) + 2
        elif type == "32":
            bufsize = 4 * len(samples) + 2
        send_buf = bytearray([0] * bufsize)
        # print "bufsize: {0} samples : {1}".format(len(send_buf), len(samples))

        # pack data into the right format
        # print samples
        for i in range(0, len(samples)):
            # print samples[i]
            if type == "8":
                sample = samples[i]
                if sample < 0:
                    sample = sample | 0x80
                send_buf[i + 2] = sample & 0xFF
            elif type == "16":
                sample = samples[i]
                if sample < 0:
                    sample = sample | 0x8000
                send_buf[2 * i + 2] = sample & 0xFF
                send_buf[2 * i + 3] = (sample >> 8) & 0xFF
                # print "{0} {1}".format(ord(send_buf[2 * i + 3]),ord(send_buf[2 * i + 2]))
            elif type == "24":
                sample = samples[i]
                if sample < 0:
                    sample = sample & 0x800000
                send_buf[3 * i + 2] = (sample & 0xFF)
                send_buf[3 * i + 3] = (sample >> 8) & 0xFF
                send_buf[3 * i + 4] = (sample >> 16) & 0xFF
            elif type == "32":
                sample = samples[i]
                if sample < 0:
                    sample = sample & 0x80000000

                send_buf[4 * i + 2] = sample & 0xFF
                send_buf[4 * i + 3] = (sample >> 8) & 0xFF
                send_buf[4 * i + 4] = (sample >> 16) & 0xFF
                send_buf[4 * i + 5] = (sample >> 24) & 0xFF
        send_buf[0] = kalcmd_cmds.STREAM_INJECT_CMD
        send_buf[1] = stream_id
        # print send_buf
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_INJECT_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def stream_extract(self, stream_id, num_of_samples, sign_extend):
        '''
        Extract a given number of samples from a stream.
        '''
        send_buf = bytearray([0] * 6)
        send_buf[0] = kalcmd_cmds.STREAM_INTERCEPT_CMD
        send_buf[1] = stream_id
        send_buf[2] = num_of_samples & 0xFF
        send_buf[3] = (num_of_samples >> 8) & 0xFF
        send_buf[4] = (num_of_samples >> 16) & 0xFF
        send_buf[5] = (num_of_samples >> 24) & 0xFF

        self.con.send(send_buf)
        # print "send"
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_INTERCEPT_CMD)
        # print "recv"
        if recv_buffer is None:
            print("recv buffer is null")
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            # based on the format and if we want to sign extend we need to
            # unpack the enormous buffer thing we just received
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}

            stream_format = recv_buffer[2]
            expected = 0
            # without the header
            num_of_bytes_received = len(recv_buffer) - 3
            format = ' '
            # print "stream_format: {0}", stream_format
            if stream_format == kal_widths.EIGHT_BIT:
                format = "8"
                expected = num_of_samples
            elif stream_format == kal_widths.SIXTEEN_BIT:
                format = '16'
                expected = 2 * num_of_samples
            elif stream_format == kal_widths.TWENTYFOUR_BIT:
                format = '24'
                expected = 3 * num_of_samples
            elif stream_format == kal_widths.THIRTYTWO_BIT:
                format = '32'
                expected = 4 * num_of_samples
            else:
                # nonesense returned
                print("silly format {0}".format(stream_format))
                return {'return_code': 1}

            # Amount of data does not match expectations
            if expected != num_of_bytes_received:
                print("expected {0} {1}".format(expected, num_of_bytes_received))
                return {'return_code': 1}

            samples = [0] * num_of_samples

            for i in range(0, num_of_samples):
                sample = 0
                if stream_format == kal_widths.EIGHT_BIT:
                    sample = recv_buffer[3 + i]
                    if sign_extend:
                        signext_mask_8bit = -1 << 7
                        sample = ((sample & 0xFFFF) + signext_mask_8bit) ^ signext_mask_8bit
                elif stream_format == kal_widths.SIXTEEN_BIT:
                    sample = (recv_buffer[4 + i * 2]) << 8 | (recv_buffer[3 + i * 2])
                    if sign_extend:
                        signext_mask_16bit = -1 << 15
                        sample = ((sample & 0xFFFF) + signext_mask_16bit) ^ signext_mask_16bit
                elif stream_format == kal_widths.TWENTYFOUR_BIT:
                    sample = (
                        recv_buffer[
                            5 +
                            i *
                            3] << 16) | (
                        recv_buffer[
                            4 +
                            i *
                            3] << 8) | (
                        recv_buffer[
                            3 +
                            i *
                            3])
                    if sign_extend:
                        signext_mask_24bit = -1 << 24
                        sample = ((sample & 0xFFFFFF) + signext_mask_24bit) ^ signext_mask_24bit
                elif stream_format == kal_widths.THIRTYTWO_BIT:
                    sample = (recv_buffer[6 + i * 4] << 24) | (recv_buffer[5 + i * 4] << 16) | (
                        recv_buffer[4 + i * 4] << 8) | (recv_buffer[3 + i * 4])

                    if sign_extend and sample > 0x80000000:
                        sample = sample - 0x80000000
                        sample = -1 * sample

                samples[i] = sample

            return {'return_code': recv_buffer[0], 'samples': samples, 'format': format}

        return {'return_code': kalcmd_response.KALCMD_ERROR}

    def stream_induce(self, stream_id, num_of_samples):
        '''
        Induce a stream to generate or consume some samples from a file. Only
        works when the stream is file backed.
        '''
        send_buf = bytearray([0] * 6)
        send_buf[0] = kalcmd_cmds.STREAM_INDUCE_CMD
        send_buf[1] = stream_id
        send_buf[2] = num_of_samples & 0xFF
        send_buf[3] = (num_of_samples >> 8) & 0xFF
        send_buf[4] = (num_of_samples >> 16) & 0xFF
        send_buf[5] = (num_of_samples >> 24) & 0xFF

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_INDUCE_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if len(recv_buffer) == 6:
                samples_induced = (recv_buffer[2] | (recv_buffer[3] << 8) |
                                   (recv_buffer[4] << 16) | (recv_buffer[5] << 24))
                return {'return_code': recv_buffer[0], 'samples_induced': samples_induced}
            else:
                return {'return_code': recv_buffer[0]}

    def stream_eof_event_query(self):
        '''
        Return which stream caused the EOF event.
        '''
        send_buf = bytearray([kalcmd_cmds.STREAM_EOF_EVENT_QUERY_CMD])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_EOF_EVENT_QUERY_CMD)

        if recv_buffer is None or recv_buffer[0] != 0:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0], 'stream_id': recv_buffer[2]}

    def stream_data_event_query(self):
        '''
        Returns information about the stream which triggered a data event.
        returns: The dict contains the stream_id that caused the event and the
                 num_of_samples requested.
        '''
        send_buf = bytearray([0] * 6)
        send_buf[0] = kalcmd_cmds.STREAM_DATA_EVENT_QUERY_CMD

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_DATA_EVENT_QUERY_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                stream_id = recv_buffer[2]
                num_of_samples = struct.unpack('I', recv_buffer[3:])
                return {'return_code': 0, 'stream_id': stream_id,
                        'num_of_samples': num_of_samples[0]}

    def stream_rewind(self, stream_id):
        '''
        Rewind a file backed stream to the beginning if reading. Requires the
        stream to be file backed.
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.STREAM_REWIND_CMD
        send_buf[1] = stream_id

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_REWIND_CMD)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def get_flag_state(self, flag):
        '''
        Returns the state of the queried flag.
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.GET_KALCMD_FLAG_CMD
        send_buf[1] = flag

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_KALCMD_FLAG_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            return {'return_code': recv_buffer[0], 'flag:': recv_buffer[2]}

    def handles_to_same_buffer(self, handle):
        '''
        Returns the handles in the system that point to the same buffer.
        '''
        send_buf = bytearray([kalcmd_cmds.SAME_BUFFER_CMD, handle])
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.SAME_BUFFER_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': ord(recv_buffer[0])}
            else:
                length = recv_buffer[2]
                handle_ids = []
                for i in range(0, length):
                    handle_ids.insert(i, recv_buffer[3 + i])
                return {'return_code': recv_buffer[0], 'handle_ids': handle_ids}

    def get_buffer_size(self, handle):
        '''
        Returns the size of a buffer in words.
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.GET_BUFFER_SIZE_CMD
        send_buf[1] = handle

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_BUFFER_SIZE_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                size = ((recv_buffer[2]) | (recv_buffer[3] << 8) |
                        (recv_buffer[4] << 16) | (recv_buffer[5] << 24))
                return {'return_code': recv_buffer[0], 'size': size}

    def get_handle_offset(self, handle):
        '''
        Returns the current offset inside the buffer pointed to by handle.
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.GET_BUFFER_OFFSET_CMD
        send_buf[1] = handle

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_BUFFER_OFFSET_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                offset = ((recv_buffer[2]) | (recv_buffer[3] << 8) |
                          (recv_buffer[4] << 16) | (recv_buffer[5] << 24))
                return {'return_code': recv_buffer[0], 'offset': offset}

        return {'return_code': 1}

    def get_handle_sample_size(self, handle):
        '''
        Returns the size of sample for a buffer.
        '''
        send_buf = bytearray([kalcmd_cmds.GET_BUFFER_SAMPLE_SIZE_CMD, handle])

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.GET_BUFFER_SAMPLE_SIZE_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                return {'return_code': recv_buffer[0], 'sample_size': recv_buffer[2]}

    def wave_flag(self, flag):
        '''
        Wave to a flag in a portscript.
        '''
        send_buf = bytearray([kalcmd_cmds.KALCMD_FLAG_CMD, flag])

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.KALCMD_FLAG_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def query_stats(self, key, watchdog):
        '''
        Query a stream propery when provided with a key.(Key is a string)
        '''
        send_buf = bytearray([0] * (len(key) + 3))
        send_buf[0] = kalcmd_cmds.QUERY_STATS_CMD
        send_buf[1] = watchdog
        send_buf[2] = len(key)
        
        for i in range(0, len(key)):
            send_buf[3 + i] = ord(key[i])
        
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.QUERY_STATS_CMD)
        if recv_buffer is None:
            return {'return_code':kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code':recv_buffer[0]}
            else:
                value = str(recv_buffer[3:].decode('utf-8'))
                return {'return_code':recv_buffer[0], 'key':key, 'value':value}

    def change_max_clockspeed(self, clock_speed):
        '''
        Change the maximum clock speed of the simulated Kalimba.
        '''
        send_buf = bytearray([0] * 5)
        send_buf[0] = kalcmd_cmds.CHANGE_MAX_CLOCK_SPEED
        send_buf[1] = clock_speed & 0xFF
        send_buf[2] = (clock_speed & 0xFF00) >> 8
        send_buf[3] = (clock_speed & 0xFF0000) >> 16
        send_buf[4] = (clock_speed & 0xFF000000) >> 24
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.CHANGE_MAX_CLOCK_SPEED)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def query_max_clock_speed(self):
        '''
        Query the maximum clock speed of the simulated Kalimba.
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.QUERY_MAX_CLOCK_SPEED
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.QUERY_MAX_CLOCK_SPEED)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                clock_speed = (
                    recv_buffer[2]) | (
                    recv_buffer[3] << 8) | (
                    recv_buffer[4] << 16) | (
                    recv_buffer[5] << 24)
                return {'return_code': recv_buffer[0], 'clock_speed': clock_speed}

    def query_dsp_reg(self, register):
        '''
        Query a DSP register
        '''
        send_buf = bytearray([0] * 5)
        send_buf[0] = kalcmd_cmds.QUERY_DSP_REG_CMD
        send_buf[1] = register & 0xFF
        send_buf[2] = (register >> 8) & 0xFF
        send_buf[3] = (register >> 16) & 0xFF
        send_buf[4] = (register >> 24) & 0xFF
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.QUERY_DSP_REG_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                value = (recv_buffer[2] | (recv_buffer[3] << 8) |
                         (recv_buffer[4] << 16) | (recv_buffer[5] << 24))
                return {'return_code': recv_buffer[0], 'value': value}

    def get_kalsim_version(self):
        '''
            Get some information about the version of Kalsim on the other side.
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.QUERY_KALSIM_VERSION_CMD
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.QUERY_KALSIM_VERSION_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            # determine if this is Kalsim test version
            release_version = (recv_buffer[2] == 1) if True else False
            # read the lengths
            branch_length = recv_buffer[3]
            major_length = recv_buffer[4]
            minor_length = recv_buffer[5]
            date_length = recv_buffer[6]
            # read out the strings
            branch_index = 7
            major_index = branch_index + branch_length
            minor_index = major_index + major_length
            date_index = minor_index + minor_length
            end_index = date_index + date_length
            branch = str(recv_buffer[branch_index:major_index - 1].decode('utf-8'))
            major = str(recv_buffer[major_index:minor_index - 1].decode('utf-8'))
            minor = str(recv_buffer[minor_index:date_index - 1].decode('utf-8'))
            build_date = str(recv_buffer[date_index: end_index - 1].decode('utf-8'))
            return {'return_code': recv_buffer[
                0], 'release_version': release_version, 'branch': branch, 'major': major, 'minor': minor, 'build_date': build_date}

    def block_mem_write(self, mem_space, address, words):
        '''
            Write a block of len(words) starting at address in memspace.
        '''
        buf_size = len(words) * 4 + 6
        send_buf = bytearray([0] * buf_size)
        send_buf[0] = kalcmd_cmds.BLOCK_MEM_WRITE_CMD
        send_buf[1] = mem_space
        send_buf[2] = address & 0xFF
        send_buf[3] = (address >> 8) & 0xFF
        send_buf[4] = (address >> 16) & 0xFF
        send_buf[5] = (address >> 24) & 0xFF
        i = 0

        for word in words:
            send_buf[4 * i + 6] = word & 0xFF
            send_buf[4 * i + 7] = (word >> 8) & 0xFF
            send_buf[4 * i + 8] = (word >> 16) & 0xFF
            send_buf[4 * i + 9] = (word >> 24) & 0xFF
            i = i + 1
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.BLOCK_MEM_WRITE_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                return {'return_code': 0}

    def block_mem_read(self, mem_space, address, num_words):
        '''
            Read a block of len(words) starting at address from memspace
        '''
        send_buf = bytearray([0] * 8)
        send_buf[0] = kalcmd_cmds.BLOCK_MEM_READ_CMD
        send_buf[1] = mem_space
        send_buf[2] = address & 0xFF
        send_buf[3] = (address >> 8) & 0xFF
        send_buf[4] = (address >> 16) & 0xFF
        send_buf[5] = (address >> 24) & 0xFF
        send_buf[6] = num_words & 0xFF
        send_buf[7] = (num_words >> 8) & 0xFF
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.BLOCK_MEM_READ_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            if recv_buffer[0] != 0:
                return {'return_code': recv_buffer[0]}
            else:
                num_samples = int((len(recv_buffer) - 1) / 4)
                words = [0] * num_samples

                for i in range(num_samples):
                    sample = ((recv_buffer[5 + i * 4]) << 24 |
                              (recv_buffer[4 + i * 4]) << 16 |
                              (recv_buffer[3 + i * 4]) << 8 |
                              (recv_buffer[2 + i * 4]))
                    words[i] = sample
                return {'return_code': 0, 'words': words}

    def print_message(self, message):
        '''
            Print a message in the Kalsim terminal
        '''
        send_buf = bytearray([0] * (1 + len(message)))
        send_buf[0] = kalcmd_cmds.PRINT_CMD
        i = 1
        for c in message:
            send_buf[i] = ord(c)
            i = i + 1
        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.PRINT_CMD)
        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}

    def stream_flush(self, stream_id):
        '''
            Flush the stream desginated with the stream_id. Accepts
            all kinds of streams however, it will only actually flush
            output streams to files. 
        '''
        send_buf = bytearray([0] * 2)
        send_buf[0] = kalcmd_cmds.STREAM_FLUSH
        send_buf[1] = stream_id

        self.con.send(send_buf)
        recv_buffer = self.__receive_cmd_response(kalcmd_cmds.STREAM_FLUSH)

        if recv_buffer is None:
            return {'return_code': kalcmd_response.KALCMD_ERROR}
        else:
            return {'return_code': recv_buffer[0]}
