############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module used for testing if the elf is supported.
"""
import ACAT.Core.Arch as Arch

from ACAT.Core.exceptions import (
    DebugInfoNoVariableError, InvalidDebuginfoTypeError,
    InvalidDebuginfoEnumError
)


class StructureTree(object):
    """Structure Tree.

    This class takes Variable object and returns its structure
    representation as a tree, name of variable being a root and members as
    branches. Every branch of a tree is another tree, thus many layers can
    exist to fully represent the structure>

    Args:
        var: Variable structure.
        indent: Indentation of every layer of the tree for string
            representation purposes.
    """

    def __init__(self, var, indent=0):
        self.indent = indent
        self.next_layer = []
        self.size = var.size
        self.name = var.name
        self.members = var.members
        # if more than one layer
        if self.members:
            for member in self.members:
                self.next_layer.append(StructureTree(member, self.indent + 1))
        else:
            self.next_layer = None

    def __str__(self):
        out_str = '{0}{1}  size: {2}\n'.format(
            '    ' * self.indent, self.name, str(self.size)
        )
        if self.next_layer is not None:
            for member in self.next_layer:
                out_str += str(member)
        return out_str

    def compare(self, tree, equals=True):
        """Compares another Structure Tree instance with this one.

        Two tree structures are equal if names and sizes of all branches
        of a tree are equal.

        Args:
            tree (:obj:`StrucutreTree`): tree instance that we want to
                compare current object with
            equals (bool)

        Returns:
            bool: True if equal and False otherwise.
        """
        equals = (
            equals and
            self.name == tree.name and
            self.size == tree.size and
            len(self.members) == len(tree.members)
        )
        if self.members:  # no more layers to compare
            return equals
        elif equals:  # if already false, no need to dig deeper
            for member, val in zip(self.next_layer, tree.next_layer):
                equals = equals and member.compare(val, equals)
        return equals

    def contains_member(self, member_name):
        """Checks if a member is part of the type structure.

        This method takes member name and checks whether this member is
        part of the type structure (represented as a tree). The member
        name can be a full name containing the whole structure name.
        e.g. 'struct ENDPOINT.ep_to_kick' or just the member name
        e.g 'ep_to_kick'.

        Args:
            member_name (str): The member name we will search in the
                variable.

        Returns:
            bool: True if exists, False otherwise.
        """
        current_var_name = self.name

        if current_var_name == member_name:
            # The variable is found.
            return True

        # Check if the variable name terminates with member name.
        # For ex: "con_data.isp" in "struct ACCMD_CON.con_data.isp"
        member_name_ext = "." + member_name
        if current_var_name[-len(member_name_ext):] == member_name_ext:
            return True

        if self.next_layer is not None:
            # check all member for match.
            for member in self.next_layer:
                if member.contains_member(member_name):
                    return True

        return False


def _check_constants(processor, analysis_name, constant_dict):
    """Checks the presence of the constants used by ACAT analyses.

    These constants can be of the following type:

           strict - These constants must be present in the build and absence
                    of them will result in at least one analysis failing
                    thus if we are missing them, error is issued.
           one_strict - This is a tuple of constants. From a tuple at least one
                        constant must be present in order for analysis not to
                        fail. They usually appear in try - except or if - else
                        clauses. If none appear in the build, error is issued.
           Architecture specific constants (e.g. 'Hydra', 'Bluecore') - these
               constants must be present if according architecture is used
               other_architecture - these constants must be present if other
               architecture is used (not currently implemented).
           p1 - These constants must be used if processor p1 is used and
                not p0 (for current tests only p0 is used).
           old_style - These constants are used if old style interface used
                       for ACAT.
           not_strict - Other constants that might be used by analyses, but
                       their absence will not result in failing analysis.
                       they usually appear in try - except clauses with except
                       being 'pass'.

    Args:
        processor: Pointer to the processor containing the chipdata and
            debug information where the analysis is defined.
        analysis_name (str):  Name of the analysis.
        constant_dict (dict): The constants used in the analysis.
    """
    for constant_type, constants in sorted(constant_dict.items()):
        if constant_type == 'strict' or (constant_type == Arch.chip_arch):
            for const in constants:
                try:
                    processor.debuginfo.get_constant_strict(const)
                except DebugInfoNoVariableError:
                    processor.formatter.error(
                        'Constant expected by %s not found in build: %s' % (
                            const, analysis_name))
        elif constant_type == 'one_strict':
            found = False
            for constants_tuple in constants:
                for const_tuple in constants_tuple:
                    try:
                        processor.debuginfo.get_constant_strict(const_tuple)
                        found = True  # at least one constant found
                    except DebugInfoNoVariableError:
                        pass
                if not found:
                    processor.formatter.error(
                        ' %s expects one of the constants: %s None found' % (
                            analysis_name, constants_tuple))
        elif constant_type == 'not_strict':
            for constants_tuple in constants:
                try:
                    processor.debuginfo.get_constant_strict(constants_tuple)
                except DebugInfoNoVariableError:
                    processor.formatter.alert(
                        "Constant which might be used by %s which is not "
                        "present: %s" % (
                            analysis_name,
                            constants_tuple
                        )
                    )


def _check_variables(processor, analysis_name, variable_dict):
    """Checks the presence of the variables used by ACAT analyses.

    These variables can be of the following type:
           strict - These variables must be present in the build and absence
               of them will result in at least one analysis failing thus if we
               are missing them, error is issued.
           one_strict - This is a tuple of variables. From a tuple at least one
               variable must be present in order for analysis not to fail.
               They usually appear in try - except or if - else clauses. If
               none appear in the build, error is issued.
           architecture specific variables (e.g. 'Hydra', 'Bluecore') - These
               variables must be present if according architecture is used.
           other_architecture - These variables must be present if other
               architecture is used (not currently implemented).
           p1 - These constants must be used if processor p1 is used and not
               p0 (for current tests only p0 is used).
           old_style - These variables are used if old style interface used for
               ACAT.
           not_strict - Other variables that might be used by analyses, but
               their absence will not result in failing analysis. they usually
               appear in try - except clauses with except being 'pass'.
           one_not_strict - This is a tuple of variable names. Not more than
               one of those variables should be present in the chipdata for the
               given analysis. When none of them or more than one of them are
               present, there will be suitable alerts.

    Args:
        processor: Pointer to the processor containing the chipdata and
            debug information where the analysis is defined.
        analysis_name (str):  Name of the analysis.
        variable_dict (dict): The variables used in the analysis.
    """
    for variable_type, variables in sorted(variable_dict.items()):
        if variable_type == 'strict' or (variable_type == Arch.chip_arch):
            for var in variables:
                try:
                    processor.chipdata.get_var_strict(var)
                except DebugInfoNoVariableError:
                    processor.formatter.error(
                        'Variable expected by %s not found in build: %s' % (
                            var, analysis_name))
        elif variable_type == 'one_strict':
            # at least one variable must be present
            found = False
            for variable_tuple in variables:
                for var in variable_tuple:
                    try:
                        processor.chipdata.get_var_strict(var)
                        found = True  # at least one constant found
                    except DebugInfoNoVariableError:
                        pass
                if not found:
                    processor.formatter.error(
                        ' %s expects one of the variables: %s None found' % (
                            analysis_name, variable_tuple))
        elif variable_type == 'not_strict':
            for var in variables:
                try:
                    processor.chipdata.get_var_strict(var)
                except DebugInfoNoVariableError:
                    processor.formatter.alert(
                        'Variable which might be used by %s which is '
                        'not present: %s' % (
                            var,
                            analysis_name
                        )
                    )
        elif variable_type == "one_not_strict":
            any_exists = False
            for var in variables:
                if processor.chipdata.identifier_exists(var) and any_exists:
                    processor.formatter.alert(
                        "{} analysis expects to see at most one of {}".format(
                            analysis_name,
                            variables
                        )
                    )
                    return
                elif processor.chipdata.identifier_exists(var):
                    any_exists = True

            if any_exists is False:
                processor.formatter.alert(
                    "None of {} variables exist for {} Analysis.".format(
                        variables,
                        analysis_name
                    )
                )


def _check_depending_types(processor, analysis_name, type_dict):
    """Checks  if the structure of the used types is valid for the analysis.

    This function takes the information about the build and tests whether
    structure of the used types still valid for the analysis.

    Args:
        processor: Pointer to the processor containing the chipdata and
            debug information where the analysis is defined.
        analysis_name (str):  Name of the analysis.
        type_dict (dict): The types used in the analysis.
    """
    for internal_type, members in sorted(type_dict.items()):
        try:
            variable = processor.chipdata.get_var_strict(internal_type)
            type_tree = StructureTree(variable)
        except DebugInfoNoVariableError:
            try:
                variable = processor.chipdata.cast(0x0, internal_type)
                type_tree = StructureTree(variable)
            except InvalidDebuginfoTypeError:
                processor.formatter.alert(
                    'Type: %s does not exist in build which might be used in '
                    'analysis: %s' % (
                        internal_type,
                        analysis_name
                    )
                )
                continue

        for member in members:
            member_exist = type_tree.contains_member(member)
            if not member_exist:
                processor.formatter.error(
                    'Analysis %s might fail since member %s is not '
                    'present in %s' % (
                        analysis_name,
                        member,
                        internal_type
                    )
                )


def _check_enums(processor, analysis_name, enums_dict):
    """Checks the presence of the enums used by ACAT analyses.

    These enums can be of the following type:
           strict - these enums must be present in the build and absence of
               them will result in at least one analysis failing thus if we are
               missing them, error is issued
           architecture specific enums (enum.g. 'Hydra', 'Bluecore') - these
               enums must be present if according architecture is used
           other_architecture - these enums must be present if other
               architecture is used (not currently implemented)
           p1 - these enums must be used if processor p1 is used and not p0
               (for current tests only p0 is used)
           old_style - these enums are used if old style interface used
                       for ACAT
           not_strict - other enums that might be used by analyses, but their
                        absence will not result in failing analysis.
                        they usually appear in try - except clauses with except
                        being 'pass'

    Args:
        processor: Pointer to the processor containing the chipdata and
            debug information where the analysis is defined.
        analysis_name (str): Name of the analysis.
        enums_dict (dict): The enums used in the analysis.
    """
    for enum_type, enums in sorted(enums_dict.items()):
        if enum_type == 'strict' or (enum_type == Arch.chip_arch):
            for enum in enums:
                try:
                    processor.debuginfo.get_enum(enum, None)
                except InvalidDebuginfoEnumError:
                    processor.formatter.error(
                        'Enum expected by %s not found in build: %s' % (
                            enum, analysis_name))
        elif enum_type == 'one_strict':
            # one of the variables must be present in the build in order for
            # analysis to work
            found = False
            for enum_tuple in enums:
                for enum in enum_tuple:
                    try:
                        processor.debuginfo.get_enum(enum, None)
                        found = True  # at least one constant found
                    except InvalidDebuginfoEnumError:
                        pass
                if not found:
                    processor.formatter.error(
                        '%s expects one of the enum: %s None found' % (
                            analysis_name, enum_tuple)
                    )
        elif enum_type == 'not_strict':
            for enum in enums:
                try:
                    processor.debuginfo.get_enum(enum, None)
                except InvalidDebuginfoEnumError:
                    processor.formatter.alert(
                        'enum which might be used by %s which is not '
                        'present: %s' % (analysis_name, enum)
                    )


def check_dependencies(processor, analysis_name, imported_analysis):
    """Checks if the analyses has all the dependencies in the debug information.

    Args:
        processor: Pointer to the processor containing the chipdata and
            debug information where the analysis is defined.
        analysis_name (str): Name of the analysis.
        imported_analysis: The imported analysis.
    """
    try:
        constant_dict = imported_analysis.CONSTANT_DEPENDENCIES
        _check_constants(processor, analysis_name, constant_dict)
    except AttributeError:
        pass
    try:
        variable_dict = imported_analysis.VARIABLE_DEPENDENCIES
        _check_variables(processor, analysis_name, variable_dict)
    except AttributeError:
        pass
    try:
        type_dict = imported_analysis.TYPE_DEPENDENCIES
        _check_depending_types(processor, analysis_name, type_dict)
    except AttributeError:
        pass
    try:
        enums_dict = imported_analysis.ENUM_DEPENDENCIES
        _check_enums(processor, analysis_name, enums_dict)
    except AttributeError:
        pass
