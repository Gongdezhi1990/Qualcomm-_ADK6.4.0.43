'''
Kalsim io abstraction layer
'''

import logging

from kats.framework.library.log import log_input, log_output


class IO(object):
    '''
    Module to handle kalcmd/kalsim i/o, memory access and interrupts

    It requires a kalcmd instance to operate.

    Args:
        kalcmd (kats.instrument.Kalcmd.Kalcmd): Kalcmd instrument
    '''

    def __init__(self, kalcmd):
        self._log = logging.getLogger(__name__) if not hasattr(self, '_log') else self._log
        self._log.info('init kalcmd:%s', kalcmd)

        self._kalcmd = kalcmd

    @log_output(logging.INFO, formatters={'address': '0x%08x', 'return': '0x%08x'})
    def mem_peek(self, memory_space, address, size):
        '''
        Read memory address

        Args:
            memory_space (int): Message type to access (PM, DM)
            address (int): Memory address
            size (int): Read size in bytes

        Returns:
            int: Memory data value
        '''
        return self._kalcmd.peek(memory_space, address, size - 1)

    @log_input(logging.INFO, formatters={'address': '0x%08x', 'value': '0x%08x'})
    def mem_poke(self, memory_space, address, size, value):
        '''
        Write memory address

        Args:
            memory_space (int): Message type to access (PM, DM)
            address (int): Memory address
            size (int): Write size in bytes
            value (int): Memory data value
        '''
        self._kalcmd.poke(memory_space, address, size - 1, value)

    @log_output(logging.INFO, formatters={'address': '0x%08x', 'return': '0x%08x'})
    def mem_block_read(self, memory_space, address, num_dwords):
        '''
        Read block memory

        Args:
            memory_space (int): Message type to access (PM, DM)
            address (int): Memory address
            num_dwords (int): Number of 32 bits blocks

        Returns:
            list[int]: Memory data dword values
        '''
        return self._kalcmd.block_mem_read(memory_space, address, num_dwords)

    @log_input(logging.INFO, formatters={'address': '0x%08x', 'dwords': '0x%08x'})
    def mem_block_write(self, memory_space, address, dwords):
        '''
        Write block memory

        Args:
            memory_space (int): Message type to access (PM, DM)
            address (int): Memory address
            dwords (list[int]): Memory data dword values
        '''
        self._kalcmd.block_mem_write(memory_space, address, dwords)

    @log_input(logging.INFO)
    def interrupt(self, interrupt):
        '''
        Assert an interrupt in kalimba

        Args:
            interrupt (int): Interrupt number to assert, starting with 0
        '''
        self._kalcmd.interrupt(interrupt)
