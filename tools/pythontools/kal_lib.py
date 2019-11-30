# Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd

from __future__ import print_function
import ka_exceptions
import math


class MissingTimerSymbolError(Exception):
    def __init__(self, symbol_name):
        Exception.__init__(self, "Symbol required for timers not found: {0}".format(symbol_name))


class Lib(object):
    """This class provides functions to inspect on-chip data structures used by
    the Kalimba standard library code."""
    def __init__(self, core):
        self._core = core

    def _read_var_with_size_check(self, address, size_in_addressable_units):
        # Check that the variable is a whole number of words. Otherwise, something's wrong.
        if size_in_addressable_units % self._core.arch.dm_address_inc_per_word() != 0:
            raise Exception("Symbol has size {0} octets, which is not a whole number of words"
                            .format(size_in_addressable_units))

        size_in_words = size_in_addressable_units // self._core.arch.dm_address_inc_per_word()
        # noinspection PyProtectedMember
        return self._core._read_dm_block(address, size_in_words)

    def _format_address_for_arch(self, address):
        # Output at a fixed width, appropriate for the target arch, e.g. 6(8) hex digits for 24(32)-bit data width.
        return "0x{addr:0{width}X}".format(addr=address, width=2 * self._core.arch.get_data_width() // 8)
        
    def _print_timer_results(self, timers, time_field_offset, handler_field_offset):
        # Produce a printable set of data, formatted with the right column widths and whatnot.
        longest_timer_len   = 0
        longest_handler_len = 0
        prev_expiry_time    = 0
        printable_info      = []
        for (timer_address, timer_memory) in timers:
            timer_symbol        = self._core.sym.varfind(timer_address)
            timer_name          = timer_symbol.name if len(timer_symbol) > 0 else "<unknown>"
            timer_name_and_addr = "{0} ({1})".format(timer_name, self._format_address_for_arch(timer_address))
            longest_timer_len   = max(longest_timer_len, len(timer_name_and_addr))

            handler_address       = timer_memory[handler_field_offset]
            handler_name_and_addr = "{0} ({1})".format(self._core.sym.modname(handler_address),
                                                       self._format_address_for_arch(handler_address))
            longest_handler_len   = max(longest_handler_len, len(handler_name_and_addr))

            expiry_time      = timer_memory[time_field_offset]
            expiry_offset    = expiry_time - prev_expiry_time
            prev_expiry_time = expiry_time

            printable_info.append((timer_name_and_addr, handler_name_and_addr, expiry_offset))

        if len(printable_info) == 0:
            print("No timers found")
        else:
            print(
                "%-*s %-*s Offset(us) [first absolute]" %
                (longest_timer_len, "Timer Structure", longest_handler_len, "Handler")
            )
            for timer_name_and_addr, handler_name_and_addr, expiry_offset in printable_info:
                print(
                    "%-*s %-*s %d" %
                    (longest_timer_len, timer_name_and_addr, longest_handler_len, handler_name_and_addr, expiry_offset)
                )

    def timers(self):
        """Reports the status of timers registered with the Kalimba standard library."""

        if not self._core.is_connected():
            raise ka_exceptions.NotConnectedError()

        if not self._core.arch.is_bluecore():
            raise ka_exceptions.UnsupportedArchitecture("timers() is not supported on non-Bluecore chips")

        # Strategy is to grab all the data in one go, then print it afterwards.
        def get_const_field(name):
            symbol = self._core.sym.constfind(name)
            if not symbol:
                raise MissingTimerSymbolError(name)
            return symbol.value

        # We need the offsets for various structure fields
        next_field_offset    = get_const_field("$timer.NEXT_ADDR_FIELD")
        time_field_offset    = get_const_field("$timer.TIME_FIELD")
        handler_field_offset = get_const_field("$timer.HANDLER_ADDR_FIELD")
        timer_structure_size = get_const_field("$timer.STRUC_SIZE")

        # Get the address of the timer at the head of the linked list.
        head_timer_symbol = self._core.sym.varfind("$timer.last_addr")
        if len(head_timer_symbol) == 0:
            raise MissingTimerSymbolError("$timer.last_addr")

        # If the processor is running, pause it while we collect data.
        was_running = self._core.is_running()
        if was_running:
            self._core.pause()

        # Read each timer structure in the linked list of timers
        current_timer_address = self._core.dm[head_timer_symbol.addr]
        timers = []
        end_marker = 2**self._core.arch.get_data_width() - 1
        while current_timer_address != end_marker:
            # noinspection PyProtectedMember
            timer_struct_memory = self._core._read_dm_block(current_timer_address, timer_structure_size)
            timers.append((current_timer_address, timer_struct_memory))
            current_timer_address = timer_struct_memory[next_field_offset]

        if was_running:
            self._core.run()

        self._print_timer_results(timers, time_field_offset, handler_field_offset)

    # noinspection PyProtectedMember
    def readcbuffer(self, cbuffer_name, n=0, do_ud='noupdate'):
        """
        Returns the contents of a cbuffer on the Kalimba.
           cbuffer_name can be a regular expression, but must end with 'cbuffer_struc'.
           n specifies the maximum number of words to read.
           Set do_ud to 'update' if the read address should be updated by this function (the default
           is 'noupdate').
        """
        if not self._core.is_connected():
            raise ka_exceptions.NotConnectedError()

        if not self._core.arch.is_bluecore():
            raise ka_exceptions.UnsupportedArchitecture("readcbuffer() is not supported on non-Bluecore chips")

        if not cbuffer_name.endswith("cbuffer_struc"):
            print("cbuffer_name must end with 'cbuffer_struc'")
            return

        cbuff_list = self._core.sym.varfind(cbuffer_name)
        if len(cbuff_list) == 0:
            print("No such cbuffer found")
            return

        if len(cbuff_list) > 1:
            print("More than one cbuffer found with that name. Match must be unique.")
            print("Matching names were:")
            for i in cbuff_list:
                print("   ", i[0])
            return

        cbuff_tuple = cbuff_list[0]
        cbuff_addr = cbuff_tuple[2]  # Fields are name, size, addr

        (sz, rd, wr) = self._core._read_dm_block(cbuff_addr, 3)

        if sz != 0:
            # start and end addresses
            mask = int(2**math.ceil(math.log(sz, 2)) - 1)
            start_addr = rd - (rd & mask)
            end_addr = start_addr + sz - 1

            # available data in buffer
            amount_data = wr - rd
            if amount_data < 0:
                amount_data += sz

            if amount_data == 0:
                return []
            else:
                if n > 0:
                    if n > amount_data:
                        n = amount_data
                        print("Warning: Not enough data in the cbuffer")
                else:
                    n = amount_data

                # end_addr is index of last item of buf
                if (rd + n - 1) > end_addr:
                    data = self._core._read_dm_block(rd, end_addr - rd + 1)
                    m = len(data)
                    data.append(self._core._read_dm_block(start_addr, m))
                    rd = start_addr + m
                else:
                    data = self._core._read_dm_block(rd, n)
                    rd += n

                if do_ud.lower() == 'update':
                    self._core._write_dm_block(cbuff_addr+1, [rd])

                return data
        else:
            print("The size of the cbuffer is 0.")
            return []

    # noinspection PyProtectedMember
    def writecbuffer(self, cbuffer_name, data, do_ud='update'):
        """
        Writes data to the specified cbuffer. Returns the left over data after
        writing as much as it can.
           cbuffer_name can be a regular expression, but must end with 'cbuffer_struc'.
           data is a list that contains the data to be written.
           Set do_ud to 'noupdate' if the write address should not be updated (the default is
           'update').
        """
        if not self._core.is_connected():
            raise ka_exceptions.NotConnectedError()

        if not self._core.arch.is_bluecore():
            raise ka_exceptions.UnsupportedArchitecture("writecbuffer() is not supported on non-Bluecore chips")

        if not cbuffer_name.endswith("cbuffer_struc"):
            print("cbuffer_name must end with 'cbuffer_struc'")
            return

        cbuff_list = self._core.sym.varfind(cbuffer_name)
        if len(cbuff_list) == 0:
            print("No such cbuffer found")
            return

        if len(cbuff_list) > 1:
            print("More than one cbuffer found with that name. Match must be unique.")
            print("Matching names were:")
            for i in cbuff_list:
                print("   ", i[0])
            return

        cbuff_tuple = cbuff_list[0]
        cbuff_addr = cbuff_tuple[2]  # Fields are name, size, addr

        (sz, rd, wr) = self._core._read_dm_block(cbuff_addr, 3)

        if sz != 0:
            # start and end addresses
            mask = int(2**math.ceil(math.log(sz, 2)) - 1)
            start_addr = rd - (rd & mask)
            end_addr = start_addr + sz - 1

            # available data in buffer
            amount_data = wr - rd
            if amount_data < 0:
                amount_data += sz
            amount_space = sz - amount_data - 1

            if amount_space == 0:
                return data
            else:
                n = min(len(data), amount_space)

                # end_addr is index of last item of buf
                if (wr + n - 1) >= end_addr:
                    self._core._write_dm_block(wr, data[0:(end_addr - wr + 1)])
                    written_size = (end_addr - wr + 1)
                    if written_size < n:
                        self._core._write_dm_block(start_addr, data[written_size:n])
                    wr = start_addr + n - written_size
                else:
                    self._core._write_dm_block(wr, data[0:n])
                    wr += n

                data = data[n:]
                if do_ud.lower() == 'update':
                    self._core._write_dm_block(cbuff_addr+2, [wr])

                return data
        else:
            print("The size of the cbuffer is 0.")
            return data

    def cbuffers(self):
        """
        Returns the status of cbuffers on the Kalimba.
        Considers only structures following the naming convention '*cbuffer_struc'.
        """
        if not self._core.is_connected():
            raise ka_exceptions.NotConnectedError()

        if not self._core.arch.is_bluecore():
            raise ka_exceptions.UnsupportedArchitecture("cbuffers() is not supported on non-Bluecore chips")

        self._core.sym.assert_have_symbols()

        cbuffer_name_key = "cbuffer_struc$"
        cbuffer_name_key_len = len(cbuffer_name_key)
        cbuffers = self._core.sym.varfind(cbuffer_name_key)
        cbuffers.sort()

        print(" " * 30 + "C-Buffer  Size  Read Addr  Write Addr      Start   Space   Data")

        # The results from varfind are unfortunately tuples -- one simply has to know the correct indices.
        for (struct_name, size_in_addressable_units, struct_address) in cbuffers:
            # Order of fields in the structure: size, read pointer, write pointer.
            (sz, rd, wr) = self._read_var_with_size_check(struct_address, size_in_addressable_units)

            struct_name = struct_name[:-cbuffer_name_key_len]
            print("%38s" % struct_name, end=' ')

            if sz != 0:
                # Find start address of the buffer
                mask = int(2**math.ceil(math.log(sz, 2)) - 1)
                start_addr = rd - (rd & mask)
                if start_addr != (wr - (wr & mask)):
                    print(
                        '\n\n** Warning the start address calculated from the read and '
                        'write pointers is different! **'
                    )

                amount_space = rd - wr
                if amount_space <= 0:
                    amount_space += sz
                amount_space -= 1
                amount_data = wr - rd
                if amount_data < 0:
                    amount_data += sz

                print(' %4d   0x%06x    0x%06x   0x%06x    %4d   %4d' %
                      (sz, rd, wr, start_addr, amount_space, amount_data))
            else:
                print("size=0")

    # noinspection PyPep8Naming
    def ports(self):
        """
        Displays information about each DSP port including format and amount of data/space.
        """
        # check connected to chip
        if not self._core.is_connected():
            raise ka_exceptions.NotConnectedError()

        if not self._core.arch.is_bluecore():
            raise ka_exceptions.UnsupportedArchitecture("ports() is not supported on non-Bluecore chips")

        # check symbols are loaded
        self._core.sym.assert_have_symbols()

        # get the symbols we need
        READ_OFFSET_ADDR  = self._core.sym.varfind('$cbuffer.read_port_offset_addr')
        WRITE_OFFSET_ADDR = self._core.sym.varfind('$cbuffer.write_port_offset_addr')
        READ_LIMIT_ADDR   = self._core.sym.varfind('$cbuffer.read_port_limit_addr')
        WRITE_LIMIT_ADDR  = self._core.sym.varfind('$cbuffer.write_port_limit_addr')
        READ_BUFFER_SIZE  = self._core.sym.varfind('$cbuffer.read_port_buffer_size')
        WRITE_BUFFER_SIZE = self._core.sym.varfind('$cbuffer.write_port_buffer_size')

        def read_dm(addr):
            return self._core.dm[addr]

        # get the read and write offset
        read_offset_addr  = self._read_var_with_size_check(
            READ_OFFSET_ADDR.addr,
            READ_OFFSET_ADDR.size_in_addressable_units
        )
        write_offset_addr = self._read_var_with_size_check(
            WRITE_OFFSET_ADDR.addr,
            WRITE_OFFSET_ADDR.size_in_addressable_units
        )
        read_offset       = list(map(read_dm, read_offset_addr))
        write_offset      = list(map(read_dm, write_offset_addr))

        # get the read and write limit
        read_limit_addr   = self._read_var_with_size_check(
            READ_LIMIT_ADDR.addr,
            READ_LIMIT_ADDR.size_in_addressable_units
        )
        write_limit_addr  = self._read_var_with_size_check(
            WRITE_LIMIT_ADDR.addr,
            WRITE_LIMIT_ADDR.size_in_addressable_units
        )
        read_limit        = list(map(read_dm, read_limit_addr))
        write_limit       = list(map(read_dm, write_limit_addr))

        # get the port size
        read_size         = self._read_var_with_size_check(
            READ_BUFFER_SIZE.addr,
            READ_BUFFER_SIZE.size_in_addressable_units
        )
        write_size        = self._read_var_with_size_check(
            WRITE_BUFFER_SIZE.addr,
            WRITE_BUFFER_SIZE.size_in_addressable_units
        )
        # calculate size mask (size-1) for non-zero sizes
        read_mask         = list(map(lambda s: s - (s > 0), read_size))
        write_mask        = list(map(lambda s: s - (s > 0), write_size))

        # calculate data/space in port
        read_data         = list(map(lambda l,o,m: (l - o) & m, read_limit, read_offset, read_mask))
        write_space       = list(map(lambda l,o,m: (l - o) & m - 1, write_limit, write_offset, write_mask))

        # read port configs
        read_conf_base   = self._core.sym.constfind('$READ_PORT0_CONFIG').value
        write_conf_base  = self._core.sym.constfind('$WRITE_PORT0_CONFIG').value
        read_conf        = self._read_var_with_size_check(read_conf_base,  READ_OFFSET_ADDR.size_in_addressable_units)
        write_conf       = self._read_var_with_size_check(write_conf_base, WRITE_OFFSET_ADDR.size_in_addressable_units)

        # extract data size (in octets) from config
        read_data_size   = list(map(lambda c: (c & 0x3) + 1, read_conf))
        write_space_size = list(map(lambda c: (c & 0x3) + 1, write_conf))

        # decode configs into strings
        read_conf_str  = list(
            map(
                lambda c, s:
                    ("8" if s == 1 else ("16" if s == 2 else ("24" if s == 3 else "??")))
                    + "-bit, "
                    + ("Big Endian" if (c & 0x4) else "Little Endian") + ", "
                    + ("No Sign Ext" if (c & 0x8) else "Sign Ext"   ),
                read_conf,
                read_data_size
            )
        )
        write_conf_str = list(
            map(
                lambda c, s:
                    ("8" if s == 1 else ("16" if s == 2 else ("24" if s == 3 else "??"))) + "-bit, "
                    + ("Big Endian" if (c & 0x4) else "Little Endian") + ", "
                    + ("Saturate" if (c & 0x8) else "No Saturate"),
                write_conf,
                write_space_size
            )
        )

        # print information
        print("Read ports:\n  Port    Status      Offset Address       Size(Bytes)      Data      Config")
        for i in range(len(read_offset_addr)):
            if read_offset_addr[i]:
                print(
                    "   %2i     Enabled   %6i (0x%04X)  %5i (0x%04X)    %5i      %s" %
                    (
                        i,
                        read_offset_addr[i],
                        read_offset_addr[i],
                        read_size[i],
                        read_size[i],
                        read_data[i] // read_data_size[i],
                        read_conf_str[i]
                     )
                )
            else:
                print("   %2i     Disabled" % i)

        print("Write ports:\n  Port    Status      Offset Address       Size(Bytes)     Space      Config")
        for i in range(len(write_offset_addr)):
            if write_offset_addr[i]:
                print(
                    "   %2i     Enabled   %6i (0x%04X)  %5i (0x%04X)    %5i      %s" %
                    (
                        i,
                        write_offset_addr[i],
                        write_offset_addr[i],
                        write_size[i],
                        write_size[i],
                        write_space[i] // write_space_size[i],
                        write_conf_str[i]
                    )
                )
            else:
                print("   %2i     Disabled" % i)

    def profiler(self):
        """
        Read the built-in profiler data. Requires the profiler library to be used.
        """

        if not self._core.is_connected():
            raise ka_exceptions.NotConnectedError()

        if not self._core.arch.is_bluecore():
            raise ka_exceptions.UnsupportedArchitecture("profiler() is not supported on non-Bluecore chips")

        class Task(object):
            def __init__(self, block_, addr):
                self.block    = block_
                self.addr     = addr
                self.name     = None
                self.CPU_FRAC = 0

            def tidy(self, sym):
                self.name     = sym.varfind(self.addr).name
                self.CPU_FRAC = sym.constfind("$profiler.CPU_FRACTION_FIELD").value

            def __repr__(self):
                if self.name is None:
                    raise Exception("Need to call the tidy method before using")
                return "%-50s - %2.1f %%" % (self.name, self.block[self.CPU_FRAC]/1000)

        # get the head of the list and a couple of constants
        head = self._core.sym.varfind("$profiler.last_addr").addr
        null = self._core.sym.constfind("$profiler.LAST_ENTRY").value
        size = self._core.sym.constfind("$profiler.STRUC_SIZE").value
        next_addr = self._core.sym.constfind("$profiler.NEXT_ADDR_FIELD").value

        # get the first address
        curr = self._core.dm[head]

        # read all the structures off the chip as fast as we can
        tasks = []
        while curr != null:
            block = self._core.dm[curr:(curr + size)]
            tasks.append(Task(block, curr))
            curr = block[next_addr]

        # now fill in the other bits
        for t in tasks:
            t.tidy(self._core.sym)

        # finally return
        return tasks

    class PcProfilerResults(object):
        """
        Results from pcprofiler().
        Provides pretty-printing as a table of function name against percentage of samples.
        """
        def __init__(self, total_samples, duration_secs, counts_and_funcs):
            self.total_samples = total_samples
            self.duration_secs = duration_secs
            self.counts_and_locs = counts_and_funcs

        def __repr__(self):
            results = ["PC profiling results ({0:g} total samples in {1:.2g} sec):".format(
                self.total_samples,
                self.duration_secs)
            ]
            item_with_longest_func_name = max(
                self.counts_and_locs, key=lambda count_loc : len(count_loc[1])
            )
            max_func_name_length = len(item_with_longest_func_name[1])
            for count, func in self.counts_and_locs:
                percent = 100. * count / self.total_samples
                results.append("{0:{width}}  {percent:6.3f}%  ({raw_count:g})".format(
                    func,
                    width=max_func_name_length,
                    percent=percent,
                    raw_count=count)
                )
            return "\n".join(results)

    def pcprofiler(self, duration_secs = 5.0, total_samples = None, modname = None):
        """
        Profile program execution by repeatedly reading the program counter
        in an efficient manner, and return an list of program counter against
        module / function.

        Specify either a duration for profiling, via the 'duration_secs'
        parameter, or a total number of samples, via the 'num' parameter.
        Set the unused parameter to None.

        The resulting output is a PcProfilerResults object. This contains a
        list of sorted values, most-called first:
           res = [[count0, 'name0'],
                  [count1, 'name1'],
                  ...]

        The PcProfilerResults object implements pretty-printing.

        The 'modname' parameter can be used to specify a custom function
        for resolving module / function names from PC values.
        If None, the default function Sym.modname is used; this requires
        a suitable ELF file to have been previously loaded.
        """
        if modname is None:
            modname = self._core.sym.modname

        if (duration_secs is not None and total_samples is not None) or \
                (duration_secs is None and total_samples is None):
            raise ValueError("Specify either a profiling duration or a total number of samples.")

        import time
        start = time.time()

        if total_samples is not None:
            if total_samples <= 0:
                raise ValueError("Total number of profiling samples cannot be <= 0")

            locations = self._core.other.pcprofile(total_samples)
        else:
            if duration_secs <= 0:
                raise ValueError("Profiling duration cannot be <= 0")

            # If profiling for a specified duration, choose a reasonable chunk size.
            chunk_size = 20000
            locations = []
            # Always do at least one sample.
            while True:
                locations += self._core.other.pcprofile(chunk_size)
                if (time.time() - start) >= duration_secs:
                    break

        actual_duration_secs = time.time() - start
        actual_num_samples = len(locations)

        # Sort to get contiguous blocks of func names.
        locations = sorted(map(modname, locations))

        # Transform to pairs (count, location).
        results = [[1, locations.pop(0)]]
        for lcn in locations:
            if results[-1][1] == lcn:
                results[-1][0] += 1
            else:
                results.append([1, lcn])

        # The output is most useful with the most frequent first.
        results.sort(reverse=True)

        return Lib.PcProfilerResults(actual_num_samples, actual_duration_secs, results)

    def _lazyload_matplotlib(self):
        try:
            import matplotlib.pyplot as plt
        except ImportError:
            # we couldn't load this, so can't offer the plotting options
            self._plt = None
        else:
            self._plt = plt
            
    def plotpcprofile(self,
                      duration_secs = 5.0,
                      total_samples = None,
                      modname = None,
                      plotter = None,
                      num_modules = 20,
                      interactive = True):
        """Use pcprofiler to build a program trace, then plot it.

           This routine will try and import matplotlib plotting routines. If
           they fail to do so or if an alternative plot tool is preferred, this
           can be provided via the 'plotter' argument.
        
           'num_modules' can be used to specify the maximum number of modules to plot
           results for. The results for the least frequently hit modules are merged into 'others'.
           
           'interactive' controls whether matplotlib's interactive mode is turned on. If 'interactive'
           is not set, the mode is not altered. See the matplotlib documentation for details.
        """

        # check we have a plotter to use
        if plotter is None:
            self._lazyload_matplotlib()
            plotter = self._plt
        if plotter is None:
            raise RuntimeError("No plotting method available")

        # get the trace
        results = self.pcprofiler(duration_secs=duration_secs, total_samples=total_samples, modname=modname)
        actual_num_samples = results.total_samples
        results = results.counts_and_locs
        results.sort()

        # interactive mode on
        if interactive:
            plotter.ion()
            
        # check if we have too many results
        size = len(results)
        if size > num_modules:
            # merge the smallest and drop the rest
            others  = sum(map(lambda x: x[0], results[:num_modules - 1]))
            # we want the last num_modules values, so the offset we want is negative
            results = results[1 - num_modules:]
            results.insert(0, [others, 'others'])
            size    = num_modules

        # split into values and names
        values  = list(map(lambda x: x[0], results))
        names   = list(map(lambda x: x[1], results))
        # get the max value to find the middle
        max_val = max(values)
        mid_val = max_val / 2
        # calculate the percentages
        pcnts = list(map(lambda x: "%5.1f %%" % (float(x) / actual_num_samples * 100), values))

        # plot these
        fig, ax = plotter.subplots()
        width   = 0.8
        bars    = ax.barh(list(range(size)), values, width, color='b')
        # put the words on and stuff
        ax.set_title('PC profiling (%d samples)' % actual_num_samples)
        ax.set_xlabel('Cycles')
        ax.set_yticks(list(map(lambda x: x + width / 2.0, list(range(size)))))
        ax.set_yticklabels(pcnts)
        # now put the function names in the plot
        for i in range(size):
            b     = bars[i]
            name  = names[i]
            width = b.get_width()
            # if the bar is bigger than half width, pin left, otherwise go right
            xloc  = int(0.05 * max_val)
            align = 'left'
            if width < mid_val:
                xloc  = max_val
                align = 'right'
            # Center the text vertically in the bar
            yloc = b.get_y() + b.get_height() / 2.0
            ax.text(xloc, yloc, name, horizontalalignment=align,
                    verticalalignment='center', weight='bold')
