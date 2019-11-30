# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd
from __future__ import print_function


class SliceError(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return self.msg


class MemoryBlock(list):
    """This class provides read-only access to a snapshot of a contiguous block of Kalimba memory.
    It provides the same operations as the built-in list, and can be diffed with another memory block.
    """
    def __init__(self, address, address_width, contents, data_width, addresses_per_word, mem_type):
        list.__init__(self, contents)
        self.address = address
        # widths are in bits; we're displaying as hex digits.
        # '*' width-specifier for print specifically requires an int.
        self.address_width_print_specifier = int(2 * address_width / 8)
        self.data_width_print_specifier    = int(2 * data_width    / 8)
        self.contents = contents
        self.addresses_per_word = addresses_per_word
        self.mem_type = mem_type
        
    def _get_repr(self, limit):
        length_words = len(self.contents)
        limit = limit or length_words
        result = [
            "%s Memory block at 0x%0*x, %d words." %
            (self.mem_type, self.address_width_print_specifier, self.address, length_words)
        ]
        words_to_display = min(length_words, limit)
        if length_words > limit:
            result.append(" First %d word(s) follow..." % limit)
        for i in range(words_to_display):
            current_address = self.address + i * self.addresses_per_word
            result.append(
                "\n0x%0*x:\t0x%0*x" %
                (self.address_width_print_specifier, current_address, self.data_width_print_specifier, self.contents[i])
            )
        return "".join(result)
    
    def __repr__(self):
        return self._get_repr(None)
        
    def display(self, limit = None):
        """Print the memory block's contents, up to the specified number of words."""
        print(self._get_repr(limit))

    def diff(self, other):
        """Perform a diff of this memory block with another, printing the results."""
        if self.mem_type != other.mem_type:
            print("Memory types differ: (%s, %s)" % (self.mem_type, other.mem_type))
        if self.address != other.address:
            print(
                "Start addresses differ: (0x%0*x, 0x%0*x)" %
                (self.address_width_print_specifier, self.address, other.address_width_print_specifier, other.address)
            )
        other_len = len(other.contents)
        my_len    = len(self.contents)
        if my_len != other_len:
            print("Lengths differ (%d, %d)" % (my_len, other_len))

        for i in range(min(my_len, other_len)):
            if self.contents[i] != other.contents[i]:
                print(
                    "Mismatch at offset %d: (0x%0*x, 0x%0*x)" %
                    (
                        i,
                        self.data_width_print_specifier,
                        self.contents[i],
                        other.data_width_print_specifier,
                        other.contents[i]
                    )
                )


class DmMemoryBlock(MemoryBlock):
    def __init__(self, address, address_width, contents, data_width, addresses_per_word):
        MemoryBlock.__init__(self, address, address_width, contents, data_width, addresses_per_word, "DM")


class PmMemoryBlock(MemoryBlock):
    def __init__(self, address, address_width, contents, data_width, addresses_per_word):
        MemoryBlock.__init__(self, address, address_width, contents, data_width, addresses_per_word, "PM")


class MemoryAccessor(object):
    def __init__(self, reader, writer, block_class):
        self.__dict__['_reader'] = reader
        self.__dict__['_writer'] = writer
        self.__dict__['_block_class'] = block_class

    @staticmethod
    def _check_step(index):
        if not (index.step is None or index.step == 1):
            raise SliceError("Step size must be one for slices used by this accessor")

    def addresses_per_word(self):
        raise NotImplementedError()

    def get_address_width(self):
        raise NotImplementedError()

    def get_data_width(self):
        raise NotImplementedError()

    def _check_endpoint(self, index):
        remainder      = (index.stop - index.start) % self.addresses_per_word()
        if remainder != 0:
            raise SliceError("Slice size must be divisible by word size on byte-addressable architectures")
                
    def __getitem__(self, index):
        if isinstance(index, slice):
            self._check_step(index)
            self._check_endpoint(index)
                
            words_to_fetch = (index.stop - index.start) // self.addresses_per_word()
            contents = self.__dict__['_reader'](index.start, words_to_fetch)
            # Construct a memory block, of class self._block_class.
            return self.__dict__['_block_class'](
                index.start,
                self.get_address_width(),
                contents,
                self.get_data_width(),
                self.addresses_per_word()
            )
        else:
            return self.__dict__['_reader'](index, 1)[0]

    def __setitem__(self, index, val):
        if isinstance(val, list) or isinstance(val, tuple):
            # Sanity check user input
            if isinstance(index, slice):
                self._check_step(index)
                self._check_endpoint(index)
            
                words_to_write = (index.stop - index.start) // self.addresses_per_word()
                if words_to_write != len(val):
                    raise SliceError("Slice and input data length mismatch")
                self.__dict__['_writer'](index.start, val)
            else:
                self.__dict__['_writer'](index, val)
        else:
            # Sanity check user input
            if isinstance(index, slice):
                raise SliceError("Data to write must be a list or tuple when destination is a slice")
            self.__dict__['_writer'](index, [val])


class DmAccessor(MemoryAccessor):
    """This class gives read and write access to Kalimba Data Memory.
       Typically one instance of this class is available and is called "dm".

       Example read usage:
         dm[0xabc]         - Reads contents of DM[0x123] returns the value as a Python integer
         dm[123:133]       - Reads contents of DM[123] through to DM[132] inclusive, in units of words. 
                             Returns a DmMemoryBlock containing the results (ten words in this case).

       Example write usage:
         dm[123] = 456     - Writes 456 into DM[123]
         dm[123] = [4,5,6] - Equivalent to dm[123] = 4, dm[124] = 5, dm[126] = 6

    """
    def __init__(self, kal):
        MemoryAccessor.__init__(self, kal._read_dm_block, kal._write_dm_block, DmMemoryBlock)
        self.__dict__['_kal'] = kal
        
    def __getitem__(self, index):
        if isinstance(index, str):
            # Some sort of symbol.
            results = self.__dict__['_kal'].sym.varfind(index)
            if len(results) == 0:
                raise KeyError("No variable matches for symbol '%s'" % index)
            if len(results) > 1:
                raise KeyError("More than one variable match for symbol '%s'" % index)
                
            address = results[0][2]
            size_in_addressable_units = results[0][1]
            size_in_words = size_in_addressable_units // self.addresses_per_word()
            # If the variable is not an integral number of words in size, round up to the next word
            if size_in_addressable_units % self.addresses_per_word() != 0:
                size_in_words += 1
            data = self.__dict__['_reader'](address, size_in_words)
            return data[0] if len(data) == 1 else DmMemoryBlock(
                address,
                self.get_address_width(),
                data,
                self.get_data_width(),
                self.addresses_per_word()
            )
        else:
            return MemoryAccessor.__getitem__(self, index)
    
    # Utility functions called by the base class to allow PM/DM variations
    def addresses_per_word(self):
        return self.__dict__['_kal'].arch.dm_address_inc_per_word()
    
    def get_address_width(self):
        return self.__dict__['_kal'].arch.get_address_width()
    
    def get_data_width(self):
        return self.__dict__['_kal'].arch.get_data_width()


class PmAccessor(MemoryAccessor):
    """This class gives read and write access to Kalimba Program Memory.
       Typically one instance of this class is available and is called "pm".

       Example read usage:
         pm[0xabc]         - Reads contents of PM[0x123] returns the value as a Python integer
         pm[123:133]       - Reads contents of PM[123] through to PM[132] inclusive, in units of words.  
                             Returns a PmMemoryBlock containing the results (ten words in this case).

       Example write usage:
         pm[123] = 456     - Writes 456 into PM[123]
         pm[123] = [4,5,6] - Equivalent to pm[123] = 4, pm[124] = 5, pm[126] = 6

    """
    def __init__(self, kal):
        MemoryAccessor.__init__(self, kal._read_pm_block, kal._write_pm_block, PmMemoryBlock)
        self.__dict__['_kal'] = kal
        
    # Utility functions called by the base class to allow PM/DM variations
    def addresses_per_word(self):
        return self.__dict__['_kal'].arch.pm_address_inc_per_word()
        
    def get_address_width(self):
        return self.__dict__['_kal'].arch.get_address_width()
        
    def get_data_width(self):
        # kalaccess does not expose "pm data width" from dspinfo, 
        # but it's the same for all current architectures.
        return 32
