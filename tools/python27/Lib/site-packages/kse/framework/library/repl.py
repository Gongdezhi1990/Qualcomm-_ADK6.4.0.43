'''
KATS framework Test Base Debug class
'''

from __future__ import print_function

import platform
import sys

REPL_PYTHON = 'python'
REPL_IPYTHON = 'ipython'
REPL_PTPYTHON = 'ptpython'
REPL_BPYTHON = 'bpython'

REPL_LIST = [REPL_PYTHON, REPL_IPYTHON, REPL_PTPYTHON, REPL_BPYTHON]


def launch_ipython(banner, namespace, magic=None, *args, **kwargs):
    '''
    Invoke the ipython Read Evaluate Print Loop

    Args:
        banner (str): Display banner
        namespace (dict): Namespace to use
    '''

    from IPython.terminal.prompts import Prompts, Token
    from IPython.terminal.embed import InteractiveShellEmbed
    from traitlets.config.loader import Config

    class KatsPrompt(Prompts):
        '''
        Custom ipython prompt class
        '''

        def in_prompt_tokens(self, cli=None):
            _ = cli
            import kats
            return [(Token, kats.__name__),
                    (Token.Prompt, ' > ')]

        def out_prompt_tokens(self):
            return [(Token, '    '),
                    (Token.Prompt, ' < ')]

    cfg = Config()
    cfg.TerminalInteractiveShell.prompts_class = KatsPrompt

    shell = InteractiveShellEmbed(header='',
                                  banner1=banner,
                                  exit_msg='See you again!!!',
                                  config=cfg,
                                  user_ns=namespace)
    if magic:
        shell.register_magics(magic(shell, *args, **kwargs))

    shell()


def launch_ptpython(banner, namespace):
    '''
    Invoke the ptpython Read Evaluate Print Loop

    Args:
        banner (str): Display banner
        namespace (dict): Namespace to use
    '''

    import os

    home = os.path.expanduser("~")
    configure = None
    filename = os.path.join(home, '.ptpython', 'config.py')
    if os.path.isfile(filename):
        print('Loading configuration file %s' % (filename))
        import imp
        config_module = imp.load_source('ptpython_config', filename)
        configure = config_module.configure

    from ptpython.repl import embed  # @UnresolvedImport pylint: disable=import-error
    print(banner)
    embed(globals(), namespace, configure=configure)


def launch_bpython(banner, namespace):
    '''
    Invoke the bpython Read Evaluate Print Loop

    Args:
        banner (str): Display banner
        namespace (dict): Namespace to use
    '''

    if sys.platform == 'win32':
        # in windows the curses implementation does not work and it has to be patched
        from bpython.cli import main as bp_main  # @UnresolvedImport pylint: disable=import-error
        bp_main(locals_=namespace, banner=banner, args=['-i'])
    else:
        from bpython import embed  # @UnresolvedImport pylint: disable=import-error
        embed(locals_=namespace, banner=banner, args=['-i'])


def launch_python(banner, namespace):
    '''
    Invoke the python standard Read Evaluate Print Loop

    Args:
        banner (str): Display banner
        namespace (dict): Namespace to use
    '''

    print('type quit() to exit')
    import code
    try:
        import kats
        sys.ps1 = '%s > ' % (kats.__name__)
        code.interact(banner, local=namespace)
    except Exception:  # pylint: disable=broad-except
        pass


def launch_repl(repl, namespace, extra_banner=None, *args, **kwargs):
    '''
    Invoke the Read Evaluate Print Loop interface

    This will try to invoke the requested repl with the provided namespace in kwargs
    if it fails it will invoke a normal python interpreter session

    Args:
        repl (str): Read Evaluate Print Loop tool to use
        kwargs (dict): Set of keywords and values to be added to the debug session
    '''

    if repl is not None and repl not in REPL_LIST:
        raise RuntimeError('repl %s not supported' % (repl))

    banner = 'Python %s (%s, %s) [%s] on %s' % \
             (platform.python_version(), platform.python_build()[0],
              platform.python_build()[1],
              platform.python_compiler(), sys.platform)
    if extra_banner:
        banner = '\n\n'.join((banner, extra_banner))

    try:
        if repl == REPL_IPYTHON:
            launch_ipython(banner, namespace, *args, **kwargs)
        elif repl == REPL_PTPYTHON:
            launch_ptpython(banner, namespace)
        elif repl == REPL_BPYTHON:
            launch_bpython(banner, namespace)
        else:
            raise ImportError
    except ImportError:
        launch_python(banner, namespace)
