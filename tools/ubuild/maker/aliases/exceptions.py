"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd
"""


class AliasWarning(RuntimeWarning):
    """
    Base class for warnings in the Aliases processing
    """
    pass


class AliasError(RuntimeError):
    """
    Base class for errors in the Aliases processing
    """
    pass


class AliasDuplicateError(AliasError):
    """
    Raised when a duplicate definition has been found for the same alias
    """
    def __init__(self, alias, values=None):
        msg = "Duplicated alias: {}".format(alias)
        if values:
            msg += ", values={}, {}".format(*values)
        super(AliasDuplicateError, self).__init__(msg)


class AliasUndefinedError(AliasError):
    """
    Raised if an alias without a definition is found when processing a file
    """
    def __init__(self, alias):
        msg = "Undefined alias: {}".format(alias)
        super(AliasUndefinedError, self).__init__(msg)


class AliasCyclicInclusionError(AliasError):
    """
    Alias files can't include themselves at any level. If a cyclic inclusion is found
    this exception is raised
    """
    def __init__(self, filename):
        msg = "Cyclic inclusion of alias file: {}".format(filename)
        super(AliasCyclicInclusionError, self).__init__(msg)