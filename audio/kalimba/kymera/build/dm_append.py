############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# dm_append.py
#
# Appends initialised data values in DM and PM RAM to ROM file contents
#

import os
import re
import sys

# Some constants
MAX_ROM_SIZE = 0x200000     # 1M of 32-bit words, but we represent the ROM In 16-bit words
MAX_DM_SIZE  = 0x10000      # Assume we won't ever have more than 64kW of DM RAM
MAX_PM_SIZE  = 0x10000      # Assume we won't ever have more than 64kB of PM RAM

MAX_KALARCH4_DM_SIZE = 0xC0000 # Bigger DM for KAL ARCH 4
MAX_KALARCH4_PM_SIZE = 0x30000 # Bigger PM for KAL ARCH 4
KALARCH4_PM_BASE_ADDR = 0x04000000 # Base address for PM RAM in KAL ARCH 4

AMBER_NVMEM_DM24_OFFSET = 0xC00000
AMBER_NVMEM_DM16_OFFSET = 0xA00000
AMBER_USED_DM_SIZE = 0x3FFC
AMBER_KALARCH = 5           # Default architecture if none is specified in command line options

NUM_TAGS = 5                # Simple implementation for now, assumes data is ZI / init / ZI / PM

TAG_END = 0
TAG_COPY = 1
TAG_ZERO = 2
TAG_COPY_PM = 3

# Globals
xuv_rom = [None] * MAX_ROM_SIZE
dm_ram =  [0] # wait for options before initialising DM array
pm_ram =  [0] # wait for options before initialising PM array

have_pm_ram = False

def parse_options():
    dm_offset = AMBER_NVMEM_DM24_OFFSET
    dm_size = AMBER_USED_DM_SIZE
    kalarch = AMBER_KALARCH
    mergefile = None
    for x in range(len(sys.argv))[2:]:
        if sys.argv[x] == "-dmsize":
            dm_size = int(sys.argv[x+1].lstrip('0x'), 16)
        if sys.argv[x] == "-offset":
            dm_offset = int(sys.argv[x+1].lstrip('0x'), 16)
        if sys.argv[x] == "-kalarch":
            kalarch = int(sys.argv[x+1].lstrip('0x'), 16)
        if sys.argv[x] == "-merge":
            # Merge in a raw .xuv fragment and check for overlaps.
            # This was added to include some temporary test content in the
            # Crescendo r00 ROM. It's not expected to have a long-term
            # future. If it's in the way, get rid of it.
            mergefile = sys.argv[x+1]

    return dm_size, dm_offset, kalarch, mergefile

def read_rom_content(rom_file):
    max_addr = 0

    for line in rom_file:
        saddr, sdata = line.split()
        addr = int(saddr.strip('@'), 16)
        data = int(sdata, 16)

        if addr > max_addr:
            max_addr = addr

        # Blindly update previous content (shouldn't be anything there)
        if xuv_rom[addr/2] is None:
            xuv_rom[addr/2] = 0
        if addr & 1:
            xuv_rom[addr/2] = (xuv_rom[addr/2] & 0x00FF) | (data<<8)
        else:
            xuv_rom[addr/2] = (xuv_rom[addr/2] & 0xFF00) | (data)

    return max_addr/2


def read_dm_content(dm_file,kalarch):
    global dm_ram
    last_used = 0;
    # Initialise DM array and first used based on architecture
    if(kalarch == 4):
        dm_ram =  [0] * MAX_KALARCH4_DM_SIZE
        first_used = MAX_KALARCH4_DM_SIZE-1;
    else:
        dm_ram =  [0] * MAX_DM_SIZE
        first_used = MAX_DM_SIZE-1;

    for line in dm_file:
        saddr, sdata = line.split()
        addr = int(saddr.strip('@'), 16)
        data = int(sdata, 16)
        dm_ram[addr] = data
        if data != 0:
            if addr > last_used:
                last_used = addr
            if addr < first_used:
                first_used = addr;

    return first_used, last_used


def read_pm_content(pm_file,kalarch):
    global pm_ram
    if(kalarch == 4):
        pm_ram =  [0] * MAX_KALARCH4_PM_SIZE
        first_used = MAX_KALARCH4_PM_SIZE-1;
    else:
        pm_ram =  [0] * MAX_PM_SIZE
        first_used = MAX_PM_SIZE-1;
    last_used = 0;

    for line in pm_file:
        saddr, sdata = line.split()
        addr = int(saddr.strip('@'), 16)
        data = int(sdata, 16)
        if(kalarch == 4):
            # KAL ARCH4 has PM RAM starting at a different base than 0.
            addr = addr-KALARCH4_PM_BASE_ADDR
        pm_ram[addr] = data
        if data != 0:
            if addr > last_used:
                last_used = addr
            if addr < first_used:
                first_used = addr;

    # PM range should be 16-bit aligned
    # Round down first to an even address
    first_used &= ~1
    # Round up last to an odd address
    last_used |= 1

    return first_used, last_used


"""Merge a hardcoded .xuv fragment into the image we've built
   Complain if there are any overlaps"""
def merge_xuv_content(merge_xuv_file):
    highest_addr = None

    for rawline in merge_xuv_file:
        # Real .xuv files don't always allow comments, but we'll allow them
        # in our input for readability (and strip them)
        line = rawline.split('#')[0].strip()
        if line == '':
            # ignore blank or comment-only lines
            continue

        saddr, sdata = line.split()
        addr = int(saddr.strip('@'), 16)
        data = int(sdata, 16)

        # FIXME: awful workaround for B-171644, which can lead to the output
        # from this script being reused as input: if we detect a clash but
        # the data that's already there is the same as what we want to put
        # there, silently let it through.
        if xuv_rom[addr] == data:
            pass
        elif xuv_rom[addr] is None:
            xuv_rom[addr] = data
        else:
            print "-merge file attempts to redefine content at xuv address " + hex(addr)
            sys.exit(3)

        if highest_addr is None:
            highest_addr = addr
        else:
            highest_addr = max(highest_addr, addr)

    return highest_addr

# Append data to existing ROM image content
# Source data is in 24-bit words
# Return value is size appended in 16-bit words
def xuv_append_dm(xuv_address, offset, length):
    for count in range(length):
        # Blindly overwrite xuv_rom[] (shouldn't be anything there)
        xuv_rom[xuv_address+2*count] = (dm_ram[count + offset] & 0xFFFF)
        xuv_rom[xuv_address+2*count+1] = (dm_ram[count + offset] >> 16)

    return 2*length

# Copy contents of DM or PM file to the end of the xuv
def xuv_force_append(xuv_address, file_name):
    prev_addr = 0
    line_count = 0
    xuv_index = 0
    file = open(file_name)

    for line in file:
        saddr, sdata = line.split()
        # We actually dont care much for the address.
        addr = int(saddr.strip('@'), 16)
        data = int(sdata, 16)

        xuv_index = xuv_address+(line_count/2)

        # Blindly update previous content (shouldn't be anything there)
        if xuv_rom[xuv_index] is None:
            xuv_rom[xuv_index] = 0
        if addr & 1:
            xuv_rom[xuv_index] = (xuv_rom[xuv_index] & 0x00FF) | (data<<8)
        else:
            xuv_rom[xuv_index] = (xuv_rom[xuv_index] & 0xFF00) | (data)
        # finally increase the line count in the loop
        line_count += 1

    file.close()
    return (line_count/2)

# Append tag data to existing ROM image content
# Return value is size appended in 16-bit words
def xuv_append_tags(xuv_address, tags):
    count = 0
    for tag in tags:
        # Blindly overwrite xuv_rom[] (shouldn't be anything there)
        xuv_rom[xuv_address+8*count] = tag['tag_type']
        xuv_rom[xuv_address+8*count+1] = 0
        xuv_rom[xuv_address+8*count+2] = tag['dest'] & 0xFFFF
        xuv_rom[xuv_address+8*count+3] = tag['dest'] >> 16
        xuv_rom[xuv_address+8*count+4] = tag['source'] & 0xFFFF
        xuv_rom[xuv_address+8*count+5] = tag['source'] >> 16
        xuv_rom[xuv_address+8*count+6] = tag['len'] & 0xFFFF
        xuv_rom[xuv_address+8*count+7] = tag['len'] >> 16
        count += 1

    return 8*len(tags)


# Append data to existing ROM image content
# Source data is in octets
# Return value is size appended in 16-bit words
def xuv_append_pm(xuv_address, offset, length):
    if length & 1:
        print "Error: PM address range must be an even number of octets"
        sys.exit(1)

    for count in range(length):
        # Blindly update previous content (shouldn't be anything there)
        if xuv_rom[xuv_address+count/2] is None:
            xuv_rom[xuv_address+count/2] = 0
        if count & 1:
            xuv_rom[xuv_address+count/2] |= (pm_ram[count + offset] << 8)
        else:
            xuv_rom[xuv_address+count/2] |= pm_ram[count + offset]
    return length/2


def write_new_xuv(xuvfile, size, kalarch):
    for addr in range(size):
        content = xuv_rom[addr]
        if content is None:
            # it's OK to emit xuv files with holes
            continue
        # Use different format according to architecture. KAL ARCH 4 has 32-bit addresses
        if(kalarch == 4):
            xuvfile.write("@{0:0>8X} {1:0>4X}\n".format(addr, content))
        else:
            xuvfile.write("@{0:0>6X} {1:0>4X}\n".format(addr, content))

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "dm_append filename [-dmsize <size>] [-offset <offset>] [-kalarch 4] [-merge extra.xuv]\n"
        sys.exit(2)

    base_file_name = sys.argv[1]
    dm_size, dm_offset, kalarch, mergefile = parse_options()

    rom_file_name = base_file_name + ".rom"
    dm_file_name = base_file_name + ".dm"
    pm_file_name = base_file_name + ".pm"
    xuv_file_name = base_file_name + ".xuv"

    rom_file = open(rom_file_name)
    dm_file = open(dm_file_name)
    pm_file = open(pm_file_name)

    # First read the base ROM content from the .rom file
    last_xuv_address = read_rom_content(rom_file);

    # Make sure everything we add is 32-bit aligned
    # Tag data starts at (last_xuv_address+1) in 16-bit addresses
    # so last_xuv_address should be odd.
    if (last_xuv_address & 1) == 0:
        last_xuv_address += 1

    # Now read the DM and PM files
    first_dm_address, last_dm_address = read_dm_content(dm_file,kalarch)
    first_pm_address, last_pm_address = read_pm_content(pm_file,kalarch)

    if last_pm_address >= first_pm_address:
        have_pm_ram = True

    rom_file.close()
    dm_file.close()
    pm_file.close()

    if(kalarch != 4):
        dm_rom_start = last_xuv_address + 8*NUM_TAGS + 1
        pm_rom_start = dm_rom_start + 2*(last_dm_address - first_dm_address + 1)

        # Create the description table.
        # Hard-coded for now with ZI / init / ZI

        dm_tags = []

        # Zero from bottom of DM RAM up to first initialised address */
        dm_tags.append(dict(tag_type = TAG_ZERO, len = first_dm_address, dest = 0, source = 0))

        # Copy block with initialised data
        dm_tags.append(dict(tag_type = TAG_COPY, len = last_dm_address - first_dm_address + 1, dest = first_dm_address, source = dm_rom_start/2 + dm_offset))

        # Zero from top of initialised data up to last used address
        dm_tags.append(dict(tag_type = TAG_ZERO, len = dm_size - (last_dm_address+1), dest = last_dm_address+1, source = 0))

        if have_pm_ram:
            # Copy PM RAM data
            dm_tags.append(dict(tag_type = TAG_COPY_PM, len = (last_pm_address - first_pm_address + 4) & (~3), dest = first_pm_address, source = pm_rom_start + AMBER_NVMEM_DM16_OFFSET))
        else:
            # Termination - all zeros
            dm_tags.append(dict(tag_type = TAG_END, len = 0, dest = 0, source = 0))

        # Termination - all zeros
        dm_tags.append(dict(tag_type = TAG_END, len = 0, dest = 0, source = 0))

        # Append the table and the data to the existing content
        last_xuv_address += xuv_append_tags(last_xuv_address+1, dm_tags);
        last_xuv_address += xuv_append_dm(dm_rom_start, first_dm_address, last_dm_address - first_dm_address + 1);
        if have_pm_ram:
            last_xuv_address += xuv_append_pm(pm_rom_start, first_pm_address, last_pm_address - first_pm_address + 1);
    else:
        # On 32-bit Kalimbas (such as Crescendo) we can let the linker script do a lot more of the work,
        # so we don't have to write out a little script here ('tags') for crt0 to follow.
        # Instead we just do the heavy lifting of copying DM and PM initialisers to where the already-linked
        # code expects to find them. (Zero initalisation is handled purely in link script / crt0.)
        # (FIXME: we're only doing this here because kalelf2mem didn't do what we wanted if we did it in the
        # linker script. Now that we're abandoning kalelf2mem per <http://ukbugdb/B-169702#h1226267>, can we
        # move this to the linker script, and turn this script into a pure ELF-to-xuv converter?)
        if have_pm_ram:
            last_xuv_address += xuv_force_append(last_xuv_address+1, pm_file_name);
        last_xuv_address += xuv_force_append(last_xuv_address+1, dm_file_name);

    # Add any hardcoded extra xuv content
    if mergefile:
        merge_xuv_file = open(mergefile)
        last_xuv_address = max(last_xuv_address, merge_xuv_content(merge_xuv_file))

    # Finally write out the new .xuv file
    xuvfile = open(xuv_file_name,"w");
    write_new_xuv(xuvfile, last_xuv_address+1, kalarch);
    xuvfile.close()
