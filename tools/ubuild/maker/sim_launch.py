"""
Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 

Provides mechanisms for launching and killing simulators
"""

import subprocess
import time
import os

def launch_audio_kse_env(devkit_root, kse_path, fw_name, kalsim_name, enable_debug, scripts, platform, patch_bundle, bundle_path):
    print "Kalsim mode selected (KALSIM_MODE project property is set to true)"
    cmd = r"start cmd.exe /C {dquotes}{d_root}\tools\python27\python.exe -m kse.kalsim.kalsim_shell --ks_path {d_root}\tools\kalsim\{k_name} --log_level 20".format(dquotes = "\"",d_root=devkit_root, k_name=kalsim_name)
    cmd = cmd + " --ks_firmware {}".format(os.path.splitext(fw_name)[0])
    cmd = cmd + " --acat_path {}".format(os.path.join(devkit_root,"audio", "acat", "ACAT"))
    cmd = cmd + " --acat_use"
    if bundle_path != "":
        cmd = cmd + " --acat_bundle {}".format(os.path.splitext(bundle_path)[0])
    cmd = cmd + " --ka_path {}".format(os.path.join(devkit_root,"tools","pythontools"))
    if enable_debug == "true":
        cmd = cmd + " --ks_debug --ka_skip"
    cmd = cmd + " --platform {} ".format(platform)
    if patch_bundle != "":
        cmd = cmd + " --hydra_ftp_server_directory {}".format(os.path.dirname(patch_bundle))
    if scripts != "":
        scripts = scripts.replace(","," ")
        cmd = cmd + " --script \"{}\"".format(scripts)
    cmd = cmd + " & pause\""
    my_env = os.environ.copy()
    my_env["KATS_WORKSPACE"] = "{}".format(kse_path)
    print "KSE launch command: {}".format(cmd)
    subprocess.Popen(cmd, env=my_env, shell=True)

    return True

def launch_sim(working_dir, kalsim_exe, kapfile):
    """
    Emulates deploying to a physical target device by using a simulator instead.
    Kills any instances of Kalsim currently running and then starts a fresh one.
    Loads the new Kalsim with the supplied kap file.
    """
    killallkal()

    cmd_line = "%s %s -d --initial-pc 0x4000000" % (kalsim_exe, kapfile)
    print "KALSIM CMD %s" % cmd_line

    si = subprocess.STARTUPINFO()
    si.dwFlags = subprocess.STARTF_USESTDHANDLES | subprocess.STARTF_USESHOWWINDOW| subprocess.CREATE_NEW_CONSOLE


    process_h = subprocess.Popen(cmd_line, startupinfo=si, cwd=working_dir)

    print 'New Kalsim PID = %s' % (process_h.pid)

def killallkal():
    """
    Kills any instances of Kalsim currently running
    """

    print 'Killing all old instances of Kalsim'

    killed = subprocess.call('taskkill /F /IM kalsim*', shell=True)


