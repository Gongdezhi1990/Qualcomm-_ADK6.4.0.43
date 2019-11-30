
import scipy.signal as signal
import scipy.io.wavfile as wf
import scipy.io as sio
import numpy as np
from matplotlib.mlab import psd, csd
import matplotlib.pyplot as plt
import time
from TestEngine import TestEngine
from TestEngine_ANC_API import *


# CONNECT TO DEVICE
BLUESUITE = r"C:\Program Files (x86)\QTIL\BlueSuite 3.1.3\x64"
libDll = TestEngine(BLUESUITE)
TRANS = 256
handle = connectToDevice(libDll, TRANS)


# SET UP KEYS FOR FIRST 5 FILTER BANK LOCATIONS
Nfilter = 1
key = 0x204100
keyIndex = (int(Nfilter) - 1) * 2
key = key + keyIndex
key1 = key
key2 = key + (int(Nfilter+1) - 1) * 2
key3 = key + (int(Nfilter+2) - 1) * 2
key4 = key + (int(Nfilter+3) - 1) * 2
key5 = key + (int(Nfilter+4) - 1) * 2
key6 = key + (int(Nfilter+5) - 1) * 2
key7 = key + (int(Nfilter+6) - 1) * 2
key8 = key + (int(Nfilter+7) - 1) * 2
key9 = key + (int(Nfilter+8) - 1) * 2
key9 = key + (int(Nfilter+9) - 1) * 2


# READ THE ANC PARAM LIST FROM THE FIRST FILTER LOCATION
readAudioKey(libDll, handle, key1)

# SAVE ORIGINAL FILTER
saveANCparams(libDll, handle, 'Original', 1)




'''
EXAMPLE FUNCTIONS FOR MODIFYING ANC BLOCK PARAMS
'''

# DISABLE THE LEFT FEEDBACK FILTER AND SAVE
DisableANC_FFa0(libDll, handle)
saveANCparams(libDll, handle, 'FFA_Left_Disabled', 2)


# SET THE LEFT FFB DC FILTER AND SAVE
readAudioKey(libDll, handle, key1)
shift = 9
SetDCFilter_Shift_FFb0(libDll, handle, shift)
saveANCparams(libDll, handle, 'FFB_Left_DC_9', 3)


# SET THE LEFT FFB GAIN AND SAVE
readAudioKey(libDll, handle, key1)
shift = 1
SetGain_Shift_FFb0(libDll, handle, shift)
saveANCparams(libDll, handle, 'FFB_Left_GainShift_1', 4)


# SET THE LEFT LPF AND SAVE
readAudioKey(libDll, handle, key1)
shift1 = 8
shift2 = 9
SetLPF_Shift_FFb0(libDll, handle, shift1, shift2)
saveANCparams(libDll, handle, 'FFB_Left_LPF_89', 5)


'''
# EC gain
shift_left = -1
shift_right = -1
gain_left = 128
gain_right = 128
SetGain_Shift_FB0(libDll, handle, shift_left)
SetGain_Shift_FB1(libDll, handle, shift_right)
SetGain_Fine_FB0(libDll, handle, gain_left)
SetGain_Fine_FB1(libDll, handle, gain_right)


# SCALED 0 dB
num0 = [0.00055436359134890690, 0.03351395030419787600, 0.15004422473627169000, 0.08951173984586181800, -0.20386045451666931000, -0.25922310448383684000, 0.05384980337780096600, 0.13521326028835670000]
den = [0.20395974718716692000, -1.43827045543601530000, -0.94132678632267630000, 0.55652423498711745000, 0.68459128856764073000, -0.00676646198113808880, -0.05221958894278509700]
num = -1.0*np.asarray(num0)
den = np.asarray(den)
C = np.concatenate((den, num))
SetIIR_Coeffs_FB0(libDll, handle, C)
SetIIR_Coeffs_FB1(libDll, handle, C)
saveANCparams(libDll, handle, 'EC_scaled_0dB', 2)
time.sleep(3)


# SCALED 6 dB
num = -2.0*np.asarray(num0)
C = np.concatenate((den, num))
SetIIR_Coeffs_FB0(libDll, handle, C)
SetIIR_Coeffs_FB1(libDll, handle, C)
saveANCparams(libDll, handle, 'EC_scaled_6dB', 3)
time.sleep(3)


# SCALED 12 dB
num = -4.0*np.asarray(num0)
C = np.concatenate((den, num))
SetIIR_Coeffs_FB0(libDll, handle, C)
SetIIR_Coeffs_FB1(libDll, handle, C)
saveANCparams(libDll, handle, 'EC_scaled_12dB', 4)
time.sleep(3)


# SCALED 18 dB
num = -8.0*np.asarray(num0)
C = np.concatenate((den, num))
SetIIR_Coeffs_FB0(libDll, handle, C)
SetIIR_Coeffs_FB1(libDll, handle, C)
saveANCparams(libDll, handle, 'EC_scaled_18dB', 5)
time.sleep(3)



pushFilter(libDll, handle, 'Original', 1)
pushFilter(libDll, handle, 'EC_scaled_0dB', 1)
pushFilter(libDll, handle, 'EC_scaled_6dB', 1)
pushFilter(libDll, handle, 'EC_scaled_12dB', 1)
pushFilter(libDll, handle, 'EC_scaled_18dB', 1)
'''

libDll.closeTestEngine(handle)