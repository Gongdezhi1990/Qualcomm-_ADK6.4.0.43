
import os
import subprocess

import maker.subsystem_numbers as subsys_numbers

  
class NvsCmd(object):
    """
    Python wrapper for the nvscmd tool
    """
    
    def __init__(self, tools_dir, transport_uri):
        """
        init the tool
        :param tools_dir: Path to nvscmd tool
        """
        NVSCMD_EXE = "nvscmd.exe"
        
        self._tools_dir = tools_dir  
        self._transport, self._port = self._get_transport_port(transport_uri)
        self._nvscmd = os.path.join(tools_dir, NVSCMD_EXE)
        
    def _get_transport_port(self, transport_uri):
        # setup a default transport and port
        trans_id = "trb"
        port = 1

         # drop uri scheme, convert to list and drop device name from end
        target_list = transport_uri.split('://')[1].split('/')[:-1]
        transport = target_list[1]
        
        valid_trans = True
        if transport == "usb2trb":
            trans_id = "trb"
        elif transport == "usb2tc":
            trans_id = "usbdbg"
        else:
            valid_trans = False
            
        if valid_trans:
            port = target_list[2]
        
        transport_str = "-{0}".format(trans_id)
        port_str = "{0}".format(port)

        return (transport_str, port_str)
    
    def burn(self, image_xuv, subsys_id, reset_chip=False):        
        cmd = "%s burn %s -deviceid %d 0 %s %s -nvstype sqif -noverify" % (self._nvscmd, image_xuv, subsys_id, self._transport, self._port)
        if not reset_chip:
            cmd += ' -norun'
        return subprocess.call(cmd)

    def erase_apps(self, reset_chip=False):
        subsys_id = subsys_numbers.SubsystemNumbers.get_subsystem_number_from_name("apps1")

        cmd = "%s erase -deviceid %s 0 %s %s -nvstype sqif" % (self._nvscmd, subsys_id, self._transport, self._port)
        if not reset_chip:
            cmd += ' -norun'
        
        return subprocess.call(cmd)
