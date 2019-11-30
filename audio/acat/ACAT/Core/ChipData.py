############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
This module defines an interface for extracting information from a chip.
"""
import logging
import numbers
import re
import threading
try:
    # Python 3
    from queue import Queue
except ImportError:
    # Python 2
    from Queue import Queue
    from future_builtins import hex

import time as import_time

from . import Arch
from ACAT.Core import CoreTypes as ct
from ACAT.Core.exceptions import (
    DebugInfoNoVariableError, InvalidDmAddressError
)
from ACAT.Core.logger import method_logger

# This is an abstract base class, but am avoiding use of Python's ABC syntax.

logger = logging.getLogger(__name__)


class ChipData(object):
    """Defines an interface to extract information from a chip.

    Whether it is connected 'live' or its state has been extracted into a
    coredump.
    """

    @method_logger(logger)
    def __init__(self):
        self.debuginfo = None

    @method_logger(logger)
    def get_patch_id(self):
        """Reads the relevant variable and return the patch ID.

        Returns:
            int: zero if there is no patch detected.
        """
        for variable_name in ('$_patched_fw_version', '$_patch_level'):
            try:
                return self.get_var_strict(variable_name).value

            except DebugInfoNoVariableError:
                logger.debug("Variable not found: %s", variable_name)

        logger.warning("Patch ID variables are not found")
        return 0

    def set_debuginfo(self, debuginfo):
        """Sets the debuginfo object.

        Chipdata uses the debug information for getting type information
        and constant data.

        Args:
            debuginfo
        """
        self.debuginfo = debuginfo

    def is_volatile(self):
        """Checks whether the data is volatile.

        Allows the user to query whether the data is volatile (e.g. from a
        live chip) or fixed (e.g. from a coredump).

        Returns:
            bool: True if the data is volatile, False othewise.
        """
        raise NotImplementedError()

    def get_data(self, address, length=0, ignore_overflow=False):
        """Returns the contents of one or more addresses in DM.

        This allows you to grab a chunk of memory, e.g. get_data(0x600, 50).
        Addresses can be from any valid DM region (DM1, PM RAM, mapped NVMEM,
        memory-mapped registers, etc.).

        Note:
            The length supplied is measured in addressable units.

            get_data(addr) will return a single value;
            get_data(addr, 1) will return a list with a single member.
            get_data(addr, 10) will return a list with ten members or a
                list with three members (when the memory is octet
                addressed, 32bit words).

        If the address is out of range, a KeyError exception will be raised.
        If the address is valid but the length is not (i.e. address+length
        is not a valid address) an OutOfRangeError exception will be raised.

        Note that reading PM RAM via DM is not supported (since not all chips
        map PM into DM). Use get_data_pm() instead.

        Args:
            address
            length
            ignore_overflow (bool, optional): Ignore if the read goes out
                from the memory region and append zeros. This is useful if
                an union is at the end of the memory region.
        """
        raise NotImplementedError()

    def get_data_pm(self, address, length=0):
        """Gets data from PM.

        This method works in the exactly the same way as `get_data`
        method, but it returns data from PM region instead of DM.

        Args:
            address
            length (int, optional)
        """
        raise NotImplementedError()

    def get_proc_reg(self, name):
        """Returns the value of the processor register specified in 'name'.

        Name is a string containing the name of the register in upper or
        lower case, with or without the prefix 'REGFILE_' e.g.
        "REGFILE_PC", "rMAC", "R10".

        Args:
            name

        Raises:
            KeyError
            AttributeError
        """
        raise NotImplementedError()

    def get_all_proc_regs(self):
        """Returns all processor registers.

        Returns:
            dict
        """
        raise NotImplementedError()

    def get_banked_reg(self, address, bank=None):
        """Returns the value of a banked register.

        The default is the current value in the register.

        Args:
            address: Address of the banked register.
            bank
        """
        raise NotImplementedError()

    def get_all_banked_regs(self, addr_bank_sel_reg):
        """Returns a string of all banked registers.

        Given the address of the register used to select the bank.

        Args:
            addr_bank_sel_reg: The address of the bank select register
                that controls the bank.
        """
        raise NotImplementedError()

    def get_firmware_id(self):
        """Returns the firmware ID in integer."""
        raise NotImplementedError()

    def get_firmware_id_string(self):
        """Returns the firmware ID in string."""
        raise NotImplementedError()

    def set_data(self, address, value):
        """Sets the contents of one or more addresses.

        This allows you to write a list of values to a chunk of memory.
        Addresses can only be from any valid DM RAM or memory mapped
        register region.

        e.g. set_data(0x3e5d, [1 2 3])

        Note:
            set_data(address, [val]) writes address with a single value.
            set_data(address, [val1 ... valn]) writes the list of values
                to memory starting from address.

        This function should only be implemented for live chips. And
        should not be called if the chip is not live.

        Args:
            address
            value

        Raises:
            KeyError: When the address is out of range.
            OutOfRangeError: When the address in valid but the length is
                not (i.e. address+length is not a valid address).
        """
        raise NotImplementedError()

    ##################################################
    # Private methods
    ##################################################

    # identifier, inspecting_all_vars = False):
    @method_logger(logger)
    def get_var_strict(self, identifier, elf_id=None):
        """Gets the value of a variable.

        Provided for the convenience of all Analysis modules, this method
        looks up the value of a variable.

        'identifier' can be a variable name (which must be EXACT), or
        address.

        Args:
            identifier
            elf_id (int, optional)

        Returns:
            a Variable.
        """
        # First, look up the variable in the debug information.
        # get_var_strict might throw an exception here; make no effort to
        # intercept it because we are being strict!
        var = self.debuginfo.get_var_strict(identifier, elf_id)

        # replace the elf id with the variable elf_id if it has debug info.
        if var.debuginfo:
            elf_id = var.debuginfo.elf_id

        # Now get the data value from chipdata. Look in DM first, only
        # try const if we run out of options.
        try:
            var.value = self.get_data(var.address, var.size)
        except InvalidDmAddressError:
            # The address wasn't valid. Could be that this variable is
            # actually in dm const.
            var.value = self.debuginfo.get_dm_const(
                var.address, var.size, elf_id
            )

        var.members = []
        return self.debuginfo.inspect_var(var, elf_id)

    @method_logger(logger)
    def identifier_exists(self, identifier):
        """Checks if an identifier exists.

        When an identifier is given as a variable name, it should be the exact
        name. This method has no intention to guess names.

        Args:
            identifier: Can be a variable name or an address.

        Returns:
            bool: True when the identifier exists, False otherwise.
        """
        try:
            self.get_var_strict(identifier)
            return True

        except DebugInfoNoVariableError:
            return False

    @method_logger(logger)
    def get_reg_strict(self, identifier):
        """Get the value of a register.

        Provided for the convenience of all Analysis modules, this method
        looks up the value of memory-mapped and processor registers.

        'identifier' can be a register name (which must be EXACT), or
        address.

        In order to make sure they are unique, the names processor registers
        must be preceded by 'REGFILE_', e.g. 'REGFILE_PC'.

        Args:
            identifier

        Returns:
            a DataSym containing the register details plus its value.
        """

        reg = None  # Will be a ConstSym.
        # If the caller supplied an address, and it smells like a register,
        # attempt to look it up.
        if isinstance(identifier, numbers.Integral):
            if Arch.get_dm_region(identifier) == "MMR":
                # Look for constants that have a value of the supplied
                # address.  Inherently risky, since any random constant
                # could have a value that looks like a register address.
                # Since we only do this to set the name, it should be ok
                # even in strict mode.
                possible_regs = [
                    item[0] for item in list(self.debuginfo.constants.items())
                    if item[1] == identifier
                ]
                if possible_regs:
                    reg_name = " or ".join(possible_regs)
                    reg = ct.ConstSym(reg_name, identifier)
        elif re.search('regfile_', identifier.lower()) is not None:
            # Must be a processor register? On Amber these are listed in
            # constants, so we could look up the address in the same way
            # as for memory-mapped registers below. But on other chips
            # (e.g. Gordon) they're not (in fact, they may not even be
            # mapped into DM at all), so we need to get them from chipdata
            # by name.
            val = self.get_proc_reg(identifier)
            return ct.DataSym(identifier, None, val)
        else:
            # Look up the supplied name in our list of constants.
            # If the name is found, reg.value is actually going to be the
            # address of the register.
            # get_constant_strict might throw an exception here; make no
            # effort to intercept it because we are being strict!
            if '?int64_lo' not in identifier and Arch.addr_per_word == 4:
                identifier = identifier + '?int64_lo'
                return self.get_reg_strict(identifier)
            else:
                reg = self.debuginfo.get_constant_strict(identifier)

        if reg is None or Arch.get_dm_region(reg.value) != "MMR":
            # The supplied identifier wasn't a unique name or valid address.
            if isinstance(identifier, int):
                errid = hex(identifier)
            else:
                errid = identifier
            raise ValueError(
                "Identifier " +
                errid +
                " is not valid in strict mode!")

        # Look up register contents.
        # This should be safe, since we know reg.value is valid (right??)
        regcontents = self.get_data(reg.value)
        return ct.DataSym(reg.name, reg.value, regcontents)

    @method_logger(logger)
    def get_banked_reg_strict(self, address, bank=None):
        """Looks up the values of a banked register.

        It takes the address of a banked register and the bank number
        (optional).

        Returns a ct.DataSym containing the bank select register name and
        address plus the values in the bank.

        Args:
            address: Address of the banked register.
            bank
        """
        reg = None
        if isinstance(address, numbers.Integral):
            if Arch.get_dm_region(address) == "MMR":
                val = self.get_banked_reg(address, bank)
                reg = self.get_reg_strict(address)
                return ct.DataSym(reg.name, address, val)
            else:
                raise ValueError("The address {} is not in MMR region.".format(
                    hex(address)
                ))

    @method_logger(logger)
    def cast(self, addr, type_def, deref_type=False, section="DM",
             elf_id=None):
        """Casts an address to a variable.

        This method takes the address of some (malloc'd) memory and
        'casts' it to be a variable of type 'type_def'.

        Type can be either a type_def name, or a type ID. However, if the
        processor is patched and the type ID is given, providing `elf_id`
        is also mandatory.

        Often (especially in automatic analysis), we know the address of
        the memory but the typeId of its _pointer_. If 'type_def' is an ID
        and 'deref_type' is set to True, we will look up the type
        pointed-to, rather than the actual typeId supplied. (This isn't
        necessary if the type_def name is supplied directly; in that case
        we must assume that the address contains something of that
        type_def.).

        Args:
            addr
            type_def
            deref_type (bool, optional)
            section (str, optional)
            elf_id (int, optional)

        Returns:
            a Variable.
        """
        is_patched = len(self.debuginfo.debug_infos[None]) > 2
        if (is_patched and
                isinstance(type_def, numbers.Integral) and
                elf_id is None):
            # Type Definition ID may refer to a different structure when a
            # processor is patched.
            logger.error(
                "When `type_def` is given as an integer and there is a "
                "patch, `elf_id` is mandatory. Please provide one."
            )
            return

        # Look up type info, even if type is already a typeId (since it
        # could be a pointer, or a typedef, or 'volatile', etc.)
        (
            varname,
            typeid,
            ptr_typeid,
            _,
            _,
            varsize,
            type_elf_id,
            has_union
        ) = self.debuginfo.get_type_info(type_def, elf_id)

        if typeid is None:
            raise ValueError("Type " + str(type_def) + " not found!")

        if deref_type:
            # All the stuff we just got is valid for the pointer, not the
            # pointed-to type. So dereference it.
            if ptr_typeid:
                (
                    varname,
                    typeid,
                    ptr_typeid,
                    _,
                    _,
                    varsize,
                    type_elf_id,
                    has_union
                ) = self.debuginfo.get_type_info(ptr_typeid, elf_id)
            else:
                raise ValueError("Type pointed to by " +
                                 str(type_def) + " not found!")

        # First check if flag 'PM' is present. This means special case that
        # casting is required for PM region
        if section == 'PM':
            pm_region = Arch.get_pm_region(addr)
            if pm_region != 'PMRAM':
                raise ValueError(
                    "Address " +
                    hex(addr) +
                    " does not point to the PMRAM region - can't cast it!")
            var = ct.Variable(
                name=varname,
                address=addr,
                size=varsize,
                value=None,
                var_type=typeid
            )  # Our juicy payload
            var.value = self.get_data_pm(addr, varsize)

        # If PM flag is not given check that the memory is in DM RAM. If
        # it points into const (or the debug region) then there should
        # already be a variable at that address which can tell us
        # everything we need to know.
        # Do this now so that we can sanity-check the size of the variable
        # with the size we extracted from typeinfo.
        else:
            dm_region = Arch.get_dm_region(addr)
            if 'NVMEM' in dm_region or dm_region == 'DEBUG':
                constvar = self.get_var_strict(addr)
                if constvar.size != varsize:
                    raise ValueError(
                        "Address " + hex(addr) +
                        " points to NVMEM/DEBUG region - can't cast it!"
                    )
                return constvar
            elif dm_region == 'BAC':
                raise ValueError(
                    "Address " + hex(addr) +
                    " points to the BAC region - can't cast it!"
                )
            elif (dm_region not in ('DM1RAM', 'DM2RAM', 'DMSRAM') and
                    Arch.get_pm_region(addr) == 'PMROM'):
                raise ValueError(
                    "Address " + hex(addr) +
                    " is a function pointer - can't cast it!"
                )

            var = ct.Variable(
                varname, addr, varsize, None, typeid
            )
            # Ignore memory overflow if the structure has a unin.
            # Some parts of the union might fit in the memory.
            var.value = self.get_data(addr, varsize, ignore_overflow=has_union)

        # Don't add this (presumably malloc'd) variable to our store of
        # variables; it might become invalid if talking to a live chip.
        return self.debuginfo.inspect_var(var, type_elf_id)

    @method_logger(logger)
    def run_poll(self, time, identifier):
        """Measures the value of variable(s) over a given period of time.

        Args:
            time
            identifier: Name/address. Argument can be a single entry or in
                the form of a list e.g. [0x8688, '$_total_pool_size'].
        """
        queue = Queue()

        _stop_event = threading.Event()
        # Detects if identifier is a list or a single entry so it can be
        # formatted correctly.  If there are multiple entries there will
        # be fewer measurements taken for each variable.
        if isinstance(identifier, list):
            reader = VariablePoller(_stop_event, identifier, self, queue)
        else:
            reader = VariablePoller(_stop_event, [identifier], self, queue)
        reader.start()

        import_time.sleep(time)
        _stop_event.set()
        poll_data = queue.get()

        # wait for the task to finish
        reader.join()

        return poll_data

    @method_logger(logger)
    def is_booted(self):
        """Checks if the processor is booted.

        The booted state is decided by the Program Counter value.

        Returns:
            bool: True if the processor was booted, False otherwise.
        """
        proc_program_counter = self.get_reg_strict("regfile_pc").value

        return proc_program_counter != 0


class VariablePoller(threading.Thread):
    """Polling a variable/register.

    Reads a defined variable/register for a period of time. If a list of
    variable/register names is passed to the class then each shall be
    measured.

    Args:
        stop_event
        identifiers
        chipdata
        queue
    """

    @method_logger(logger)
    def __init__(self, stop_event, identifiers, chipdata, queue):
        threading.Thread.__init__(self)

        self._stop_event = stop_event
        self.chipdata = chipdata
        self.identifiers = identifiers
        self.queue = queue

    def run(self):
        """Starts the thread.

        Reads the desired variable until the stop event arrives, returns
        list.
        """
        multi_flag = 0
        poll_data = []
        # Puts into list if identifiers is lone address
        if isinstance(self.identifiers, numbers.Integral):
            self.identifiers = [self.identifiers]
        elif len(self.identifiers) > 1:
            multi_flag = 1

        while not self._stop_event.is_set():
            values = []

            for var in self.identifiers:
                try:
                    value = self.chipdata.get_var_strict(var).value
                except DebugInfoNoVariableError:
                    try:
                        value = self.chipdata.get_reg_strict(var).value
                    except KeyError:
                        return None
                if multi_flag:
                    values.append(value)
                # need to add value from identifier list to small list before
                # appending to values
            if multi_flag:
                poll_data.append(values)
            else:
                poll_data.append(value)

        self.queue.put(poll_data)
