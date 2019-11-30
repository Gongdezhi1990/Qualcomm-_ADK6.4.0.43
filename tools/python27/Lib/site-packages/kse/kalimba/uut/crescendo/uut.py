'''
Kalsim UUT abstraction for Crescendo kalsim
'''

import logging

from kats.framework.library.docstring import inherit_docstring
from kats.framework.library.log import log_input, log_output, log_input_output
from ..kalsim.io import IO
from ..kalsim.message import Message
from ..kalsim.timer import Timer
from ..uut_base import UutBase


class CrescendoUut(UutBase):
    '''
    Unit Under Test abstraction for crescendo running in kalsim.

    It requires a kalcmd instrument instance to operate.

    Args:
        kalcmd (kats.instrument.kalcmd.kalcmd.Kalcmd): Kalcmd instrument
    '''

    platform = 'crescendo'
    interface = 'crescendo'

    def __init__(self, kalcmd):
        inherit_docstring(self)
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init kalcmd:%s', kalcmd)

        self._timer = Timer(kalcmd)
        self._message = Message(kalcmd)
        self._io = IO(kalcmd)
        self._kalcmd = kalcmd

        super(CrescendoUut, self).__init__(kalcmd)

    @log_input(logging.INFO)
    def timer_get_time(self):
        return self._timer.get_time()

    @log_input(logging.INFO)
    def timer_add_relative(self, period, callback=None):
        return self._timer.add_relative_timer(period, callback)

    @log_input(logging.INFO)
    def timer_add_absolute(self, period, callback=None):
        return self._timer.add_absolute_timer(period, callback)

    @log_input(logging.INFO)
    def timer_cancel(self, timer_id):
        self._timer.cancel_timer(timer_id)

    def timer_check_expired(self, timer_id):
        return self._timer.check_timer_expired(timer_id)

    @log_input(logging.INFO)
    def timer_wait_relative(self, period):
        return self._timer.wait_relative_timer(period)

    @log_input(logging.INFO)
    def message_register_handler(self, header, callback=None):
        return self._message.register_message_handler(header, callback)

    @log_input(logging.INFO)
    def message_unregister_handler(self, handler):
        return self._message.unregister_message_handler(handler)

    @log_input(logging.INFO, formatters={'msg': '0x%04x'})
    def message_send(self, handler, msg):
        self._message.send_message(handler, msg)

    @log_input_output(logging.INFO, logging.INFO, formatters={'msg': '0x%04x', 'return': '0x%04x'})
    def message_send_recv(self, handler, msg, timeout=None, flt=None):
        return self._message.send_recv_message(handler, msg, timeout=timeout, flt=flt)

    @log_output(logging.INFO, formatters={'return': '0x%04x'})
    def message_recv(self, handler, timeout=None, flt=None):
        return self._message.recv_message(handler, timeout=timeout, flt=flt)

    @log_output(logging.INFO, formatters={'address': '0x%08x', 'return': '0x%08x'})
    def mem_peek(self, memory_space, address, size):
        return self._io.mem_peek(memory_space, address, size)

    @log_input(logging.INFO, formatters={'address': '0x%08x', 'value': '0x%08x'})
    def mem_poke(self, memory_space, address, size, value):
        self._io.mem_poke(memory_space, address, size, value)

    @log_output(logging.INFO, formatters={'address': '0x%08x', 'return': '0x%08x'})
    def mem_block_read(self, memory_space, address, num_dwords):
        return self._io.mem_block_read(memory_space, address, num_dwords)

    @log_input(logging.INFO, formatters={'address': '0x%08x', 'dwords': '0x%08x'})
    def mem_block_write(self, memory_space, address, dwords):
        self._io.mem_block_write(memory_space, address, dwords)

    @log_input(logging.INFO)
    def interrupt(self, interrupt):
        self._io.interrupt(interrupt)
