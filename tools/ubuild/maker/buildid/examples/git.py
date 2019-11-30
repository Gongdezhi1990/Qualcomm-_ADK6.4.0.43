import sys
import errno
import os
import subprocess
from maker.buildid.writer import BuildIdWriter

GIT_USUAL_WIN_PATHS = [
    os.path.join(os.sep, 'msys64', 'usr', 'bin'),
    os.path.join(os.sep, 'msys32', 'usr', 'bin'),
    os.path.join(os.sep, 'cygwin', 'bin'),
    os.path.join(os.sep, 'Program Files', 'Git', 'bin'),
    os.path.join(os.sep, 'Program Files(x86)', 'Git', 'bin'),
]


class GitBuildId(object):
    def __init__(self):
        self.__git_exe = self.__find_git()
        self._branch = self.__git(['rev-parse', '--abbrev-ref', 'HEAD'])
        self._hash = self.__git(['rev-parse', '--short', 'HEAD'])
        self._commiter = self.__git(['show', '-s', '--format=%ae', 'HEAD'])

    def get_id_string(self):
        return "QTIL ADK {:s}@{:s} by {:s}".format(self._hash, self._branch, self._commiter)

    def get_id_number(self):
        UINT32_MAX = 4294967295
        return int(self._hash, 16) % UINT32_MAX

    def __git(self, options):
        cmd = [self.__git_exe] + options
        return subprocess.check_output(cmd, universal_newlines=True).strip()

    def __find_git(self):
        for p in GIT_USUAL_WIN_PATHS:
            git_exe = os.path.join(p, 'git.exe')
            try:
                subprocess.check_output([git_exe, '--version'])
                print("Using git in {:s}".format(git_exe))
                return git_exe
            except OSError as e:
                if e.errno != errno.ENOENT:
                    raise
        else:
            raise RuntimeError("ERROR: git not found. Please provide the path to the git executable")


if __name__ == '__main__':
    build_id_string_file = sys.argv[1]
    gen = GitBuildId()
    BuildIdWriter.write(build_id_string_file, gen.get_id_number(), gen.get_id_string())
