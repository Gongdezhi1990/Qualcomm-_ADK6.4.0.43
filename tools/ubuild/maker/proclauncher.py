"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd 

Provides portable mechanism for launching a subordinate build tool such as make
or python scripts. The tool is launched in its own process with the specified
enviroment and command line arguments.

The build tool's STDERR is redirected to STDOUT to preserve ordering. STDOUT is
captured and is forwarded to a multiprocessing.Pipe() for capture by another
process. See test_proc_launcher for an example.
"""
import subprocess as SP

class ProcLauncher(object):
    """ Launches the subordinate process with environment and command line arguments. """
    def __init__(self, cmd=None, env_vars=None, cwd=None,
                 output_pipe=None):
        '''
        NB: On Windows, in order to run a side-by-side assembly the specified env
        must include a valid SystemRoot.
        '''
        self.output_pipe = output_pipe

        self.proc = SP.Popen(cmd,
                             stdout=SP.PIPE,
                             stderr=SP.STDOUT,
                             env=env_vars,
                             cwd=cwd,
                             universal_newlines=True,
                             startupinfo=None,
                             creationflags=0)

    def run(self):
        """ capture the child's STDOUT and wait for it to finish. Return the
        exit status of the child """
        try:
            self.child_output_handler(self.proc.stdout)
            self.proc.wait()
            return self.proc.returncode
        except BaseException as excep:
            print "Hit exception", type(excep), "val =", excep

    def kill(self):
        """ terminate the launched process """
        self.proc.kill()

    def child_output_handler(self, child_out_stream):
        """ process the STDOUT of the child process until it is closed """
        while True:
            line = child_out_stream.readline()
            if line != '':
                self.output_pipe.send('%s' % (line))
            else:
                self.output_pipe.close()
                return

