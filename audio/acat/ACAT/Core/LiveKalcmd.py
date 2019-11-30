############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module used to connect to a Kalcmd simulation.
"""
import logging

from . import Arch
from . import CoreUtils as cu
from . import LiveChip
from ACAT.Core.exceptions import ChipdataError

logger = logging.getLogger(__name__)


class DmAccessorKalcmd(cu.global_options.kmem.DmAccessor):
    """Modified DM memory accessor for kalcmd.

    Args:
        words_to_fetch
    """

    def __init__(self, kalcmd_object):
        def read_dm_block(address, words_to_fetch):
            maximum_read = 32

            # When the number of words to fetch is large, we have to split
            # it into chunks of maximum_read sizes.
            values = []
            while words_to_fetch:
                if words_to_fetch < maximum_read:
                    fetch_words = words_to_fetch
                else:
                    fetch_words = maximum_read

                values.extend(
                    kalcmd_object.block_mem_read(
                        kalcmd_object.kalcmd2.kalmem_spaces.DM,
                        address,
                        fetch_words
                    )
                )

                # Re calculate the address and outstanding words to fetch
                address += fetch_words * Arch.addr_per_word
                words_to_fetch -= fetch_words

            return values

        def write_dm_block(address, values):
            """Write words to kalsim.

            Args:
                address
                values
            """
            maximum_write = 32
            words_to_write = values[:]

            # When the number of words to write is large, we have to split
            # it into chunks of maximum_write sizes.
            while words_to_write:
                if len(words_to_write) < maximum_write:
                    write_words = words_to_write[:]
                else:
                    write_words = words_to_write[:maximum_write]

                kalcmd_object.block_mem_write(
                    kalcmd_object.kalcmd2.kalmem_spaces.DM,
                    address,
                    write_words
                )

                # Re calculate the address and outstanding words to write
                address += len(write_words) * Arch.addr_per_word
                words_to_write = words_to_write[maximum_write:]

        cu.global_options.kmem.MemoryAccessor.__init__(
            self,
            read_dm_block,
            write_dm_block,
            cu.global_options.kmem.DmMemoryBlock
        )
        self.__dict__['_kalcmd_object'] = kalcmd_object

    def addresses_per_word(self):
        """Returns address per word."""
        return self.addr_per_word

    def get_address_width(self):
        """Returns Address Width."""
        return self.address_width

    def get_data_width(self):
        """Returns Data Width."""
        return self.data_width


class PmAccessorKalcmd(cu.global_options.kmem.PmAccessor):
    """PM memory accessor for kalcmd.

    Args:
        kalcmd_object
    """

    def __init__(self, kalcmd_object):
        def read_pm_block(address, words_to_fetch):
            """Read words from kalsim."""
            return kalcmd_object.block_mem_read(
                kalcmd_object.kalcmd2.kalmem_spaces.PM,
                address,
                words_to_fetch
            )

        def write_pm_block(address, values):
            """Write words to kalsim."""
            kalcmd_object.block_mem_write(
                kalcmd_object.kalcmd2.kalmem_spaces.PM,
                address,
                values
            )

        cu.global_options.kmem.MemoryAccessor.__init__(
            self,
            read_pm_block,
            write_pm_block,
            cu.global_options.kmem.PmMemoryBlock
        )
        self.__dict__['_kalcmd_object'] = kalcmd_object
        # save the data width so we can change it dynamically.
        self._data_width = 32

    def addresses_per_word(self):
        """Returns Address Per Word."""
        return self.addr_per_word

    def get_address_width(self):
        """Returns Address Width."""
        return self.address_width

    def get_data_width(self):
        """Returns Data Width.

        kalaccess does not expose "pm data width" from dspinfo, but it's
        the same for all current architectures.
        """
        return self._data_width


##################################################
class LiveKalcmd(LiveChip.LiveChip):
    """Provides access to chip data on a live chip over SPI.

    Args:
        kalcmd_object
        processor
    """

    def __init__(self, kalcmd_object, processor):
        LiveChip.LiveChip.__init__(self)

        self._kalcmd_object = kalcmd_object
        self.processor = processor

        self.dm = DmAccessorKalcmd(kalcmd_object)
        self.pm = PmAccessorKalcmd(kalcmd_object)

        logger.info("Initialising connection with Kalcmd")

        kal_arch = self._kalcmd_object.getKalArch()

        kalimba_name = self._kalcmd_object.getKalimbaName()

        self.dm.addr_per_word = 4
        self.dm.address_width = 32
        self.dm.data_width = 32

        self.pm.addr_per_word = 4
        self.pm.address_width = 32

        # kalarch
        # print self._kalcmd_object.get_version()
        # kal_version = self._kalcmd_object.get_version()
        chip_id = None
        if kalimba_name == "csra68100_audio":
            # Crescendo
            chip_arch = "Hydra"
            chip_revision = 0
            chip_id = 0x46
        elif (kalimba_name == "QCC3400_audio" or
                kalimba_name == "QCC302x_audio" or
                kalimba_name == "QCC512x_audio"):
            # Aura
            chip_arch = "Hydra"
            chip_revision = 0
            chip_id = 0x49
        elif kalimba_name == "QCC514x_audio":
            # AuraPlus
            chip_arch = "Hydra"
            chip_revision = 0
            chip_id = 0x4b
        elif kalimba_name == "napier_audio":
            chip_arch = "Napier"
            chip_revision = 0
            chip_id = 0
        else:
            chip_arch = "Bluecore"
            raise Exception("Chip not supported! %s" % kalimba_name)

        if chip_id is not None:
            Arch.chip_select(kal_arch, chip_arch, chip_id, chip_revision)
        else:
            Arch.chip_select(kal_arch, chip_arch, chip_id)

    @LiveChip.re_entrant_lock_decorator
    def get_proc_reg(self, name):
        """Returns the value of a Processor Register.

        Return the value of the processor register specified in 'name'.
        name is a string containing the name of the register in upper or
        lower case, with or without the prefix 'REGFILE_':

            e.g. "REGFILE_PC", "rMAC", "R10".

        Args:
            name

        Raises:
            KeyError
            AttributeError
        """
        name = name.upper()
        if "REGFILE_" in name:
            # replace REGFILE with REG which is recognised by kalcmd
            name = name.replace("REGFILE", "REG")
        elif "REG_" in name:
            # this is the good format for the kalcmd
            pass
        else:
            # kalcmd needs "REG_" prefix
            name = "REG_" + name

        # Get the kalcmd internal reg number
        reg_number = self._kalcmd_object.kalcmd2.kal_regs.__dict__[name]
        return self._kalcmd_object.querryDspReg(reg_number)

    @LiveChip.re_entrant_lock_decorator
    def get_all_proc_regs(self):
        """Returns a dictionary containing all processor registers.

        Dictionary maps name to value. NB on a running chip there is the
        obvious risk of tearing.  Consider stopping the chip if this is an
        issue.
        """
        regs = {}
        for reg_name in self._kalcmd_object.kalcmd2.kal_regs.__dict__.keys():
            if reg_name[0] != '_':
                regs[
                    reg_name.replace("REG_", "REGFILE_")
                ] = self.get_proc_reg(reg_name)

        return regs

    def get_banked_reg(self, address, bank=None):
        """Return the value of a banked register in a specific bank.

        Note:
            Default is the current value in the register.

        Args:
            address: Address of the banked register.
            bank
        """
        raise ChipdataError("get_banked_reg not supported!")

    def get_all_banked_regs(self, addr_bank_sel_reg):
        """Gets all banked registers.

        Returns a string containing all banks in a banked register given
        the address of the register used to select the bank.

        Args:
            addr_bank_sel_reg: The address of the bank select register
                that controls the bank.
        """
        raise ChipdataError("get_all_banked_regs not supported!")
