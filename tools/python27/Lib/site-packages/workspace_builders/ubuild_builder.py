from .base_builder import BaseBuilder
from .base_builder import BuildError
from ubuild import main as ubuild


class UbuildBuilder(BaseBuilder):
    def _construct_ubuild_args(self, project, config):
        """ Build the arguments list for passing to ubuild"""
        args = self._base_args_copy()
        args += ['-p', project.filename]
        args += ['-c', config.name]
        return args

    def _run_builder(self, project, config):
        """ Call the ubuild module to perform the actual build. """
        super(UbuildBuilder, self)._run_builder(project, config)
        args = self._construct_ubuild_args(project, config)
        if not ubuild(args):
            raise BuildError("Failed to build {}".format(project))
