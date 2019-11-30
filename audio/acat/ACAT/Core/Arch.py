############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2019 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module holding the chip architecture related data.
"""
# pylint: disable=invalid-name
# All the name are kept lower case because this is a special module which only
# holds some data.

# There are places where we magically need to know, say, whether an address
# is in the DM region, BAC, MMR, or whatever. This module is used to
# perform the lookup.
# Like CoreUtils and CoreTypes, all the variables and functions are
# globally-scoped.  It's easier than putting them all into a singleton
# class or passing an object around between all the analyses.

# Information about the chip we're analysing.
# This is provided so that any component can query whether the chip
# is (e.g.) KAL_ARCH_3 or KAL_ARCH_5.
# (Both Chipdata and Debuginfo probably know this already, but
# this provides a common format).
# Set by a call to chip_select.
# * kal_arch is an integer, e.g. 3, 4, 5
# * chip_arch is a string containing e.g. "Bluecore", "Hydra", "KAS"
# * chip_id is either an integer representing the chip id, e.g. 28
#   (see http://wiki/ChipVersionIDs), or a string representing the
#   internal name of the chip (e.g. "amber")
kal_arch = 0
chip_arch = None
chip_id = 0
chip_cpu_speed_mhz = 0
addr_per_word = 0
chip_revision = 0
dm_ram_aliased = None

# There's *probably* enough information in the elf 'section hdrs' to
# distinguish the different PM and DM regions, so we should really use
# that. There are two problems with that though:
#  - The section information would only be available
#    to components with access to debuginfo.
#    This rules out Chipdata (specifically, LiveSpi),
#    which sometimes wants to know whether
#    (e.g.) a PM address is in RAM.
#  - The names of sections/regions are not guaranteed
#    to remain constant, and certainly will
#    vary between Bluecore/HydraCore chips. So
#    we'd need some kind of chip-specific code here anyway.
# These need to be set via a call to chip_select()
dRegions = None
pRegions = None


def chip_clear():
    """Resets the internal global architecture related variables."""
    global dRegions, pRegions, kal_arch, \
        chip_arch, chip_id, chip_cpu_speed_mhz, \
        addr_per_word, chip_revision, dm_ram_aliased
    kal_arch = 0
    chip_arch = None
    chip_id = 0
    chip_cpu_speed_mhz = 0
    addr_per_word = 0
    chip_revision = 0
    dm_ram_aliased = None

    dRegions = None
    pRegions = None


dRegions_Amber = {
    'DM1RAM': (0x000000, 0x004000),
    # The view from DM; the processor sees the PM address
    'PMRAM': (0x100000, 0x120000),
    'BAC': (0x200000, 0x300000),
    # $flash.debugdata.__Base to $flash.debugdata.__Limit
    'DEBUG': (0x800000, 0x810000),
    'NVMEM1': (0xa26000, 0xc00000),
    'NVMEM0': (0xc14000, 0xe00000),
    'SLT': (0xc00008, 0xc0000a),
    # Also aliased to DM1, confusingly.
    'DM2RAM': (0xff8000, 0xffc000),
    'MMR': (0xffe000, 0x1000000),  # Memory-mapped registers
    # PMRAM which should really be a DC section but the Amber
    # coredump tool desn't to know the difference. Ignore it
    # and read the values from the chip.
    'IGNORE': (0x10000000, 0x10010000)
}

dRegions_Gordon = {
    'SLT': (0x000002, 0x000004),
    'DM1RAM': (0x000004, 0x008000),
    # The view from DM (PMWIN_24); the processor sees the PM address
    'PMRAM': (0x040000, 0x043000),
    'NVMEM1': (0xd00000, 0xe00000),  # DMFLASHWIN1_LARGE_REGION
    'NVMEM2': (0xe00000, 0xf00000),  # DMFLASHWIN2_LARGE_REGION
    # We use DMFLASHWIN3_LARGE_REGION for debug (and only debug) at present.
    'DEBUG': (0xf00000, 0xfd0000),
    'DM2RAM': (0xff3000, 0xffb000),
    'MCU1': (0xffe000, 0xfff000),  # Windowed XAP memory
    'MCU2': (0xfff000, 0xfffe00),  # Windowed XAP memory
    'MMR': (0xfffe00, 0x1000000),  # Memory-mapped registers
}

dRegions_Rick = {
    'SLT': (0x000002, 0x000004),
    'DM1RAM': (0x000004, 0x008000),
    # The view from DM (PMWIN_24); the processor sees the PM address
    'PMRAM': (0x040000, 0x043000),
    'NVMEM1': (0xd00000, 0xe00000),  # DMFLASHWIN1_LARGE_REGION
    'NVMEM2': (0xe00000, 0xf00000),  # DMFLASHWIN2_LARGE_REGION
    # We use DMFLASHWIN3_LARGE_REGION for debug (and only debug) at present.
    'DEBUG': (0xf00000, 0xfd0000),
    'DM2RAM': (0xff3000, 0xffb000),
    'MCU1': (0xffe000, 0xfff000),  # Windowed XAP memory
    'MCU2': (0xfff000, 0xfffe00),  # Windowed XAP memory
    'MMR': (0xfffe00, 0x1000000),  # Memory-mapped registers
}

dRegions_KAS = {
    'DM1RAM': (0x000000, 0x008000),
    'SLT': (0x000002, 0x000004),  # SLT is a subregion of DM1RAM.
    # The view from DM1 (PMWIN_24); the processor sees the PM address
    'PMRAM': (0x040000, 0x043000),
    # We've created an out of bounds region (DEBUG_REGION).
    'DEBUG': (0xFFD000, 0xFFE000),
    'DM2RAM': (0xff3000, 0xFFA7FF),
    'NVMEM0': (0xFFA800, 0xFFA9FF),  # const16
    'NVMEM1': (0xFFAa00, 0xFFAF99),  # const24
    'MMR': (0xfffe00, 0x1000000),  # Memory-mapped registers
    'DM2SHARED': (0xFFAF9A, 0xFFAFFF),  # Memory shared with host
}

dRegions_Crescendo_d00 = {
    'DM1RAM': (0x00000000, 0x000C0000),
    # The view from DM; used for constants right now.
    'PMRAM': (0x00100000, 0x00130000),
    'BAC': (0x00800000, 0x01000000),
    # 'DMFLASHWIN1_LARGE_REGION' (note name confusion)
    'NVMEM0': (0xf8000000, 0xf8800000),
    # 'DMFLASHWIN2_LARGE_REGION' (note name confusion)
    'NVMEM1': (0xf8800000, 0xf9000000),
    'NVMEM2': (0xf9000000, 0xf9800000),
    'NVMEM3': (0xf9800000, 0xfa000000),
    # Lots more NVMEM windows here...
    'DEBUG': (0x13500000, 0x13600000),
    'DBG_PTCH': (0x14500000, 0x14600000),
    'DBG_DWL': (0x15500000, 0x15600000),
    'DM2RAM': (0xfff00000, 0xfffc0000),  # Aliased to DM1.
    'MMR': (0xffff8000, 0x100000000)  # Memory-mapped registers
}

dRegions_Crescendo_d01 = {
    'DM1RAM': (0x00000000, 0x00080000),
    # The view from DM; used for constants right now.
    'PMRAM': (0x00100000, 0x00130000),
    'BAC': (0x00800000, 0x00C00000),
    # 'DMFLASHWIN1_LARGE_REGION' (note name confusion)
    'NVMEM0': (0xf8000000, 0xf8800000),
    # 'DMFLASHWIN2_LARGE_REGION' (note name confusion)
    'NVMEM1': (0xf8800000, 0xf9000000),
    'NVMEM2': (0xf9000000, 0xf9800000),
    'NVMEM3': (0xf9800000, 0xfa000000),
    'DMSRAM': (0xfa000000, 0xfa800000),  # SRAM
    # Lots more NVMEM windows here...
    'DEBUG': (0x13500000, 0x13600000),
    'DBG_PTCH': (0x14500000, 0x14600000),
    'DBG_DWL': (0x15500000, 0x15600000),
    'DM2RAM': (0xfff00000, 0xfff80000),  # Aliased to DM1.
    'MMR': (0xffff8000, 0x100000000)  # Memory-mapped registers
}

dRegions_Aura = {
    'DM1RAM': (0x00000000, 0x00040000),
    # The view from DM; used for constants right now.
    'PMRAM': (0x00100000, 0x00114000),
    'BAC': (0x00800000, 0x00C00000),
    # 'DMFLASHWIN1_LARGE_REGION' (note name confusion)
    'NVMEM0': (0xf8000000, 0xf8800000),
    # 'DMFLASHWIN2_LARGE_REGION' (note name confusion)
    'NVMEM1': (0xf8800000, 0xf9000000),
    'NVMEM2': (0xf9000000, 0xf9800000),
    'NVMEM3': (0xf9800000, 0xfa000000),
    'DMSRAM': (0xfa000000, 0xfa800000),  # SRAM
    # Lots more NVMEM windows here...
    'DEBUG': (0x13500000, 0x13600000),
    'DBG_PTCH': (0x14500000, 0x14600000),
    'DBG_DWL': (0x15500000, 0x15600000),
    'DM2RAM': (0xfff00000, 0xfff40000),  # Aliased to DM1.
    'MMR': (0xffff8000, 0x100000000)  # Memory-mapped registers
}

dRegions_AuraPlus = {
    'DM1RAM': (0x00000000, 0x00070000),
    # The view from DM; used for constants right now.
    'PMRAM': (0x00100000, 0x0011C000),
    'BAC': (0x00800000, 0x00C00000),
    # 'DMFLASHWIN1_LARGE_REGION' (note name confusion)
    'NVMEM0': (0xf8000000, 0xf8800000),
    # 'DMFLASHWIN2_LARGE_REGION' (note name confusion)
    'NVMEM1': (0xf8800000, 0xf9000000),
    'NVMEM2': (0xf9000000, 0xf9800000),
    'NVMEM3': (0xf9800000, 0xfa000000),
    'NVMEM4': (0xfa000000, 0xfa800000),
    'NVMEM5': (0xfa800000, 0xfb000000),
    'NVMEM6': (0xfb000000, 0xfb800000),
    'NVMEM7': (0xfb800000, 0xfc000000),
    # Lots more NVMEM windows here...
    'DEBUG': (0x13500000, 0x13600000),
    'DBG_PTCH': (0x14500000, 0x14600000),
    'DBG_DWL': (0x15500000, 0x15600000),
    'DM2RAM': (0xfff00000, 0xfff70000),  # Aliased to DM1.
    'MMR': (0xffff8000, 0x100000000)  # Memory-mapped registers
}

dRegions_Napier = {
    'DM1RAM': (0x00000000, 0x00048000),
    # The view from DM; used for constants right now.
    'PMRAM': (0x00100000, 0x00110000),
    # 'DMFLASHWIN1_LARGE_REGION' (note name confusion)
    'NVMEM0': (0xf8000000, 0xf8800000),
    # 'DMFLASHWIN2_LARGE_REGION' (note name confusion)
    'NVMEM1': (0xf8800000, 0xf9000000),
    'NVMEM2': (0xf9000000, 0xf9800000),
    'NVMEM3': (0xf9800000, 0xfa000000),
    # Lots more NVMEM windows here...
    'DEBUG': (0x13500000, 0x13600000),
    'DM2RAM': (0xfff00000, 0xfff48000),  # Aliased to DM1.
    'MMR': (0xffff8000, 0x100000000)  # Memory-mapped registers
}

# PM regions
pRegions_Amber = {
    'PMCACHE': (0x000000, 0x000800),
    'PMRAM': (0x000000, 0x004000),
    'PMROM': (0x400000, 0x440000)
}

pRegions_Gordon = {
    'PMRAM': (0x000000, 0x002c00),
    'PMCACHE': (0x002c00, 0x003000),
    # Technically could run up to 0xffffff, but no-one has that much flash.
    'PMROM': (0x003000, 0x100000)
}

pRegions_Rick = {
    'PMRAM': (0x000000, 0x00b000),
    'PMCACHE': (0x00b000, 0x00c000),
    # Technically could run up to 0xffffff, but no-one has that much flash.
    'PMROM': (0x00c000, 0x200000)
}

pRegions_KAS = {
    'PMRAM': (0x000000, 0x040000)  # No NVMEM
}

pRegions_Crescendo = {
    'PMCACHE': (0, 0),  # Doesn't seem to be one at the moment
    'PMROM': (0x00000000, 0x02000000),
    # No defined region for SLT at the moment
    'SLT': (0x00000020, 0x00000028),
    'PMRAM': (0x04000000, 0x04030000)
}

pRegions_Aura = {
    'PMCACHE': (0, 0),  # Doesn't seem to be one at the moment
    'PMROM': (0x00000000, 0x02000000),
    'SLT': (0x00000020, 0x00000028),
    'PMRAM': (0x04000000, 0x04014000)
}

pRegions_AuraPlus = {
    'PMCACHE': (0, 0),  # Assume that the PM cache is disabled
    'PMROM': (0x00000000, 0x02000000),
    'SLT': (0x00000020, 0x00000028),
    'PMRAM': (0x04000000, 0x0401C000)
}


pRegions_Napier = {
    'PMCACHE': (0, 0),  # Doesn't seem to be one at the moment
    'PMROM': (0x00000000, 0x02000000),
    # No defined region for SLT at the moment
    'SLT': (0x00000020, 0x00000028),
    'PMRAM': (0x04000000, 0x04010000)
}

#####################
# Utility functions #
#####################


def chip_select(l_kal_arch, l_chip_arch, l_chip_id, l_chip_revision=None):
    """Sets dRegions and pRegions, based on chip architecture/ID.

    * l_kal_arch is an integer, e.g. 3, 4, 5
    * l_chip_arch is a string containing e.g. "Bluecore", "Hydra", "KAS"
    * l_chip_id is an integer representing the chip id, e.g. 28
      (see http://wiki/ChipVersionIDs)
    This function must be called prior to any calls to get_dm_region or
    get_pm_region.

    Args:
        l_kal_arch (int)
        l_chip_arch (str)
        l_chip_id (int)
        l_chip_revision
    """
    # pylint: disable=global-statement
    # pylint: disable=too-many-statements
    # At this stage, the chip_id isn't necessary to distinguish between
    # the chips we support, but eventually it probably will be.
    global dRegions, pRegions, kal_arch, \
        chip_arch, chip_id, chip_cpu_speed_mhz, \
        addr_per_word, chip_revision, dm_ram_aliased

    kal_arch = l_kal_arch
    chip_arch = l_chip_arch
    chip_id = l_chip_id
    chip_revision = l_chip_revision

    old_dRegions = dRegions
    old_pRegions = pRegions

    if kal_arch == 3 and chip_arch == "Bluecore":
        # Gordon or one of its descendents.
        dRegions = dRegions_Gordon
        pRegions = pRegions_Gordon
        chip_cpu_speed_mhz = 80
        addr_per_word = 1
        dm_ram_aliased = False
    elif (kal_arch == 4 and chip_arch == "Hydra" and
            chip_id == 0x46 and chip_revision <= 0x1F):
        # Crescendo d00 or one of its cohorts
        dRegions = dRegions_Crescendo_d00
        pRegions = pRegions_Crescendo
        chip_cpu_speed_mhz = 240
        addr_per_word = 4
        dm_ram_aliased = True
    elif (kal_arch == 4 and chip_arch == "Hydra" and
            chip_id == 0x46 and chip_revision > 0x1F):
        # Crescendo d01
        dRegions = dRegions_Crescendo_d01
        pRegions = pRegions_Crescendo
        chip_cpu_speed_mhz = 240
        addr_per_word = 4
        dm_ram_aliased = True
    elif kal_arch == 4 and chip_arch == "Hydra" and chip_id == 0x49:
        # Aura
        dRegions = dRegions_Aura
        pRegions = pRegions_Aura
        chip_cpu_speed_mhz = 120
        addr_per_word = 4
        dm_ram_aliased = True
    elif kal_arch == 4 and chip_arch == "Hydra" and chip_id == 0x4A:
        # Stretto uses the same memory map as Aura.
        dRegions = dRegions_Aura
        pRegions = pRegions_Aura
        chip_cpu_speed_mhz = 120
        addr_per_word = 4
        dm_ram_aliased = True
    elif kal_arch == 4 and chip_arch == "Hydra" and chip_id == 0x4b:
        # AuraPlus
        dRegions = dRegions_AuraPlus
        pRegions = pRegions_AuraPlus
        chip_cpu_speed_mhz = 120
        addr_per_word = 4
        dm_ram_aliased = True
    elif kal_arch == 4 and chip_arch == "Hydra" and chip_id == 0x4c:
        # StrettoPlus uses the same memory map as AuraPlus.
        dRegions = dRegions_AuraPlus
        pRegions = pRegions_AuraPlus
        chip_cpu_speed_mhz = 120
        addr_per_word = 4
        dm_ram_aliased = True
    elif kal_arch == 5 and chip_arch == "Bluecore":
        # Rick
        dRegions = dRegions_Rick
        pRegions = pRegions_Rick
        chip_cpu_speed_mhz = 120
        addr_per_word = 1
        dm_ram_aliased = False
    elif kal_arch == 5 and chip_arch == "Hydra":
        # Assume this is Amber, for now. We might need to make further
        # distinction later.
        dRegions = dRegions_Amber
        pRegions = pRegions_Amber
        chip_cpu_speed_mhz = 80
        addr_per_word = 1
        dm_ram_aliased = True
    elif kal_arch == 5 and chip_arch == "KAS":
        # KAS
        dRegions = dRegions_KAS
        pRegions = pRegions_KAS
        chip_cpu_speed_mhz = 200
        addr_per_word = 1
        dm_ram_aliased = False
    elif kal_arch == 4 and chip_arch == "Napier":
        # Napier
        dRegions = dRegions_Napier
        pRegions = pRegions_Napier
        chip_cpu_speed_mhz = 128
        addr_per_word = 4
        dm_ram_aliased = True
    else:
        raise Exception("Unknown/unsupported chip architecture")

    if old_dRegions is not None and (
            dRegions != old_dRegions or pRegions != old_pRegions
    ):
        # if the secondary processor is asleep, the chip_revision will be None.
        # must also check for kal_arch to make sure it's a platform that
        # supports multi-core (only Crescendo for now).
        if chip_revision is None and kal_arch == 4:
            raise Exception(
                "Secondary processor is asleep - cannot connect to it!")
        else:
            raise Exception("Conflicting chip details provided!")


# exceptions
class ChipNotSet(Exception):
    """
    @brief Exception:
        Signals if the chip type is not yet set internally. This could happen
        if it is not clear form the coredump or live connection the version
        of the chip.
    """


class NotPmRegion(Exception):
    """
    @brief Exception:
        Signals if an address was badly assumed to be in PM memory.
    """


class NotDmRegion(Exception):
    """
    @brief Exception:
        Signals if an address was badly assumed to be in DM memory.
    """


def get_dm_region(address, panic_on_error=True):
    """Gets symbols in DM region.

    Args:
        address
        panic_on_error (bool, optional)
    """
    if dRegions is None:
        raise ChipNotSet("Chip architecture not set")

    try:
        # Try to convert from a hex string to an integer
        address = int(address, 16)
    except TypeError:
        # Probably means address was already an integer
        pass

    region = None
    for k in dRegions.keys():
        if (address >= dRegions[k][0]) and (address < dRegions[k][1]):
            region = k

    if region is None and panic_on_error:
        raise NotDmRegion("Address 0x%x is not in any regions." % (address))
    return region


def get_pm_region(address, panic_on_error=True):
    """Get symbols in PM region.

    Args:
        address
        panic_on_error (bool, optional)
    """
    if pRegions is None:
        raise ChipNotSet("Chip architecture not set")

    try:
        # Try to convert from a hex string to an integer
        address = int(address, 16)
    except TypeError:
        # Probably means address was already an integer
        pass

    region = None
    for k in pRegions.keys():
        if (address >= pRegions[k][0]) and (address < pRegions[k][1]):
            region = k

    if region is None and panic_on_error:
        raise NotPmRegion("Address 0x%x is not in any regions." % (address))
    return region
