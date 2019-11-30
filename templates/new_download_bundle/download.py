# The following script will be run when deploying the project into the simulator environment (KSE)
# When the DOS shell is presented, it automatically performs a download of the download_@@@project@@@.dkcs. 
# You can add additional setup commands to this file if desired.
# Please refer to the Kymera Simulator Environment User Guide for instructions on how to use it.
# Please note: This file is ONLY used for KSE (Kymera Simulator Environment)

if __name__ == '__main__':

    print('Downloading download_@@@project@@@.dkcs')

    id = hydra_cap_download.download('script/kalsim/download_@@@project@@@/download_@@@project@@@.dkcs')
    print('Bundle downloaded. ID is %s' % (id))
