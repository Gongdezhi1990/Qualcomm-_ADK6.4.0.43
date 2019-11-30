############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2012 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
This module is used to hold all the clobal types in ACAT.
"""
import numbers

from ACAT.Core.exceptions import (
    InvalidDebuginfoTypeError, InvalidDebuginfoEnumError
)

try:
    from future_builtins import hex
except ImportError:
    pass


#############
# Data types
#############


class ConstSym(object):
    """A symbolic constant.

    e.g. $sbc.mem.ENC_SETTING_BITPOOL_FIELD or $_REGFILE_PC.

    Note:
        That register names are constants, and their values are the
        address of the register. (If the contents of register is also
        known then it should be wrapped as a DataSym instead.)

    Args:
        name
        value
    """

    def __init__(self, name, value):
        self.name = name
        self.value = value

    def __str__(self):
        return (
            'Name: ' + str(self.name) + '\n' + 'Value: ' + hex(self.value) +
            '\n'
        )


class DataSym(object):
    """A data symbol (register, dm_const entry, variable).

    Args:
        name
        address
        value
    """

    def __init__(self, name, address, value=None):
        self.name = name
        self.address = address  # Stored as an integer
        self.value = value  # Integer; won't necessarily know this.

    def __str__(self):
        # Special rule since you can't call hex() on None
        if self.address is not None:
            hex_addr = hex(self.address)
        else:
            hex_addr = 'None'

        return ('Name: ' + str(self.name) + '\n' + 'Address: ' +
                hex_addr + '\n' + 'Value: ' + self._value_to_hex() + '\n')

    def _value_to_hex(self):
        """Converts the value into hex and return it.
        
        The value of the Variable might be in different types. This method
        supports the conversion when the type is in Integer, list or tuple
        of Integers and None.

        When the value is a list of integers, each number will be
        converted into hex. Also, in case the value is None, the `None`
        string will be returned.

        Returns:
            str: A string representation of the Variable's value.
        """
        if isinstance(self.value, numbers.Integral):
            display_value = hex(self.value)

        elif isinstance(self.value, (list, tuple)):
            display_value = str([hex(val) for val in self.value])

        else:
            # Conversion to hex was unsuccessful.
            display_value = str(self.value)

        return display_value


class Variable(DataSym):
    """A variable.

    Variables are more complicated than a standard data symbol, since they
    have a size and can be structures.

    'value' can be a single integer or (more likely) a list/tuple.

    Args:
        name
        address
        size
        value
        var_type
        debuginfo
        members
        parent
    """
    integer_names = [
        "uint32",
        "uint24",
        "uint16",
        "uint8",
        "int",
        "unsigned",
        "unsigned int"
    ]

    def __init__(
            self,
            name,
            address,
            size,
            value=None,
            var_type=None,
            debuginfo=None,
            members=None,
            parent=None
    ):
        DataSym.__init__(self, name, address, value)
        self.size = size
        self.type = var_type  # type ID; meaningless to users

        # list of members (Variable objects).
        # These can be struct/union members, or array elements
        if members is None:
            self.members = []
        else:
            self.members = members
        self.parent = parent  # The Variable which owns this one (if any).
        # if non-zero, variable is an array of this length
        # Note: some types define an array of length 0...
        self.array_len = 0
        # e.g. struct foo, or uint16*
        self.type_name = ''
        # name of this struct/union member, without the parent part.
        self.base_name = ''

        # This will be set if the variable is part of a bitfield.
        self.size_bits = None  # Size in bits (self.size will always be '1').
        # Mask that must be ANDed with self.value to yield the correct bitfield
        self.val_mask = None
        # Right-shift that must be applied to self.value to yield the correct
        # bitfield
        self.val_rshift = None

        self.indent = ""  # Used when printing a variable out

        # Debuginfo is used to cast the structure fileds for better display.
        self.debuginfo = debuginfo

    def __repr__(self, *args, **kwargs):
        """String representation.

        The standard representation will be the same as the standard to
        string to make the interactive interpreter more user-friendly.
        """
        return self.var_to_str()

    def __str__(self):
        """The standard to string function."""
        return self.var_to_str()

    def _get_enum_name(self):
        """Returns the enum name based on the value."""
        # check if the debug info is set to get the enum names from
        # there.
        ret_string = ""
        if self.debuginfo is not None:
            enum_value_name = ""
            error_msg = ""
            try:
                # get the enum type name which is after the "enum " word
                # 1- is used as an indexer to avoid index error which
                # in this case is favourable
                enum_type_name = self.type_name.split("enum ")[-1]
                enum_value_name = self.debuginfo.get_enum(
                    enum_type_name,
                    self.value
                )
            except InvalidDebuginfoEnumError:  # The enum type is missing
                # This can happen if an enum type is typdefed. Something like:
                # typedef enum_type new_enum_type; try to dereference
                try:
                    # get the enum type
                    enum_type_id = self.debuginfo.get_type_info(
                        enum_type_name
                    )[1]
                    # get the referenced type from the enum type.
                    enum_type = self.debuginfo.types[enum_type_id]
                    enum_type = self.debuginfo.types[enum_type.ref_type_id]
                    enum_type_name = enum_type.name
                    # Finally, get the enum value name.
                    enum_value_name = self.debuginfo.get_enum(
                        enum_type.name,
                        self.value
                    )
                except (InvalidDebuginfoTypeError,
                        InvalidDebuginfoEnumError):
                    error_msg += (
                        "(enum type \"" + enum_type_name +
                        "\" not found for \"" + self.base_name + "\" member)"
                    )
                except KeyError:  # the enum is missing the values
                    error_msg += (
                        "(enum \"" + enum_type_name +
                        "\" has no value " + self._value_to_hex() + ")"
                    )
            except KeyError:  # the enum is missing the values
                error_msg += (
                    "(enum \"" + enum_type_name +
                    "\" has no value " + self._value_to_hex() + ")"
                )

            if enum_value_name == "":
                enum_value_name = self._value_to_hex() + " " + error_msg
            else:
                if len(enum_value_name) > 1:
                    enum_value_name = sorted(enum_value_name)

                    # There are multiple matches for the values. Display all
                    # of them one after the other.
                    temp_name = ""
                    for value_name in enum_value_name:
                        if temp_name == "":
                            temp_name += "( "
                        elif value_name == enum_value_name[-1]:
                            temp_name += " and "
                        else:
                            temp_name += ", "
                        temp_name += value_name
                    temp_name += " have value " + self._value_to_hex()
                    temp_name += " in " + enum_type_name + ")"
                    enum_value_name = self._value_to_hex() + " " + temp_name
                else:
                    # get_enum panics if there are no matches so we are
                    # sure that enum_value_name has at lest one element.
                    temp_name = enum_value_name[0]
                    enum_value_name = temp_name + " " + self._value_to_hex()
            # concatenate the return string.
            ret_string += self.base_name + ": " + enum_value_name + "\n"
        elif self.base_name != "":
            # Just display the base name and value.
            ret_string += self.base_name + ": " + self._value_to_hex() + "\n"
        else:
            # Display the value and name.
            ret_string += self.name + ": " + self._value_to_hex() + "\n"
        return ret_string

    def var_to_str(self, depth=0):
        """Converts a structure to a base_name: value string.

        Args:
            depth (int, optional)
        """
        depth_str = "  " * depth
        fv_str = ""

        if depth == 0:
            fv_str += "0x%08x " % self.address

        if self.members:
            if self.base_name == "":
                # Probably it was a pointer to something
                fv_str += (depth_str + self.name + ":\n")
            else:
                fv_str += (depth_str + self.base_name + ":\n")
            for member in self.members:
                fv_str += member.var_to_str(depth + 1)
        else:
            part_of_array = False
            if self.parent and \
                    isinstance(self.parent.array_len, int):
                # this member is an element of an array.
                part_of_array = True
                fv_str += (
                    depth_str + "[" + str(self.parent.members.index(self)) +
                    "]"
                )
                # no need to add additional depth string.
                depth_str = ""

            if self.type_name in self.integer_names:
                # display integers in hex
                if not part_of_array:
                    fv_str += (depth_str + self.base_name)
                fv_str += (": " + self._value_to_hex() + "\n")
            elif "bool" in self.type_name:
                # booleans are displayed as true (1) or false (0).
                fv_str += (depth_str + self.base_name + ": ")
                if self.value != 0:
                    fv_str += ("True\n")
                else:
                    fv_str += ("False\n")
            elif "enum " in self.type_name:
                fv_str += depth_str + self._get_enum_name()
            else:
                # This is probably a pointer to something.
                if not part_of_array:
                    if self.base_name != "":
                        fv_str += (depth_str + self.base_name)
                    else:
                        fv_str += (depth_str + self.name)

                fv_str += (": " + self._value_to_hex() + "\n")

        return fv_str

    def set_debuginfo(self, debuginfo):
        """Sets the debug information for the variable.

        The debuginfo set will be used to better display the variable.

        Args:
            debuginfo: Debug information which will be used to get type
                and enum information.
        """
        self.debuginfo = debuginfo

    def __getitem__(self, key):
        if self.array_len > 0:
            item = self.members[key]
            # Politeness: if .value is an array of length 1, turn it into a
            # simple int.
            if item.value and \
                    not isinstance(item.value, numbers.Integral) and \
                    len(item.value) == 1:
                item.value = item.value[0]
            return self.members[key]

        return None

    def get_member(self, name):
        """Gets a variable member by name.

        Only valid if the variable is a struct or union.

        Note:
            Could have overridden the dot operator here, but wary of a
            clash with actual Variable members.

        Args:
            name
        """
        if not self.members:
            # No members to return!
            return None

        if self.array_len is not None and self.array_len > 0:
            # This is an array; you can't access members by name
            return None

        for member in self.members:
            if member.base_name == name:
                return member
        return None


class SourceInfo(object):
    """A bunch of information about a particular address in PM.

    Args:
        address
        module_name
        src_file
        line_number
    """

    def __init__(self, address, module_name, src_file, line_number):
        self.address = address
        self.module_name = module_name
        self.src_file = src_file
        self.line_number = line_number
        self.nearest_label = None  # a CodeLabel.

        # Note that nearest_label should be filled in on-demand, e.g. by calls
        # to DebugInfo.get_nearest_label(). It's too slow to calculate it
        # up-front for every PM RAM/ROM address.

    def __str__(self):
        if self.nearest_label is None:
            nearest_label_str = "Uncalculated"
        else:
            nearest_label_str = str(self.nearest_label)

        # Using str(x) here since it copes with the value being None
        return (
            'Code address: ' + hex(self.address) + '\n' + 'Module name: ' +
            str(self.module_name) + '\n' + 'Found in: ' + self.src_file +
            ', line ' + str(self.line_number) + '\n' + 'Nearest label is: ' +
            nearest_label_str
        )


class CodeLabel(object):
    """Information about a code label.

    Args
        name: Code label name.
        address: Code label address.
    """

    def __init__(self, name, address):
        self.name = name
        self.address = address

    def __str__(self):
        return self.name + ', address ' + hex(self.address) + '\n'
