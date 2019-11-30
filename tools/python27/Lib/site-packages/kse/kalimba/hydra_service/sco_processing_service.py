'''
Hydra SCO Processing Services
'''

import argparse
import logging
from functools import partial

from kalcmd2 import kalmem_spaces

from kats.framework.library.docstring import inherit_docstring
from kats.framework.library.log import log_input
from kats.framework.library.schema import DefaultValidatingDraft4Validator
from kats.kalimba.hydra_service.constants import SERVICE_TYPE_SCO_PROCESSING, \
    TIMER_TIME, SCO_PROC_SERV_RUN_STATE_MSG, SCO_DIR_TO_AIR, SCO_DIR_FROM_AIR
from kats.library.registry import get_instance

TIMESLOT_DURATION = 0.000625
TIMESLOT_DURATION_US = 625
TIMESLOT_FRACTION = 400
WALLCLOCK_UPDATE_PERIOD = 0.250
DUMMY_ANTI_TEAR_COUNTER = 0x4B0

SERVICE_TAG = 'service_tag'
HCI_HANDLE = 'hci_handle'
NEXT_SLOT_TIME = 'next_slot_time'
NEXT_SLOT_TIME_DEFAULT = 0
WALLCLOCK_ACCURACY = 'wallclock_accuracy'
WALLCLOCK_ACCURACY_DEFAULT = 0
WALLCLOCK_HANDLE = 'wallclock_handle'
WALLCLOCK_HANDLE_DEFAULT = 0x254
WALLCLOCK_OFFSET = 'wallclock_offset'
WALLCLOCK_OFFSET_DEFAULT = 0
ROLE = 'role'
ROLE_MASTER = 'master'
ROLE_SLAVE = 'slave'
ROLE_DEFAULT = ROLE_SLAVE
ROLE_VALUES = [ROLE_MASTER, ROLE_SLAVE]
AIR_COMPRESSION_FACTOR = 'air_compression_factor'
AIR_BUFFER_SIZE = 'air_buffer_size'
AIR_BUFFER_SIZE_DEFAULT = 512
AIR_PACKET_LENGTH = 'air_packet_length'
TESCO = 'tesco'
WESCO = 'wesco'
SLOT_OCCUPANCY = 'slot_occupancy'
SLOT_OCCUPANCY_DEFAULT = 1

PARAM_SCHEMA = {
    'type': 'object',
    'required': [SERVICE_TAG, AIR_COMPRESSION_FACTOR, AIR_PACKET_LENGTH, TESCO, WESCO],
    'properties': {
        SERVICE_TAG: {'type': 'integer', 'minimum': 1},
        HCI_HANDLE: {'type': 'integer', 'minimum': 0},
        WALLCLOCK_ACCURACY: {'type': 'number', 'default': WALLCLOCK_ACCURACY_DEFAULT},
        WALLCLOCK_HANDLE: {'type': 'integer', 'minimum': 0, 'default': WALLCLOCK_HANDLE_DEFAULT},
        WALLCLOCK_OFFSET: {'type': 'integer', 'minimum': 0, 'default': WALLCLOCK_OFFSET_DEFAULT},
        NEXT_SLOT_TIME: {'type': 'integer', 'minimum': 0, 'default': NEXT_SLOT_TIME_DEFAULT},

        ROLE: {'type': 'string', 'enum': ROLE_VALUES, 'default': ROLE_DEFAULT},
        AIR_COMPRESSION_FACTOR: {'type': 'integer'},
        AIR_BUFFER_SIZE: {'type': 'integer', 'minimum': 1, 'default': AIR_BUFFER_SIZE_DEFAULT},
        AIR_PACKET_LENGTH: {'type': 'integer', 'minimum': 1},
        TESCO: {'type': 'integer', 'minimum': 2},
        WESCO: {'type': 'integer', 'minimum': 0},
        SLOT_OCCUPANCY: {'type': 'integer', 'minimum': 1, 'default': SLOT_OCCUPANCY_DEFAULT},
    }
}


class HydraScoProcessingService(object):
    '''
    Hydra SCO Processing Service

    This is a hydra service
    It supports one bidirectional connection, with from air and to air streams. Both streams
    are symmetrical, same tesco, slots and air_packet_length

    Args:
        service_tag (int): Hydra service tag. Every service including non sco processing services
            should have a unique tag
        hci_handle (int): Host Controller Interface handle. This should be a unique number among all
            sco processing services. This number is used to create sco endpoints
        wallclock_accuracy (float): Wallclock simulation accuracy in part per million.
        wallclock_handle (int): TBD FIXME
        wallclock_offset (int): TBD FIXME
        next_slot_time (int): Next reserved slot time in bt ticks (2*slits)
        role (str): Bluetooth link role, master or slave
        air_compression_factor (int): SPS to/from-air compression factor
                     VALID_COMPRESSION_FACTORS = (1, 2)
        air_buffer_size (int): SPS to/from-air buffer size in octets
        air_packet_length (int): SPS to/from-air packet length in octets
        tesco (int): SPS BT link TeSCO (interval between transmissions)
                     VALID_TESCOS = (2, 4, 6, 8, 10, 12, 14, 16, 18)
        wesco (int): SPS BT link WeSCO (retransmission window)
        slot_occupancy (int): Number of slots on the BT physical channel taken by one packet
                     VALID_SLOT_OCCUPANCIES = (1, 3)
    '''

    def __init__(self, **kwargs):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        inherit_docstring(self)

        DefaultValidatingDraft4Validator(PARAM_SCHEMA).validate(kwargs)

        self.__helper = argparse.Namespace()  # helper modules
        self.__helper.uut = get_instance('uut')
        self.__helper.hydra_prot = get_instance('hydra_protocol')
        self.__helper.kalcmd = get_instance('kalcmd')

        self.__config = argparse.Namespace()  # configuration values
        self.__config.service_tag = kwargs.get(SERVICE_TAG)
        self.__config.hci_handle = kwargs.get(HCI_HANDLE, self.__config.service_tag)
        self.__config.wallclock_accuracy = kwargs.get(WALLCLOCK_ACCURACY)
        self.__config.wallclock_handle = kwargs.get(WALLCLOCK_HANDLE)
        self.__config.wallclock_offset = kwargs.get(WALLCLOCK_OFFSET)
        self.__config.next_slot_time = kwargs.get(NEXT_SLOT_TIME)
        self.__config.role = kwargs.get(ROLE)
        self.__config.air_compression_factor = kwargs.get(AIR_COMPRESSION_FACTOR)
        self.__config.air_buffer_size = kwargs.get(AIR_BUFFER_SIZE)
        self.__config.air_packet_length = kwargs.get(AIR_PACKET_LENGTH)
        self.__config.tesco = kwargs.get(TESCO)
        self.__config.wesco = kwargs.get(WESCO)
        self.__config.slot_occupancy = kwargs.get(SLOT_OCCUPANCY)

        # calculate/update other parameters
        self.__config.wallclock_period = \
            WALLCLOCK_UPDATE_PERIOD * (1 + self.__config.wallclock_accuracy / 1e6)

        if self.__config.role == ROLE_SLAVE:
            self.__config.to_air_latency = (self.__config.wesco + 1) * \
                                           TIMESLOT_DURATION_US + TIMESLOT_FRACTION
            self.__config.from_air_latency = -125
        else:
            self.__config.to_air_latency = -125
            self.__config.from_air_latency = (self.__config.wesco + 1) * \
                                             TIMESLOT_DURATION_US + TIMESLOT_FRACTION

        self.__data = argparse.Namespace()  # data values
        self.__data.started = False  # service has not been started
        self.__data.channel_ready = [False, False]  # none of the possible channels are ready yet
        self.__data.bt_clock = 0  # bluetooth wallclock (2*slot)
        self.__data.wallclock_remain = 0  # remainder for bluetooth clock (usec units)
        self.__data.wallclock_timer_id = None
        self.__data.to_air_read_handle = None
        self.__data.to_air_write_handle = None
        self.__data.to_air_aux_handle = None
        self.__data.from_air_read_handle = None
        self.__data.from_air_write_handle = None
        self.__data.from_air_aux_handle = None
        self.__data.msg_handler = self.__helper.hydra_prot.install_message_handler(
            self._message_received)

        if kwargs:
            self._log.warning('unknown kwargs:%s', str(kwargs))

    @log_input(logging.INFO, formatters={'msg': '0x%04x'})
    def _message_received(self, msg):
        '''
        Hydra message sniffer. Waits for SCO_PROC_SERV_RUN_STATE_MSG messages to detect when
        streams are ready

        Args:
            msg (list[int]): Message received
        '''
        if len(msg) == 4 and \
                msg[0] == SCO_PROC_SERV_RUN_STATE_MSG and \
                msg[1] == self.__config.service_tag:

            direction = msg[2]
            if direction == SCO_DIR_TO_AIR:
                self.__data.channel_ready[0] = True
            elif direction == SCO_DIR_FROM_AIR:
                self.__data.channel_ready[1] = True

    def _compute_period(self, period, remainder, resolution=1e-6):
        '''
        Helper function to compute next timer period based of the nominal period, a remainder
        value and the resolution of the timer

        Example:
            Overflow period example::

                period = 0.00000166667
                new_period, remain = self._compute_period(period, 0)
                print(new_period, remain)
                new_period, remain = self._compute_period(period, remain)
                print(new_period, remain)

        Args:
            period (float): Timer nominal period
            remainder (float): Carried remainder from previous timer
            resolution (float): Timer resolution f.i. 0.001 for msecs, 0.000001 for usecs

        Returns:
            tuple:
                float: Timer period
                float: Carried remainder

        '''
        inv_res = 1.0 / resolution
        period_new = int((period + remainder) * inv_res) / inv_res
        remainder_new = (int(int((period + remainder) * 1e9) % int(1e9 / inv_res))) / 1e9
        return period_new, remainder_new

    def _wallclock_update_callback(self, timer_id):
        _ = timer_id
        self.__data.wallclock_timer_id = None

        self.__data.bt_clock += 2 * TIMESLOT_FRACTION
        self._update_wallclock(self.__data.bt_clock)

        period, self.__data.wallclock_remain = self._compute_period(
            self.__config.wallclock_period, self.__data.wallclock_remain)
        self.__data.wallclock_timer_id = self.__helper.uut.timer_add_relative(
            period, self._wallclock_update_callback)

    def _update_wallclock(self, bt_clock):
        '''
        Update wall clock

        Args:
            bt_clock (int): Bluetooth clock value, this clock is half a slot 312.5 usec
        '''
        self._log.info('_update_wallclock bt_clock:%s time:%s',
                       bt_clock, self.__helper.uut.timer_get_time())
        time_stamp = self.__helper.uut.mem_peek(1, TIMER_TIME, 4)
        mem_space = kalmem_spaces.BAC_WINDOW_2
        offset = self.__config.wallclock_offset

        self.__helper.uut.mem_poke(mem_space, offset + 0x0000, 2, DUMMY_ANTI_TEAR_COUNTER)
        self.__helper.uut.mem_poke(mem_space, offset + 0x0002, 2, (bt_clock >> 16) & 0xFFFF)
        self.__helper.uut.mem_poke(mem_space, offset + 0x0004, 2, (bt_clock & 0xFFFF))
        self.__helper.uut.mem_poke(mem_space, offset + 0x0006, 2, (time_stamp >> 16) & 0xFFFF)
        self.__helper.uut.mem_poke(mem_space, offset + 0x0008, 2, time_stamp & 0xFFFF)

    def _start_channel_callback(self, callback, timer_id):
        _ = timer_id
        callback()

    def start(self):
        '''
        Start SCO processing service

        Raises:
            RuntimeError: - If the service has already been started
                          - The response payload length is incorrect
                          - The response service tag doesn't match expected value
        '''
        if self.__data.started:
            raise RuntimeError('service already started')

        msg = [
            self.__config.service_tag,
            self.__config.hci_handle,
            self.__config.air_buffer_size,
            self.__config.air_compression_factor,
            self.__config.wallclock_handle,
            self.__config.wallclock_offset
        ]

        service_tag, payload = self.__helper.hydra_prot.start_service(
            SERVICE_TYPE_SCO_PROCESSING, msg)
        if len(payload) < 6:
            raise RuntimeError('start response length:%s invalid' % (len(payload)))
        if service_tag != self.__config.service_tag:
            raise RuntimeError('start response service tag:%s invalid' % (service_tag))

        self.__data.started = True
        self.__data.to_air_read_handle = payload[0]
        self.__data.to_air_write_handle = payload[1]
        self.__data.to_air_aux_handle = payload[2]
        self.__data.from_air_read_handle = payload[3]
        self.__data.from_air_write_handle = payload[4]
        self.__data.from_air_aux_handle = payload[5]

        # start wall clock simulation
        self.__data.bt_clock = 0
        self._update_wallclock(self.__data.bt_clock)

        self.__data.wallclock_remain = 0
        period, self.__data.wallclock_remain = self._compute_period(
            self.__config.wallclock_period, self.__data.wallclock_remain)
        self.__data.wallclock_timer_id = self.__helper.uut.timer_add_relative(
            period, self._wallclock_update_callback)

    @log_input(logging.INFO)
    def config(self):
        if not self.__data.started:
            raise RuntimeError('set_sco_parameters sco service not started')

        kwargs = {
            'tesco': self.__config.tesco,
            'wesco': self.__config.wesco,
            'to_air_length': self.__config.air_packet_length,
            'from_air_length': self.__config.air_packet_length,
            'next_slot_time': self.__config.next_slot_time,
            'to_air_latency': self.__config.to_air_latency,
            'from_air_latency': self.__config.from_air_latency,
        }
        self.__helper.hydra_prot.set_sco_params(self.__config.service_tag, **kwargs)

    def stop(self):
        '''
        Stop SCO processing service

        Raises:
            RuntimeError: If the service as not been previously started
        '''
        if not self.__data.started:
            raise RuntimeError('stop sco service not started')

        if self.__data.wallclock_timer_id:
            self.__helper.uut.timer_cancel(self.__data.wallclock_timer_id)
            self.__data.wallclock_timer_id = None

        self.__helper.hydra_prot.stop_service(SERVICE_TYPE_SCO_PROCESSING,
                                              self.__config.service_tag)
        self.__data.started = False

    def check_started(self):
        '''
        Check if the SCO processing service is started

        Returns:
            bool: Service already started
        '''
        return self.__data.started

    def get_hci_handle(self):
        '''
        Get SCO processing service hci handle

        Returns:
            (int): hci_handle
        '''
        return self.__config.hci_handle

    def get_tesco(self):
        '''
        Get SCO processing service tesco value

        Returns:
            (int): tesco
        '''
        return self.__config.tesco

    def get_from_air_handle(self):
        '''
        Get SCO processing service from air bac handle

        Returns:
            (int): bac handle
        '''
        return self.__data.from_air_aux_handle

    def get_to_air_handle(self):
        '''
        Get SCO processing service to air bac handle

        Returns:
            (int): bac handle
        '''
        return self.__data.to_air_aux_handle

    @log_input(logging.INFO)
    def start_channel(self, channel, callback):
        '''
        Request callback to be invoked when a given channel is ready to start streaming

        Args:
            channel (int): 0 for to air, 1 for from air
            callback (func): Callback to be called when the service is ready to start streaming
        '''
        # verify channel is ready to start
        if not self.__data.channel_ready[channel]:
            raise RuntimeError('start channel:%s channel not ready' % (channel))

        # FIXME reception at the end of wesco if available

        if self.__config.role == ROLE_MASTER:
            if channel == 0:  # to air
                # master transmits first. data ready at the end of slot_occupancy
                period = self.__config.slot_occupancy * TIMESLOT_DURATION
            else:
                # master receives second. data ready at the end of 2 slot_occupancy
                period = 2 * self.__config.slot_occupancy * TIMESLOT_DURATION

        else:
            if channel == 0:  # to air
                # slave transmits second. data ready at the end of 2 slot_occupancy
                period = 2 * self.__config.slot_occupancy * TIMESLOT_DURATION
            else:
                # slave receives first. data ready at the end of slot_occupancy
                period = self.__config.slot_occupancy * TIMESLOT_DURATION

        self._timer_id = self.__helper.uut.timer_add_relative(
            period, partial(self._start_channel_callback, callback))
