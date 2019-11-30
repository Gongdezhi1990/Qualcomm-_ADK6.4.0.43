############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import subprocess
import re
import logging
import os

filenameRe = re.compile("^(.*\.p?o): *file format.*")
relocationRe = re.compile("^RELOCATION RECORDS FOR \[(.*)\]:")
relocationOffsetRe = re.compile("[0-9a-fA-F]{8} .* +(.*)")
sectionPMRe = re.compile(".*PM.*\?(.*)",re.IGNORECASE)
sectionDMRe = re.compile(".*\?(.*) ([0-9a-fA-F]{8}) *[0-9a-fA-F]{8} *[0-9a-fA-F]{8} *[0-9a-fA-F]{8} *2\*\*",re.IGNORECASE)
sectionDissRe = re.compile(".*PM.*\?(.*):",re.IGNORECASE)
maximOpcodeRe = re.compile(".*[0-9a-fA-F]+:.*([0-9a-fA-F]{2} [0-9a-fA-F]{2} [0-9a-fA-F]{2} [0-9a-fA-F]{2}).*",re.IGNORECASE)
sectionHeaderRe = re.compile(".* ([0-9]+) .*\?(.*)")
kalscrambleRe = re.compile(".*\?(.*) +-> +.*\?(.*)")

def runObjDump(fileName, param, exe="C:/BlueLab/main-1119/tools/bin/kobjdump.exe"):
    # if the object file is included in an archive, then run
    # kobjdump on the archive and return only the lines from the requested object file
    # Example, given:
    # /basic_passthrough/debugbin/libbasic_passthrough/basic_passthrough/debugbin/libbasic_passthrough.a(basic_passthrough.o)
    # run kobjdump on /basic_passthrough/debugbin/libbasic_passthrough/basic_passthrough/debugbin/libbasic_passthrough.a
    # and then search for "basic_passthrough.o:"
    objfile = None
    if fileName.find("@") != -1:
        objfile = fileName.split("@")[1]
        fileName = fileName.split("@")[0]
    logging.debug(str([exe, param, fileName]))
    proc = subprocess.Popen([exe, param, fileName], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    raw_lines = proc.stdout.readlines()
    err = None
    if len(raw_lines) == 0:
        err = "".join(proc.stderr.readlines())
    r = proc.wait()
    lines = []
    found = False
    for line in raw_lines:
        l = line.decode("ascii")
        if objfile:
            m = filenameRe.match(l)
            if (m is not None):
                # new object file section found, but did we already 
                # find the one we were interested in before?
                if found:
                    # if we did, then do not return more text
                    break
                else:
                    # is the object file we are interested in?
                    if (objfile in l):
                        found = True
            # if object file section found, keep adding lines
            if found:
                lines.append(l)
        # if we aren't looking for a specific object file, then return all contents
        # if it's an archive, the caller will have to process the multiple object files
        else:
            lines.append(l)
    return (lines, r, err)

def generateFileNameObj(filename, obj_file):
    isArchive = os.path.splitext(filename)[1] == ".a"
    if isArchive:
        return filename + "@" + obj_file
    else:
        return filename

def parseRelocObjFile(lines, prog, var, const, filename, mappings):
    class Reference:
        TYPE_BRANCH, TYPE_SE = list(range(2))

        def __init__(self, **kwargs):
            kwargs['type'] = Reference.TYPE_BRANCH if kwargs['type'].endswith('branch') else Reference.TYPE_SE
            self.__dict__.update(kwargs)
        def __setattr__(self, k, v):
            raise Exception("Cannot set value")
        def __repr__(self):
            return "%08X %10s %s[%d]" % (self.pos, self.type, self.name, self.off)
        def updatePos(self, pos):
            """Because the reference information is taken from the object files,
            i.e. before the linker has run, we cannot take relative/absolute
            references into account. As such all references are assumed to be
            absolute, i.e. require a prefix. However, if the linker is run with
            --relax, some may collapse to realtive references and no longer
            require a prefix. This changes the size of the module and hence all
            if its references.

            Therefore we need to be able to update the position variable."""
            self.__dict__['pos'] = pos
    class Symbol(list):
        def __init__(self, name, region):
            self.name   = name
            self.region = region
        def append(self, **kwargs):
            list.append(self, Reference(**kwargs))
    
    def twos_comp(val, bits):
        if (val & (1 << (bits - 1))) != 0:
            val = val - (1 << bits)
        return val
    
    if type(lines) != list:
        raise Exception("parseRelocObjFile requires a list of strings")

    # initialise the dictionary of symbols
    currSym = None
    i       = 0
    filename_obj = ""
    while i < len(lines):
        l  = lines[i].rstrip()
        logging.debug(l)
        i += 1
        if l == "":
            continue
        m = filenameRe.match(l)
        if m is not None:
            # form the object path name found in the archive
            filename_obj = generateFileNameObj(filename, m.group(1))
            logging.debug("Parsing object file: {}".format(filename_obj))
        else:
            # is this is a start of variable
            m = relocationRe.match(l)
            if m is not None:
                if filename_obj == "":
                    raise Exception("Couldn't find what object file this relocation record belongs to: {}".format(l))
                bits = m.group(1).split("?")
                if len(bits) != 2:
                    # we need to keep moving on
                    currSym = None
                    continue

                (region, name)  = bits
                
                # Add the file name to symbols that don't start with $ to be able to identify them
                region = region.lower()
                
                # Encrypted input sections names for data are trimmed, 
                # find full name from mappings if present
                originalName = ""
                for v in mappings:
                    if v.startswith(name):
                        logging.debug("parseRelocObjFile:{v1} corresponds to {v2}".format(v1=name, v2=v))
                        name = v
                        originalName = mappings[v]
                        logging.debug("originalName is: " + originalName)
                        break
                            
                # If it's an encrypted file, first we need to find out whether it's a global
                # symbol or not by looking at the translation table (originalName)
                if not name.startswith("$") and not originalName.startswith("$"):
                        # Use % as a separator between file name and variable name
                        name = filename_obj + "%" + name


                if name in var or name in const:
                    raise Exception("%s: declared twice" % name)
                if name in prog:
                    logging.debug(name + " declared more than once. This could happen when using inline functions. Ignoring this occurence")
                else:
                    currSym         = Symbol(name, region)
                    logging.debug(str(name) + " is in region " + str(region))
                    if region.endswith('dmconst') or region.endswith('const') or region.endswith('const16') or region.endswith('dmconst_windowed16'):
                        logging.debug("Storing in const...")
                        const[name] = currSym
                    elif region.endswith('pm') or region.endswith('pm_flash') or region.endswith("pm_ram") or region.endswith("pm_keep"):
                        logging.debug("Storing in prog...")
                        prog[name]  = currSym
                    elif region.endswith('dm') or region.endswith('dm1') or region.endswith('dm2'):
                        logging.debug("Storing in var...")
                        var[name]   = currSym
                    else:
                        raise Exception("Couldn't find a region for {}".format(l))
                # bin the next line
                i += 1
                continue
            # do we have a current symbol
            if currSym is None:
                continue

            # this is a line with offset information
            bits = l.split()
            logging.debug(bits)
            if len(bits) != 3:
                errString  = "Bad table entry for symbol: %s\n  %s" % (currSym.name, l)
                raise Exception(errString)

            # parse the entry
            pos     = int(bits[0], 16)
            symType = bits[1]
            sym     = bits[2]
            symOff = 0
            if sym.find("=+") != -1:
                # It's a scrambled symbol with an offset
                symName = sym[:sym.find("=+")+1]
                symOff = int(sym[sym.find("=+")+2:],16)
            elif sym.endswith("="):
                # It's a scrambled symbol ending with "="
                symName = sym
            else:
                symsplit = sym.split("+0x")
                # If it's a scrambled symbol we might have multiple occurrences
                # of +0x, so always keep the latest which will be an offset
                if len(sym) > 1:
                    try:
                        symOff = int(symsplit[-1], 16)
                        symsplit = symsplit[:-1]
                    except:
                        symOff = 0
                    symName = "+0x".join(symsplit)
                else:
                    symName = symsplit[0]
                    symOff = 0
            # symOff is in 32-bit two's complement, undo it
            symOff = twos_comp(symOff,32)
            # Static variables have dmzi? prefix added to the name, remove it

            if symName.lower().startswith("dmzi?"):
                symName = symName[len("dmzi?"):]
            # Const variables have const? prefix added to the name, remove it
            elif symName.lower().startswith("const?"):
                symName = symName[len("const?"):]
            # Some variables are defined as dmconst, remove that as well
            elif symName.lower().startswith("dmconst?"):
                symName = symName[len("dmconst?"):]
            elif symName.lower().startswith("dmconst16?"):
                symName = symName[len("dmconst16?"):]
            elif symName.lower().startswith("dmconst24?"):
                symName = symName[len("dmconst24?"):]
            elif symName.lower().startswith("dm?"):
                symName = symName[len("dm?"):]
            elif symName.lower().startswith("dm1_zi?"):
                symName = symName[len("dm1_zi?"):]
            elif symName.lower().startswith("dm2_zi?"):
                symName = symName[len("dm2_zi?"):]
            elif symName.lower().startswith("dm1?"):
                symName = symName[len("dm1?"):]
            elif symName.lower().startswith("dm2?"):
                symName = symName[len("dm2?"):]
            # Ignore DEBUG_TRACE_STRINGS
            elif symName.startswith("DEBUG_TRACE_STRINGS?"):
                continue
            elif symName.find("?") != -1:
                raise Exception("Unrecognised section for variable: " + str(symName) + " in file " + str(filename_obj))

            # Assign a unique name to anonymous variables by prefixing the object file to the variable name
            if not symName.startswith("$"):
                # But if it's an encrypted file, first we need to find out whether it's a global
                # symbol or not by looking at the translation table
                originalName = ""
                for v in mappings:
                    if v.startswith(symName):
                        # Expand the name in case it's not expanded already
                        symName = v
                        originalName = mappings[v]
                        break
                # Use % as a separator between file name and variable name
                if not originalName.startswith("$"):
                    symName = filename_obj + "%" + symName
                    logging.debug("name modified_: " + symName)
                else:
                    logging.debug("name NOT modified_: " + symName)

            currSym.append(pos=pos, type=symType, name=symName, off=symOff)
    logging.debug("parse_const:" + str(const))
    logging.debug("parse_prog:" + str(prog))
    logging.debug("parse_var:" + str(var))

def parseHeadersFile(lines, func, var_sizes, filename, mappings):
    i       = 0
    procFunc = False
    funcname = ""
    while i < len(lines):
        l  = lines[i].rstrip()
        i += 1
        if l == "":
            continue
        if (procFunc == True):
            if (l.find("MINIM")!=-1):
                func[funcname] = ["MINIM"]
            procFunc=False
        else:
            m = filenameRe.match(l)
            if m is not None:
                # form the object path name found in the archive
                filename_obj = generateFileNameObj(filename, m.group(1))
            # is this is a start of a PM section?
            m = sectionPMRe.match(l)
            if m is not None:
                bits = m.group(1).split("?")
                funcname = bits[0].split(" ")[0]
                size = int(bits[0].split(" ")[1],16)
                if size != 0:
                    # Add the object file name to the symbols that don't start with $ to be able to identify them
                    if not funcname.startswith("$"):
                        originalName = ""
                        if funcname in mappings:
                            originalName = mappings[funcname]
                        if not originalName.startswith("$"):
                            # Use % as a separator between file name and variable name
                            funcname = filename_obj + "%" + funcname
                    if funcname in func:
                        logging.debug(funcname + " declared more than once, ignoring this ocurrence")
                    else:
                        # Got a function name, next line comes with MINIM/MAXIM information, but default is MAXIM
                        func[funcname] = ["MAXIM"]
                        procFunc = True
                else:
                    logging.debug(funcname + " has size 0")
            else:
                m = sectionDMRe.match(l)
                if m is not None:
                    logging.debug(l)
                    varname = m.group(1).strip()
                    size = int(m.group(2),16)
                    if size != 0:
                        if not varname.startswith("$"):
                            originalName = ""
                            # Encrypted input sections names are trimmed, 
                            # find full name from mappings if present
                            for var in mappings:
                                if var.startswith(varname):
                                    originalName = mappings[var]
                                    varname = var
                                    logging.debug("{v1} corresponds to {v2} and has a size {s}".format(v1=varname, v2=var, s=size))
                                    break
                            # Only add if found in mappings
                            if originalName != "":
                                if not originalName.startswith("$"):
                                    # Use % as a separator between file name and variable name
                                    varname = filename_obj + "%" + varname
                                var_sizes[varname] = int(m.group(2),16)
                        elif varname.startswith("$___"):
                            # library symbols starting with $___ could be missed by kalelfreader
                            logging.debug("{v1} with size {s} noted".format(v1=varname, s=size))
                            var_sizes[varname] = int(m.group(2),16)

def parseDisObjFile(lines, func, filename, mappings, func_export, kdc_maker):
    """Parses the disassembly of the object file and stores the opcodes before they are passed to the linker
    so that relocation information matches exactly the instructions extracted from here. This will later be used
    when fixing up the code offsets in MiniM"""
    i       = 0
    procFunc = False
    funcname = ""
    export = False
    logging.debug("func here is:" + str(func))
    while i < len(lines):
        l  = lines[i].rstrip()
        i += 1
        if l == "":
            continue
        #logging.debug(l)
        if (procFunc == True):
            m = maximOpcodeRe.match(l)
            if m is not None:
                opcode_bits = m.group(1).split(" ")
                # Rearrange endiannes
                opcode_bits = opcode_bits[::-1]
                opcode=""
                for num in opcode_bits:
                    opcode = opcode + num
                if kdc_maker.isPrefix(int(opcode,16)):
                    opcodes_list.append("P")
                else:
                    opcodes_list.append("I")
            else:
                m = sectionDissRe.match(l)
                if m is not None:
                    procFunc=False
        m = filenameRe.match(l)
        if m is not None:
            # form the object path name found in the archive
            filename_obj = generateFileNameObj(filename, m.group(1))
            logging.debug("Parsing object file: {}".format(filename_obj))
        # is this is a start of a PM section?
        m = sectionDissRe.match(l)
        if m is not None:
            bits = m.group(1).split("?")
            funcname = bits[0].split(" ")[0]
            logging.debug("Processing funcname: " + str(funcname))
            originalName = ""
            for var in mappings:
                if mappings[var] == funcname:
                    logging.debug("Found! {fname} is now: {mfname}".format(fname=funcname,mfname=var))
                    originalName = funcname
                    funcname = var
                    export = True
                    break

            # Add the object file name to the symbols that don't start with $ to be able to identify them
            if not funcname.startswith("$") and not originalName.startswith("$"):
                # Use % as a separator between file name and variable name
                funcname = filename_obj + "%" + funcname
            if funcname not in func:
                raise Exception("%s: wasn't found on headers list" % funcname)
            else:
                # Create an array. Instruction set info, list of opcodes
                opcodes_list = []
                func[funcname].append(opcodes_list)
                if export:
                    func_export[funcname] = func[funcname]
                    export = False
            procFunc = True

def insertMapping(mappings, input_name, value):
    # Trim the input strings for protection in case
    # special characters are added at the end of strings
    input_name = input_name.strip()
    value = value.strip()
    name_to_use = input_name
    name_to_pop = ""
    for name in mappings:
        # Nothing to do if entry already exists
        if name == input_name:
            return
        # Otherwise always use the longest version
        if name.startswith(input_name):
            name_to_use = name
            name_to_pop = input_name
            break
        elif input_name.startswith(name):
            name_to_use = input_name
            name_to_pop = name
            break
    mappings[name_to_use] = value
    if name_to_pop in mappings:
        mappings.pop(name_to_pop)

def censorMappings(mappings, mappings_to_export):
    for name in mappings:
        if mappings[name].startswith("$"):
            mappings_to_export[name] = "$"
        elif mappings[name].startswith("L_"):
            mappings_to_export[name] = "L_"
        # C anonymous variables
        elif mappings[name].startswith("A_"):
            mappings_to_export[name] = "A_"
        elif mappings[name].startswith("Lc"):
            mappings_to_export[name] = "Lc"
        # C static functions
        elif mappings[name].startswith("_"):
            mappings_to_export[name] = "_"
        elif mappings[name].startswith("rwgroup"):
            mappings_to_export[name] = "rwgroup"
        elif mappings[name].startswith("rogroup"):
            mappings_to_export[name] = "rogroup"
        # KCC might produce labels with this name (without the expected 'Lc' prefix)
        # Add a special case here while this is not fixed
        elif mappings[name].startswith("loop"):
            mappings_to_export[name] = name
        else:
            raise Exception("Unexpected symbol name: {}".format(mappings[name]))

def parseMappingsReloc(mappings, lines1, lines2, f):
    cur_obj_file = ""
    for l1, l2 in zip(lines1, lines2):
        if l1 != l2 or l1.find("file format") != -1:
            m1 = filenameRe.match(l1)
            if m1 != None:
                cur_obj_file = m1.group(1)
            else:
                m1 = relocationRe.match(l1)
                if m1 is not None:
                    if cur_obj_file == "":
                        raise Exception("Error. No object file name found")
                    m2 = relocationRe.match(l2)
                    name_left = m2.group(1).split("?")[1]
                    name_right = m1.group(1).split("?")[1]
                    insertMapping(mappings, name_right, name_left)
                else:
                    m1 = relocationOffsetRe.match(l1)
                    if m1 is not None:
                        if cur_obj_file == "":
                            raise Exception("Error. No object file name found")
                        m2 = relocationOffsetRe.match(l2)
                        name_left = m2.group(1)
                        name_right = m1.group(1)
                        if name_left.find("?") != -1:
                            name_left = name_left.split("?")[1]
                            name_right = name_right.split("?")[1]
                        # do we have an offset
                        if name_left[-10:].startswith("0x"):
                            # remove offset
                            name_left = name_left[:-11]
                            name_right = name_right[:-11]
                        # For some reason Windows version of kobjdump generates a carriage return
                        # on captured names, that prevents variables from getting recognised later on
                        # so trim the strings here
                        name_right = name_right.strip()
                        name_left = name_left.strip()
                        insertMapping(mappings, name_right, name_left)

def parseMappingsHeaders(mappings, lines1, lines2):
    i1 = 0
    idx1 = -1
    for l2 in lines2:
        l1 = lines1[i1]
        m2 = sectionHeaderRe.match(l2)
        if m2 is not None:
            m1 = None
            while m1 == None:
                m1 = sectionHeaderRe.match(l1)
                # Advance l1 until we match the index
                i1 = i1 + 1
                l1 = lines1[i1]
            name_left = m2.group(2).split()[0]
            name_right = m1.group(2).split()[0]
            insertMapping(mappings, name_right, name_left)

def parseMappingsKalscramble(mappings, lines):
    for l in lines:
        m = kalscrambleRe.match(l)
        if m != None:
            name_left = m.group(1).strip()
            name_right = m.group(2).strip()
            logging.debug("kalscramble:" + repr(name_left))
            logging.debug("kalscramble:" + repr(name_right))
            if name_right.find("->") != -1:
                name_right = name_right.split("->")[1].strip()
            insertMapping(mappings, name_right, name_left)

def getVariables(files, objDumpPath, mappings, func_import, kdc_maker):
    """A simple function that takes a list of .o files and parses them for the
    variables they contain, along with their dependencies."""

    prog  = {}
    var   = {}
    const = {}
    func = {}
    func_export = {}
    var_sizes = {}
    # Get relocation info
    for f in files:
        if os.path.exists(f):
            (lines, r, msg) = runObjDump(f, "-r", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f, msg))
            logging.debug("Fetching relocation information from file: " + str(f))
            parseRelocObjFile(lines, prog, var, const, f, mappings)
    # Get Maxim/Minim info from each function
    for f in files:
        if os.path.exists(f):
            (lines, r, msg) = runObjDump(f, "-h", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f, msg))
            logging.debug("Fetching functions from file: " + str(f))
            parseHeadersFile(lines, func, var_sizes, f, mappings)
    # Get Maxim code from object files to allow relocation offset fix-ups in MiniM later
    for f in files:
        if os.path.exists(f):
            if os.path.isfile(f + ".xxx"):
                (lines, r, msg) = runObjDump(f + ".xxx", "-d", exe=objDumpPath)
            else:
                (lines, r, msg) = runObjDump(f, "-d", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f, msg))
            logging.debug("Fetching disassembly from file: " + str(f))
            func_export[f] = {}
            parseDisObjFile(lines, func, f, mappings, func_export[f], kdc_maker)
            logging.debug("func_import:" + str(func_import))
            logging.debug("func::" + str(func))
            # Merge information coming from func_import with func
            if f in func_import:
                for imported_name in func_import[f]:
                    # Note that imported module names prefixed with the 
                    # object name will have the path information stripped
                    # so try to match only the end of the strings
                    found = False
                    for mod_name in func:
                        if mod_name.endswith(imported_name):
                            func[mod_name] = func_import[f][imported_name]
                            logging.debug("{f1} found in func {f2}".format(f1=imported_name,f2=mod_name))
                            found = True
                            break
                    if not found:
                        raise Exception("Couldn't find {} in func".format(imported_name))
            logging.debug("func:::" + str(func))
    # Sanitize func_export (remove keys without values and paths to object files)
    #func_export = dict((k, v) for k, v in func_export.iteritems() if v)
    func_export = dict((k, v) for k, v in func_export.items() if v)
    if len(func_export) > 1:
        raise Exception("Error. Only one private library can be built at once")
    for func_dict in func_export:
        len_before = len(func_export[func_dict])
        func_export[func_dict] = dict((os.path.basename(k.split("%")[0]) + "%" + k.split("%")[1] if k.find("%") != -1 else k, v) for k, v in func_export[func_dict].items())
        if len(func_export[func_dict]) != len_before:
            raise Exception("Error. func_export dictionary lost some symbols when trimming path")

    return (prog, var, const, func, func_export, var_sizes)

def getMappings(files, objDumpPath, mappings, mappings_to_export):
    for f in files:
        if os.path.isfile(f + ".xxx"):
            (linesr1, r, msg) = runObjDump(f, "-r", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f, msg))
            (linesr2, r, msg) = runObjDump(f + ".xxx", "-r", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f + ".xxx", msg))
            (linesh1, r, msg) = runObjDump(f, "-h", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f, msg))
            (linesh2, r, msg) = runObjDump(f + ".xxx", "-h", exe=objDumpPath)
            if r != 0:
                raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, f + ".xxx", msg))
            # Get translation from input sections and from symbols found in relocation sections
            logging.debug("parsing mappings for reloc {}".format(f))
            parseMappingsReloc(mappings, linesr1, linesr2, f)
            logging.debug("parsing mappings for headers {}".format(f))
            parseMappingsHeaders(mappings, linesh1, linesh2)
            logging.debug("parsing mappings for kalscramble translation {}".format(f))
            f = open(f + ".translation")
            lines = f.readlines()
            f.close()
            parseMappingsKalscramble(mappings, lines)
    censorMappings(mappings, mappings_to_export)

def getAnonymousVariableSize(file, anonymVarName, objDumpPath):
    # if it's a private file, get the .a using obj_lib_map
    #if os.path.splitext(file)[1] == ".po":
    #    file = obj_lib_map[os.path.basename(file)]
    (lines, r, msg) = runObjDump(file, "-h", exe=objDumpPath)
    if r != 0:
        raise Exception("Error returned: %d, file: %s\nMessage:\n  %s" % (r, file, msg))
    anonymousRe = re.compile(".*\?" + anonymVarName + "\s*([0-9A-Fa-f]{8})",re.IGNORECASE)

    for l in lines:
        if l == "":
            continue
        m = anonymousRe.match(l)
        if m is not None:
            return int(m.group(1), 16)
    raise Exception("Error. Anonymous variable " + str(anonymVarName) + " could not be found in " + str(file))

if __name__ == "__main__":
    import sys
    import getopt

    options, objFiles = getopt.gnu_getopt(sys.argv[1:], "d:")
    objDumpPath = None
    for (switch, value) in options:
        if switch == "-d":
            objDumpPath = value

    if objDumpPath == None:
        raise Exception("No path to kobjdump specified. Usage: python parseObjFile.py -d kobjdump_path/kobjdump objfile.o...")

    (prog, var, const) = getVariables(objFiles, objDumpPath)

    res = ""

    for (name, syms) in (("Program", prog), ("Variables", var), ("Constant", const)):
        res += "Region: %s\n" % name
        keys = list(syms.keys())
        keys.sort()

        regions = list( set( [syms[s].region for s in syms] ) )
        res += ", ".join(regions) + "\n"

        for k in keys:
            res += "  %s[%s]\n" % (syms[k].name, syms[k].region)
            for s in syms[k]:
                res += "    %08X %-15s %s[%d]\n" % (s.pos, s.type, s.name, s.off)


    print(res)
