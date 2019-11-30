# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd

from __future__ import print_function
import ka_arch
import ka_break
import ka_core
import ka_enables
import ka_exec
import ka_minim
import ka_fw
import ka_maxim
import ka_memory
import ka_other
import ka_registers
import ka_trans
import kalelfreader_lib_wrappers
import kalmemaccessors
import os
from kal_lib import Lib
from kal_stack import Stack
import kal_sym
from kal_sym import Sym
import sys
import ka_ctypes
from ka_exceptions import NotConnectedError


# Method decorator to grab the docstring from a method of the same name in the given class 'delegated_to'.
def docstring_delegate(delegated_to):
    def func(method):
        method.__doc__ = getattr(delegated_to, method.__name__).__doc__
        return method
    return func
        

class Kalaccess(ka_core.KaCore):
    """
    Defines functions that can be used to debug a Kalimba over a debug transport.

    This object implements a few common functions directly but most functionality is contained
    within sub-objects.
    """

    def __init__(self):
        # Specify the names of fields that are expected to be seen in __setattr__. This
        # includes fields that will be accessed by any class we inherit from.
        # If other values are seen then we can assume that they are invalid
        # register values specified on the command line and we should warn about that.
        self.__dict__["_okfields"] = "_core dm pm minim maxim fw enables brk other trans " + \
                                     "arch reg sym lib stack _ka_dll _cfuncs _exec _memory exec_low brk_low prep " + \
                                     "elf_loader"

        ka_core.KaCore.__init__(self)

        self.reg       = ka_registers.KaReg(self)
        self.minim     = ka_minim.KaMinim(self)
        self.maxim     = ka_maxim.KaMaxim(self)
        self.fw        = ka_fw.KaFw(self)
        self.enables   = ka_enables.KaEnables(self)
        self.brk       = ka_break.KaBreak(self)
        self.other     = ka_other.KaOther(self)
        self.trans     = ka_trans.KaTrans(self)
        self.arch      = ka_arch.KaArch(self)
        # These are private, because their functionality is exposed at the top (i.e. this) level.
        self._exec     = ka_exec.KaExec(self)
        self._memory   = ka_memory.KaMemory(self)
        self.sym       = Sym(self)
        self.lib       = Lib(self)
        self.stack     = Stack(self)

        self.dm = kalmemaccessors.DmAccessor(self)
        self.pm = kalmemaccessors.PmAccessor(self)

    def __del__(self):
        try:
            self.disconnect()
        except:
            pass

    def __getattr__(self, elem):
        """Magic routine to let us trap register read accesses, such as "print kal.pc" """
        # Does elem specify a register name?
        if 'reg' in self.__dict__ and self.__dict__['reg']._check_reg_name(elem):
            return eval("self.reg." + elem)

        # We don't recognise elem as a register name, so raise the same exception
        # that Python normally would if we hadn't implemented __getattr__
        raise AttributeError(str(self.__class__) + " has no attribute '" + elem + "'")

    def __setattr__(self, elem, val):
        """Magic routine to let us trap register writes, such as "kal.pc = 123" """
        if 'reg' in self.__dict__ and self.__dict__['reg']._check_reg_name(elem):
            setattr(self.reg, elem, val)
            return

        # No, so just write elem as a object member (this is what would happen if we hadn't
        # implemented __setattr__ ...

        # .. but warn if an unexpected field is specified
        if elem not in self._okfields:
            # NOTE if this warning gets kicked out due to internal calls - i.e. not
            # because the user has specified an invalid register name - then the
            # field should be added to 'okfields' - see __init__ above
            print(
                "Warning: not a known register name. Setting Python member '%s', but no Kalimba register was set!" %
                elem
            )
        self.__dict__[elem] = val

    def get_version(self):
        """Returns a tuple containing the version of these Python tools, the version of the 
           underlying kalaccess library, and the version of the kalelfreader library."""
        our_version = "kalaccess.py - $Change: 3012700 $"

        kalaccess_version = self.get_ka_version()
        ker = kalelfreader_lib_wrappers.Ker()
        kalelfreader_version = ker.get_version()
        return our_version, kalaccess_version, kalelfreader_version


    @staticmethod
    def get_package_release_version():
        """
        :return: a string containing the release number of the Kalimba Python tools package.
        This is a less detailed, and usually more useful, alternative to get_version().
        """
        return "1.1.6"

    @staticmethod
    def _check_usbsys_version():
        # This check isn't supported on Linux
        if sys.platform.startswith('linux'):
            return

        def usb_drv_str_unrecognised(ver_got):
            print("*******************************")
            print("USB SPI driver (usbspi.sys) version unrecognised.")
            print("Was expecting a string like 2.4.0.1. Got '%s' instead" % ver_got)
            print("Please contact Qualcomm support.")
            print("*******************************")

        # Directly read the version string out of the .sys file
        usbsys_path = os.path.join(os.environ["SystemRoot"], "system32/drivers/usbspi.sys")
        try:
            with open(usbsys_path, "rb") as driver_file:
                driver_file_data = driver_file.read()[0::2]  # Slurp the whole file as a unicode string
        except:
            return

        # Find the file version
        idx = driver_file_data.find(b"FileVersion")
        if idx < 0:
            print("*******************************")
            print("USB SPI driver (usbspi.sys) doesn't seem to have a version string.")
            print("Please contact Qualcomm support.")
            print("*******************************")
            return

        # Parse the version string. Here are some examples (whitespace added for clarity):
        #   bluesuite 2.2 = \0 \0 2 . 2               \0  6 \0b \01
        #   bluesuite 2.3 = \0 \0 2 . 3 .  0 . 1 5 \0 \0  6 \0b \01
        #   bluesuite 2.4 = \0 \0 2 . 4 .  0 . 1 5 \0 \90 6 \0b \01
        idx += 13   # Skip "FileVersion" and two NULL chars
        ver_str = driver_file_data[idx:idx + 9]
        end = ver_str.find(b"\0")    # Version string should end in a NULL char
        if end < 0:
            usb_drv_str_unrecognised(ver_str)
            return

        ver_nums = ver_str.split(b".")
        if len(ver_nums) < 2 or len(ver_nums) > 4:
            usb_drv_str_unrecognised(ver_str)
            return

        if int(ver_nums[0]) < 2 or int(ver_nums[1]) < 2:
            print("*******************************")
            print("USB SPI driver (usbspi.sys) version is too old. Please update it. '%s' found" % ver_str)
            print("*******************************")

    def _connect_auto(self, ignore_fw):
        print("\nSearching for available Kalimbas...", end=' ')
        kalimbas = self.trans.enumerate_transports().devices
        if len(kalimbas) == 0:
            print("No Kalimbas found")
            return False
        
        if len(kalimbas) == 1:
            idx = 0
            print("\rAttempting to connect to:          \n", kalimbas[idx])
        else:
            print("\rAvailable Kalimba cores:           ")
            for idx, val in enumerate(kalimbas):
                print("  {0}  -  {1}".format(idx + 1, val))

            print("\nType a number:", end=' ')

            # Py2/3
            try:
                # noinspection PyUnresolvedReferences
                rinput = raw_input
            except NameError:
                rinput = input

            idx = int(rinput())
            if idx < 1 or idx > len(kalimbas):
                print("Invalid input")
                return False
    
            idx -= 1
            
        trans = kalimbas[idx].transport_string
        subsys = kalimbas[idx].subsystem_id
        processor = kalimbas[idx].processor_id

        conn_details = ka_core.ka_connection_details(
            ka_ctypes.CTypesStrIn.from_param(trans),
            subsys,
            processor,
            None
        )
        ka_core.KaCore.connect(self, conn_details, ignore_fw, 0)
        
        return True
    
    def _report_connected_chip(self):
        # Say what we're connected to
        article = "a"
        if self.arch.get_chip_name()[0] in "aeiou":
            article = "an"
        print("Connected to", article, self.arch.get_chip_name())

    def _report_interlock_status(self, ignore_fw):
        # Are we attached to a device that might support the kalimba interlock?
        if self.arch.get_arch() in [2, 3]:
            host_status_addr = self.fw.get_interlock_host_status_addr()
            fw_status_addr = self.fw.get_interlock_fw_status_addr()
            # If interlock detected, print some helpful info
            if host_status_addr != 0 or fw_status_addr != 0:
                print("Kalimba Interlock: fw=0x%x host=0x%x" % (fw_status_addr, host_status_addr))
                if ignore_fw:
                    print("Ignoring the interlock anyway")
            else:
                print("Could not find the Kalimba Interlock")

    def _check_spi_stack(self):
        # Make sure the USB<>SPI adapter firmware is a working one
        firmware_ver = self.trans.trans_get_var("BABELVERSION")
        firmware_ver = int(firmware_ver, 16)
        if firmware_ver < 0x0112:
            print("*******************************")
            print("Your USB<>SPI adapter has firmware version", firmware_ver, "on it.")
            print("Only versions after 0112 are known to work.")
            print("You should reflash your USB<>SPI adapter; please contact Qualcomm support.")
            print("*******************************")

        # Make sure the USB SPI driver is reasonably up-to-date
        self._check_usbsys_version()

    def connect(self, trans="autodetect", mul=-1, ignore_fw=0, subsys=-1, processor=0):
        """Connect to a Kalimba chip or simulator.

        'trans' must be one of
            'autodetect' (the default)
            a PTTrans string (eg 'SPITRANS=usb' or 'SPITRANS=kalsim')
            a PTTrans string with the 'SPITRANS=' prefix missing
            a TRB device (eg 'trb/usb2trb/0' for the first usb2trb dongle found,
                          or 'trb/usb2trb/12345' for a specific usb2trb dongle)
            a Lauterbach/JTAG device (e.g. 'lauterbach/jtag' for Lauterbach TRACE32
                                      running at the default localhost:20000 or
                                      'tcp/somehost:12345/lauterbach/jtag' to 
                                      use a specific hostname and port).
            a usb2tc / USBDbg device (e.g. 'usb2tc/1')

        'mul' [optional] specifies the SPI multiplexor ID. This needs to be set
            on some FPGA boards, generally the default value is fine. Only applicable
            if trans is a SPI transport name.

        'ignore_fw' [optional]. Set ignore_fw=1 if you want to ignore the FW/Host
            Kalimba Keyhole interlock. Only applicable on CSR8670.

        'subsys' [optional]. This must be specified if you are connecting to a Hydra
        based chip. Acceptable values are 0 to 15.
        
        'processor' [optional]. Specify this if you want to connect to the nth core
        of a multi-core Kalimba.
        
        
        Notes:
        ======

        If using a PtTrans string, other pttransport variables can also be set within the 
        'trans' parameter.
        
        Useful pttransport variables include:

        'SPIMAXCLOCK=X' - allows slower than normal SPI clockrates to be specified. X is
            in kHz. The default max clock is 1000 kHz. This setting is meaningless for 
            kalsim.

            eg connect("usb SPIMAXCLOCK=80")

        'SPIPORT=X' - Selects which port to use.
            For USB, this allows you to specify which USB<>SPI adapter to use if you have more than
                one.
                Possible values are 0, 1, 2..., 0 is the default.
            For kalsim, this allows you to specify which kalsim binding to use. The
                bindings are specified in the system environment variable called
                KALSIM_HOSTS. For example, the following will configure two bindings,
                corresponding to SPIPORT=2 and SPIPORT=3:
                    KALSIM_HOSTS="localhost=123;camunxgrd93=12345"
                SPIPORT=1 corresponds to the default binding, which is always
                localhost=31400.
                eg connect("kalsim SPIPORT=2")
        """

        # Close any existing connection
        try:
            self.disconnect()
        except:
            pass

        if trans.lower() == "autodetect":
            if not self._connect_auto(ignore_fw):
                return
        else:
            # Add SPITRANS preamble and SPIMUL if appropriate
            non_spi_transports = ["trb", "lauterbach", "tc/usb2tc"]
            if all(nst not in trans.lower() for nst in non_spi_transports):
                if not trans.startswith("SPITRANS="):
                    trans = "SPITRANS=" + trans
                if mul != -1:
                    trans += " SPIMUL=" + str(mul)
            conn_details = ka_core.ka_connection_details(
                ka_ctypes.CTypesStrIn.from_param(trans),
                subsys,
                processor,
                None
            )

            # Call kalaccess_lib_wrapper's connect func
            ka_core.KaCore.connect(self, conn_details, ignore_fw, 0)
            
        self._report_connected_chip()
        if "kalsim" not in trans.lower():
            self._report_interlock_status(ignore_fw)
        if "spitrans=usb" in trans.lower():
            self._check_spi_stack()

        # Print if Kalimba is clocked and enabled / running
        self.is_running()

    @docstring_delegate(ka_core.KaCore)
    def connect_with_uri(self, core_uri):
        # Close any existing connection
        try:
            self.disconnect()
        except:
            pass
            
        ka_core.KaCore.connect_with_uri(self, core_uri)
        
        self._report_connected_chip()
        if not any(sim_pat in core_uri.lower() for sim_pat in ["/kalsim/", "/sim/", "/tcpsim/"]):
            # We do not yet support the ignore_fw flag in URIs.
            self._report_interlock_status(False)
        if "/spi" in core_uri.lower() and \
                any(usbspi_pat in core_uri.lower() for usbspi_pat in ["/usb2spi/", "/babel/", "/usb/"]):
            self._check_spi_stack()
        self.is_running()
        
    def is_running(self):
        """
        Returns true if the Kalimba is running. Prints a warning if Kalimba is not
        clocked and/or enabled.
        """
        if self.arch.is_hydra():
            ok = True
            connected_subsystem = self.arch.get_subsystem_id()
            if not self.enables.read_subsystem_up(connected_subsystem):
                print("Connected Kalimba subsystem (%d) is not up" % connected_subsystem)
                ok = False
            if not self.enables.read_subsystem_clocked(connected_subsystem):
                print("Connected Kalimba subsystem (%d) is not clocked" % connected_subsystem)
                ok = False
            if not self.enables.read_subsystem_powered(connected_subsystem):
                print("Connected Kalimba subsystem (%d) is not powered" % connected_subsystem)
                ok = False
            if not self.enables.read_subsystem_run(connected_subsystem):
                print("Connected Kalimba subsystem (%d) is not run-enabled" % connected_subsystem)
                ok = False
            if not ok:
                return False
        else:
            if not self.arch.is_always_enabled():
                if not self.enables.read_dsp_clock_enable():
                    print("Kalimba clock not enabled")

                if not self.enables.read_dsp_enable():
                    print("Kalimba not enabled")

        return self._exec.get_exec_state().is_running()

    # The following methods are implemented as forwarding functions here for convenience (i.e., to save typing)

    @docstring_delegate(ka_exec.KaExec)
    def run(self):
        return self._exec.run()

    @docstring_delegate(ka_exec.KaExec)
    def run_to(self, address, instruction_set_at_target=None):
        return self._exec.run_to(address, instruction_set_at_target)

    def pause(self):
        """Pauses the Kalimba if it is currently running."""
        return self._exec.pause()

    def step(self):
        """Steps the Kalimba processor. This function will step over instruction prefixes so that
           the program counter does not end up on a prefix."""
        return self._exec.step()

    def step_over(self):
        """Steps the Kalimba processor, stepping over call instructions."""
        return self._exec.step_over()

    @docstring_delegate(ka_exec.KaExec)
    def get_exec_state(self):
        return self._exec.get_exec_state()

    def _read_pm_block(self, start_addr, num_words):
        return self._memory.read_pm_block(start_addr, num_words)
        
    def _write_pm_block(self, start_addr, data):
        return self._memory.write_pm_block(start_addr, data)
        
    def _read_dm_block(self, start_addr, num_words):
        return self._memory.read_dm_block(start_addr, num_words)
        
    def _write_dm_block(self, start_addr, data):
        return self._memory.write_dm_block(start_addr, data)

    def frac_to_float(self, data):
        """
        Wrapper for the free function frac_to_float(), inferring the data width from the connected chip.
        """
        if not self.is_connected():
            raise NotConnectedError('Need a chip connection to infer data width. Use kalaccess.frac_to_float() to use '
                                    'offline.')
        return frac_to_float(data, self.arch.get_data_width())

    def float_to_frac(self, data):
        """
        Wrapper for the free function float_to_frac(), inferring the data width from the connected chip.
        """
        if not self.is_connected():
            raise NotConnectedError('Need a chip connection to infer data width. Use kalaccess.float_to_frac() to use '
                                    'offline.')
        return float_to_frac(data, self.arch.get_data_width())

    def help(self):
        """Displays help on how to use this object. This is slightly more useful than the auto-generated
        help because it includes details of the sub-objects."""
        print(self.__doc__)

        funcs = []
        subobs = []
        for i in dir(self):

            suppress = ['add_cfunc', 'extract_c_integer_constant', 'handle_error']
            if i.startswith('_') or i in suppress:
                continue

            try:
                from types import InstanceType
            except ImportError:
                InstanceType = object

            from types import MethodType

            t = eval("type(self." + i + ")")
            if t == MethodType:
                funcs.append(i)
            elif t == InstanceType or str(t).startswith('<class '):
                subobs.append(i)

        print("    Functions:\n       ", "\n        ".join(funcs))
        print("\n    Subobjects:\n       ", "\n        ".join(subobs))

        print("    Use the standard Python help command for further info on the functions or sub-objects.")


def toHex(data):
    return kal_sym.toHex(data)


def frac_to_float(data, frac_width_bits):
    """
    Converts binary data representing Kalimba fractionals into Python floats.
    e.g. 0x007fffff is converted to 0.999999
    Data can be a single value, or an array/list of values.
    """
    m = 1.0 / (2 ** (frac_width_bits - 1))

    def single_frac_to_float(value):
        if value < 0 or value > ((2 ** frac_width_bits) - 1):
            raise OverflowError('Input value {0:#x} outside acceptable {1}-bit range'.format(value, frac_width_bits))

        return value * m - ((value >> (frac_width_bits - 2)) & 2)

    try:
        return list(map(single_frac_to_float, data))
    except TypeError:
        return single_frac_to_float(data)


def frac24_to_float(data):
    return frac_to_float(data, frac_width_bits=24)


def frac32_to_float(data):
    return frac_to_float(data, frac_width_bits=32)


def float_to_frac(data, width):
    """
    Converts Python floats into Kalimba fractionals (see the Kalimba Architecture User Guide);
    e.g. 0.9999999 is converted to 0x007fffff.
    The range of acceptable input values is [-1.0, 1.0); this function raises an OverflowError if an input value is
    outside this range.
    :param data: can be a single value or a sequence of values.
    :param width: the bit width of the output word(s). See also the convenience functions float_to_frac24() and
    float_to_frac32().
    """
    def single_float_to_frac(i):
        if (i < -1.0) or (i >= 1.0):
            raise OverflowError('float_to_frac: input value {0} outside range [-1.0, 1.0)'.format(i))
        i = int(i * 2**(width - 1))
        if i < 0:
            return i + 2**width
        return i

    try:
        return list(map(single_float_to_frac, data))
    except TypeError:
        return single_float_to_frac(data)


def float_to_frac24(data):
    return float_to_frac(data, width=24)


def float_to_frac32(data):
    return float_to_frac(data, width=32)


def s24_to_int(data, width=24):
    """
    Converts binary data representing Kalimba 24 bit signed numbers into Python integers.
    e.g. 0x00ffffff is converted to -1.
    Data can be a single value, or an array/list of values.
    """
    mask = 1 << (width-1)
    try:
        return list(map(lambda x: x - ((x & mask) << 1), data))
    except TypeError:
        return data - ((data & mask) << 1)


def int_to_s24(data, width=24):
    """
    Converts Python integers to Kalimba 24-bit signed numbers.
    e.g. -1 is converted to 0x00ffffff.
    Data can be a single value, or an array/list of values.
    """
    mask = 1 << width

    def to_s24(datum):
        if datum < 0:
            return mask + datum
        else:
            return datum

    try:
        return list(map(to_s24, data))
    except TypeError:
        return to_s24(data)
