
from abc import abstractmethod, ABCMeta


class BuildError(RuntimeError):
    pass


class DefaultBuilder:
    def build(self, project):
        raise NotImplementedError("No builder defined for the workspace")


class BaseBuilder(object):
    __metaclass__ = ABCMeta

    def __init__(self, args=''):
        self.base_args = args
        self.built = set()
        self.finalBuild = set()

    def build(self, project):
        """ Build a project and all its dependencies """
        self._build_project_with_dependencies(project)

    def final_build(self):
        for step in self.finalBuild:
            self._build_project(step)

    def _build_project_with_dependencies(self, project):
        """ Walk through each dependency and build them """
        for dep in project.dependencies:
            self._build_project_with_dependencies(dep)
        else:
            if project.default_configuration.is_final_build():
                self.finalBuild.add(project)
            elif self._project_not_built(project):
                self.built.add(project)
                self._build_project(project)

    def _project_not_built(self, project):
        """ Returns True if the project hasn't been built yet """
        return project not in self.built

    def _build_project(self, project_to_build):
        """ Build an individual project """
        config = project_to_build.default_configuration
        if self._is_project_buildable(config):
            self._run_builder(project_to_build, config)

    def _is_project_buildable(self, config):
        """ Returns True if the project can be built """
        return config.is_buildable()

    def _base_args_copy(self):
        """ Get a new list initialized with the contents of base_args """
        return list(self.base_args)

    @abstractmethod
    def _run_builder(self, project, config):
        """ Call the builder module to perform the actual build.
            The method must raise a BuildError if the build step fails
        """
        print("Building project: {}, config: {}".format(project, config))
