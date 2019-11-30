############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
from kcs import KCSFile

class chip_info(object):
    """
    A class representing a chip.
    This class encapsulates all information relevant to KCSMaker
    """
    A7DA_KAS  = "a7da_kas"
    CRESCENDO = "crescendo"
    STRE      = "stre"
    AURAPLUS  = "auraplus"
    NAPIER    = "napier"
    GORDON    = "gordon"
    RICK      = "rick"

    # List of currently supported chips by KCSMaker
    _supported_chips_list     = [A7DA_KAS, CRESCENDO, STRE, AURAPLUS, NAPIER]
    _pm_octet_addressing_list = [CRESCENDO, A7DA_KAS, STRE, AURAPLUS, NAPIER]
    _dm_octet_addressing_list = [CRESCENDO, STRE, AURAPLUS, NAPIER]
    _kal_arch4_list           = [CRESCENDO, STRE, AURAPLUS, NAPIER]
    _kal_arch5_list           = [A7DA_KAS]
    _minim_list               = [A7DA_KAS, CRESCENDO, STRE, AURAPLUS, NAPIER]

    _kld_list                 = [CRESCENDO, STRE, AURAPLUS, NAPIER]
    _mark_code_section_type   = [CRESCENDO, STRE, AURAPLUS, NAPIER]

    # Get this from http://wiki/ChipVersionIDs
    # DSP_ID in A7DA chip is a special case, see B-159218 if interested
    _dsp_id_dict = {GORDON: 0xC9, A7DA_KAS: 0x60A0, NAPIER: 0x61, CRESCENDO: 0x46, STRE: 0x49, AURAPLUS: 0x4B}

    _default_kcs_type = {GORDON: KCSFile.STREAM_TYPE, A7DA_KAS: KCSFile.RANDOM_ACCESS_TYPE, CRESCENDO: KCSFile.STREAM_TYPE, STRE: KCSFile.STREAM_TYPE, AURAPLUS: KCSFile.STREAM_TYPE, NAPIER: KCSFile.STREAM_TYPE}

    # Conversion table for CRT libraries
    _crt_lib_name = {A7DA_KAS: "a7da", CRESCENDO : "csra68100_audio", STRE : "QCC302x_audio", AURAPLUS : "QCC514x_audio", NAPIER : "napier_audio"}

    def __init__(self, chip_name):
        if chip_name.lower() == chip_info.A7DA_KAS:
            self.chip_name = chip_info.A7DA_KAS
        elif chip_name.lower() == chip_info.CRESCENDO:
            self.chip_name = chip_info.CRESCENDO
        elif chip_name.lower() == chip_info.STRE:
            self.chip_name = chip_info.STRE
        elif chip_name.lower() == chip_info.AURAPLUS:
            self.chip_name = chip_info.AURAPLUS
        elif chip_name.lower() == chip_info.NAPIER:
            self.chip_name = chip_info.NAPIER
        elif chip_name.lower() == chip_info.GORDON:
            self.chip_name = chip_info.GORDON
        elif chip_name.lower() == chip_info.RICK:
            self.chip_name = chip_info.RICK
        else:
            raise Exception("Unsupported chip type")

    def marks_code_section_type(self):
        if self.chip_name in chip_info._mark_code_section_type:
            return True
        else:
            return False

    def supports_minim(self):
        if self.chip_name in chip_info._minim_list:
            return True
        else:
            return False

    def get_crt_lib_name(self):
        return chip_info._crt_lib_name[self.chip_name]

    def get_dsp_id(self):
        return chip_info._dsp_id_dict[self.chip_name]

    def get_string(self):
        return self.chip_name

    def get_linkscript_template_name(self):
        return "linkscript_" + self.chip_name

    def get_KAL_ARCH(self):
        if self.chip_name in chip_info._kal_arch4_list:
            return "KAL_ARCH4"
        elif self.chip_name in chip_info._kal_arch5_list:
            return "KAL_ARCH5"
        else:
            raise Exception("Unsupported chip type")

    def is_supported(self):
        if self.chip_name in chip_info._supported_chips_list:
            return True
        else:
            return False

    def does_pm_octet_addressing(self):
        if self.chip_name in chip_info._pm_octet_addressing_list:
            return True
        else:
            return False

    def does_dm_octet_addressing(self):
        if self.chip_name in chip_info._dm_octet_addressing_list:
            return True
        else:
            return False

    def uses_kld(self):
        if self.chip_name in chip_info._kld_list:
            return True
        else:
            return False

    def get_default_kcs_type(self):
        return chip_info._default_kcs_type[self.chip_name]
