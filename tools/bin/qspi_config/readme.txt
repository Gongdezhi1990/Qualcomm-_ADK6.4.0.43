Copyright (c) 2018 Qualcomm Technologies International, Ltd.
All Rights Reserved.
Qualcomm Technologies International, Ltd. Confidential and Proprietary.



This folder contains files which store configuration data for QSPI parts which are not
supported in the target Qualcomm device's ROM. When selecting a file, it must be taken
from the folder matching the target Qualcomm product.

Product ID                 |  Config Directory
-----------------------------------------------
CSRA68100                  |  1102_csra68100
QCC302x/3x or QCC512x      |  1283_qcc512x
CSRA68105                  |  1390_csra68105
QCC302x/3x or QCC512x      |  1500_qcc512x


Each manufacturer will have their own folder, within which .htf files can be found.
Naming convention:
    <jedec id>_<device id>_<other identifiers>.htf
File layout:
    Same format as used by configcmd.exe -
        <MIB name> = <MIB value>
        Inline comments only, starting with '#'

Files used to perform QE bit configuration are also in this folder.
Naming convention:
    <jedec id>_<device id>_<other identifiers>.inject
    'xxxx' can be used as a wildcard in place of device id
File layout:
    Should contain entries for the following:
        
        Vectors for injection:
            The values for the following are made up of 'read_length', 'vector_length'
            and 'vector' fields. They are octet strings as shown in the example below.
            
            QEEnable
                Vector to inject when enabling the QE bit
            QEDisable
                Vector to inject when disabling the QE bit
            QECheck
                Vector to inject when checking the state of the QE bit
        
        Scalar values for bitmask:
            QEMask
                Value used to mask the response of QECheck to find the QE bit
            QEValue
                Value to compare against the masked result of the QECheck vector, used
                to determine whether the QEEnable vector needs to be sent
            
Example vector:

    QEEnable = [00 07 02 01 06 03 01 40 00]
                 |  |  |  |  |  |  |  |  |
                 1  2  3  4  5  6  7  8  9

      read_length:
     1) read_length (8 bit octets)
     
      vector_length:
     2) vector_length (16 bit words, rounded up)
     
      vector:
     3) total number of commands
     4) length of command 1
     5) command 1[0]
     6) length of command 2
     7) command 2[0]
     8) command 2[1]
     9) command 2[2]