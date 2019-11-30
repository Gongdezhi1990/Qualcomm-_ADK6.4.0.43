"""
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd 

Defines  a base class and all the exceptions raisable by both the build and
deploy services and
their helpers
"""
import sys
import traceback
import maker.err_codes as erc


class BDError(Exception):
    """Base class for exceptions in this module."""
    pass

class BDInputError(BDError):
    """Exception raised for errors in the input. """
    def __init__(self, ecode, info):

        # Call the base class constructor with the parameters it needs
        super(BDInputError, self).__init__(ecode)

        # Errors should be a dict of subsidiary error information
        self.ecode = ecode
        self.info = info

def raise_bd_err(ecode, *args):
    """ Checks for a valid error code and raises a build and deploy exception """
    assert ecode in erc.CODES
    raise BDInputError(ecode, args)

LOGGER_INSTANCE = None

class Logger(object):
    """ Singleton for handling various styles of output and debug trace """
    def __init__(self, args):
        """ initialise trace and output style switches from parsed command line
        args"""
        global LOGGER_INSTANCE
        self.trace_on = args.trace_enabled
        self.xml_on = args.xml_output
        LOGGER_INSTANCE = self

    def expand_char_ent_refs(self, line):
        """replaces characters such as !, <, >, &, =, that have meaning in xml with
        the appropriate character entity reference e.g. 
            '>' is replaced with '&gt;'
            '!' is replaced with '&exclamation;'
            '=' is replaced with '&equal;'
        The full range of Character entity references includes a lot of unicode characters. 
        This routine only expands those charcters we expect command line tools to produce.
        """
        if not line:
            return line

        subst_dict = {
            # The TinyXML parser in MDE only understands a subset of character references
            #'!': '&exclamation;',
            #'"': '&quot;',
            #'%': '&percent;',
            #'&': '&amp;',
            #"'": '&apos;',
            #'+': '&add;',
            #'<': '&lt;',
            #'=': '&equal;',
            #'>': '&gt;',
            #======================================
            '&': '&amp;',
            "'": '&apos;',
            '<': '&lt;',
            '>': '&gt;',
        }

        outline = ''

        for c in line:
            if c in subst_dict:
                outline +=  subst_dict[c]
            else:
                outline += c 

        return outline

    def output(self, id, attribs, line):
        """ wraps a line of build process output in XML tags for the IDE or other
        caller

        <buildlog module='build'
                  type='error/warning/info'
                  id='buildstart/etc'
                  core='app/p0'
                  config='debug/release'>
                  <text>logged stdio/stderr</text>
        </buildlog>
        """

        if self.xml_on:
            sys.stdout.write('<buildlog ')
            sys.stdout.write('id="%s" ' % id)
            for attrib, val in attribs.iteritems():
                sys.stdout.write('%s="%s" ' % (attrib, val))
            sys.stdout.write('>\n')

            if id == 'buildend':
                sys.stdout.write('  <exitstatus>%s</exitstatus>\n' % attribs['exit_status'])
            else:
                sys.stdout.write('  %s\n' % '<text>')
                outline = self.expand_char_ent_refs(line)
                sys.stdout.write('    %s' % outline)
                sys.stdout.write('  %s\n' % '</text>')

            sys.stdout.write('%s\n\n' % '</buildlog>')
        else:
            if id == 'buildend':
                sys.stdout.write('exitstatus = %s\n' % attribs['exit_status'])
            else:
                sys.stdout.write('%s' % line)
        sys.stdout.flush()

    def trace(self, line):
        """ prints debug trace to stdout gated by trace_on and xml_on switches """
        if self.trace_on:
            if self.xml_on:
                sys.stdout.write("<!-- " + line + " -->\n")
            else:
                sys.stdout.write(line + "\n")
        else:
            pass

    def err_handler(self, excep):
        """ Report an error and exit with the appropriate error code"""
        if self.xml_on:
            if excep:
                sys.stderr.write('\n<buildlog id="buildoutput" type="error" module="build">\n')
                sys.stderr.write('  <text>\n')
                sys.stderr.write('    Exception Code: %s\n' % excep.ecode)
                sys.stderr.write('    Message: %s\n' % erc.CODES[excep.ecode]['msg'])
                for arg in excep.info:
                    sys.stderr.write('    %s\n' % arg)
                sys.stderr.write('  </text>\n')
                sys.stderr.write('</buildlog>\n')
                sys.exit(erc.CODES[excep.ecode]['val'])
            else:
                exc_type, exc_value, exc_tb = sys.exc_info()
                sys.stderr.write('\n<buildlog id="buildoutput" type="fatal" module="build">\n')
                sys.stderr.write('  <text>\n')
                sys.stderr.write('    Exception Name: %s\n' % exc_type.__name__)
                traceback.print_exception(exc_type, exc_value, exc_tb)
                sys.stderr.write('  </text>\n')
                sys.stderr.write('</buildlog>\n')
                sys.exit(erc.CODES['FATAL_ERROR']['val'])
        else:
            if excep:
                for arg in excep.info:
                    sys.stderr.write('Error %s %s\n' % (excep.ecode, arg))
                sys.exit(erc.CODES[excep.ecode]['val'])
            else:
                exc_type, exc_value, exc_tb = sys.exc_info()
                traceback.print_exception(exc_type, exc_value, exc_tb)
                sys.exit(erc.CODES['FATAL_ERROR']['val'])


def log_buildproc_output(id, attribs, line=None):
    """ Accessor for the singleton logger instance """
    LOGGER_INSTANCE.output(id, attribs, line)

def trace(line):
    """ Accessor for the singleton logger instance """
    LOGGER_INSTANCE.trace(line)

def err_handler(excep):
    """ Accessor for the singleton logger instance """
    LOGGER_INSTANCE.err_handler(excep)
