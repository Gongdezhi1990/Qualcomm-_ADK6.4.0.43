# ***********************************************************************
# * Copyright 2016 Qualcomm Technologies International, Ltd.
# ***********************************************************************

from contextlib import contextmanager
from trbtrans import TrbDriver


def read16(trb, subsys, address, num_items=1, block=0):
    """
    Reads 16-bit words from a subsystem, using debug read requests.
    Multiplies the supplied address by 2 to convert it from a 16-bit word (XAP) address to a TBUS address.
    Uses one transaction per 16-bit word.

    :type trb: Trb (defined in trbtrans.py)
    :param subsys: ID of the subsystem to read from
    :param address: address to start reading from, as a 16-bit word-addressable (XAP) address.
    :param num_items: the number of words to read
    :param block: ID of the block within the subsystem.
    :return: a list of words read
    """
    octets = trb.read(subsys, block, address * 2, 2, num_items * 2)
    sixteen_bit_words = []
    for i in xrange(0, len(octets), 2):
        sixteen_bit_words.append((octets[i] << 8) + octets[i + 1])
    return sixteen_bit_words


def read32(trb, subsys, address, num_items=1, block=0):
    """
    Reads 32-bit words from a subsystem, using debug read requests.

    The address should be a TBUS address -- e.g. for a Kalimba subsystem on CSRA68100,
    bits 28-31 should be set to properly by the caller to route the read to desired destination.

    Uses one transaction per 32-bit word.

    :type trb: Trb (defined in trbtrans.py)
    :param subsys: ID of the subsystem to read from
    :param address: address to start reading from
    :param num_items: the number of words to read
    :param block: ID of the block within the subsystem.
    :return: a list of words read
    """
    octets = trb.read(subsys, block, address, 4, num_items * 4)
    thirty_two_bit_words = []
    for i in xrange(0, len(octets), 4):
        thirty_two_bit_words.append(
            (octets[i] << 24) + (octets[i + 1] << 16) + (octets[i + 2] << 8) + octets[i + 3]
        )
    return thirty_two_bit_words


class TrbDongleUnsupported(Exception):
    pass


def get_trb_clock_mhz(trb):
    """
    Returns the clock rate the TRB interface is running at.
    Only supported on usb2trb; on other targets, raises TrbDongleUnsupported.

    :type trb: Trb (defined in trbtrans.py)
    """
    if trb.get_dongle_details().driver != TrbDriver.USB2TRB:
        raise TrbDongleUnsupported("This operation is only supported on usb2trb")

    val = trb.read_dongle_register(0, 0x1064)

    base_freq_mhz = 40.0
    clock_mult = val & 0xff
    clock_div = (val & 0xff00) >> 8

    return base_freq_mhz * clock_mult / clock_div


def set_trb_clock_mhz(trb, clock_mhz):
    """
    Change the clock speed the TRB interface runs at.
    Only supported on usb2trb; on other targets, raises TrbDongleUnsupported.

    :type trb: Trb (defined in trbtrans.py)
    :param clock_mhz: must be one of 10, 20, 30, 40, 50, 60, 70, 80, 100, 110 or 120.
    """
    if trb.get_dongle_details().driver != TrbDriver.USB2TRB:
        raise TrbDongleUnsupported("This operation is only supported on usb2trb")

    if clock_mhz < 10 or clock_mhz > 120:
        raise ValueError("clock_mhz must be between 10 and 120 (inclusive)")

    if clock_mhz % 10 != 0:
        raise ValueError("clock_mhz must be a multiple of 10")

    clock_div_and_mul_table = {
        120: (4, 12),
        110: (4, 11),
        100: (4, 10),
        90:  (4, 9),
        80:  (4, 8),
        70:  (4, 7),
        60:  (4, 6),
        50:  (4, 5),
        40:  (4, 4),
        30:  (4, 3),
        20:  (4, 2),
        10:  (8, 2)  # Avoid an illegal multiplier of unity
    }

    val = trb.read_dongle_register(0, 0x1064)
    val &= 0xfffff0f0
    val |= clock_div_and_mul_table[clock_mhz][0] << 8
    val |= clock_div_and_mul_table[clock_mhz][1]

    trb.write_dongle_register(0, 0x1064, val)


# noinspection PyPep8Naming
def _toggle_usb2trb_router(stream, enable):
    """
    Enable the "router" on the usb2trb dongle, so that the debug subsystem SDRAM can be accessed.
    """
    T_BRIDGE_32_BASE_ADDRESS = 0x1000
    TRB_ROUTER_CTRL_AD = T_BRIDGE_32_BASE_ADDRESS + 0x00A0
    # bit 0     - ROUTER_ENABLE_IN
    # bit 1     - ROUTER_DATA_REQ_ADDR_LOOKUP_EN_IN
    # bit 2     - ROUTER_DEBUG_REQ_ADDR_LOOKUP_EN_IN
    TRB_ROUTER_ADDR_LOOKUP_VALUE_AD = T_BRIDGE_32_BASE_ADDRESS + 0x00A4
    TRB_ROUTER_ADDR_LOOKUP_MASK_AD  = T_BRIDGE_32_BASE_ADDRESS + 0x00A8

    if enable:
        stream.write_dongle_register(0, TRB_ROUTER_CTRL_AD, 0x00000007)
        stream.write_dongle_register(0, TRB_ROUTER_ADDR_LOOKUP_VALUE_AD, 0x00000000)
        stream.write_dongle_register(0, TRB_ROUTER_ADDR_LOOKUP_MASK_AD, 0x80000000)
    else:
        stream.write_dongle_register(0, TRB_ROUTER_CTRL_AD, 0x0)


def enable_usb2trb_router(stream):
    _toggle_usb2trb_router(stream, True)


def disable_usb2trb_router(stream):
    _toggle_usb2trb_router(stream, False)


@contextmanager
def scoped_usb2trb_router_session(stream):
    try:
        enable_usb2trb_router(stream)
        yield
    finally:
        disable_usb2trb_router(stream)
