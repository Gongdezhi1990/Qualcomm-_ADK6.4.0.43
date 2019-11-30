"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd

Package initialisation file

When importing a package, Python searches through the directories on sys.path
looking for the package subdirectory.

The __init__.py files are required to make Python treat directories as
containing packages; this is done to prevent directories with a common name,
such as 'string', from unintentionally hiding valid modules that occur later in
the module search path. In the simplest case, __init__.py can just be an empty
file, but it can also execute initialization code for the package or set the
__all__ variable, described later.

See https://docs.python.org/2/tutorial/modules.html for details
"""
