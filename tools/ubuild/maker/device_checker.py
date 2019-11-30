import time
import os


def check_device_with_timeout(device_url, wait_before_check=5, timeout=10, devkit_path=os.path.join('..', '..', '..')):
    """
    Attaches to a device using pydbg and checks if the device is readable.
    This keeps checking until either:
        - The check is successful
        - The timeout (in seconds) expires
    """
    _append_pylib_path(devkit_path)

    print("Checking device: %s" % device_url)
    pydbg_device_url = _pydbg_device_url(device_url)

    time.sleep(wait_before_check)

    stop_time = time.time() + timeout
    while stop_time - time.time() > 0:
        if check_device(pydbg_device_url):
            return True
        else:
            print("Retrying...")
            time.sleep(1)

    print("ERROR: Device check failed!")
    return False


def check_device(pydbg_device_url):
    """
    Attaches to a device using pydbg and checks if the device is readable
    """
    from csr.front_end.pydbg_front_end import PydbgFrontEnd
    from csr.transport.tctrans import TcError
    try:
        device, _ = PydbgFrontEnd.attach({"device_url": pydbg_device_url}, interactive=False)
    except TcError:
        print("Connection failed")
        return False

    print("Connected to device")
    print("Checking if device is readable...")
    try:
        device_is_readable = device.chip.curator_subsystem.core.data[0x8000]
        return True
    except RuntimeError:
        print("Device not readable")
        return False


def _pydbg_device_url(qmde_device_url):
    """
    Convert QMDE syntax for dongles into pydbg's.
    E.g.: device://tc/usb2tc/105/qcc5120 to tc:usb2tc:105
    """
    device_url_no_scheme = qmde_device_url.split('://')[1]
    device_url_parts = device_url_no_scheme.split('/')
    device_url_parts_no_device_name = device_url_parts[:-1]
    return ':'.join(device_url_parts_no_device_name)


def _append_pylib_path(devkit_path):
    import sys
    path = os.path.normpath(os.path.join(devkit_path, "apps", "fw", "tools", "pylib"))
    if path not in sys.path:
        sys.path.append(path)
