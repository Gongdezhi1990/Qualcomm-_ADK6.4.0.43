
"""
Copyright (c) 2018 Qualcomm Technologies International, Ltd

This API is an example for using Testengine.dll to calibrate ANC gains on the production line
"""

from __future__ import division # this will force floating point division in python 2.x
import ctypes as ct
import math
import sys
import os
import time
import collections
import timeit
from TestEngine_ANC_constants import *
from TestEngine import TestEngine
import numpy as np

#from numpy import loadtxt

# dict to keep track of last written gain values
GAINS = {
         'FFA0_gain': 0,
         'FFA0_shift': 0,
         'FFA0_gain_dB':0,
         'FFB0_gain': 0,
         'FFB0_shift': 0,
         'FFB0_gain_dB':0,
         'FB0_gain': 0,
         'FB0_shift': 0,
         'FB0_gain_dB':0,
         'FFA1_gain': 0,
         'FFA1_shift': 0,
         'FFA1_gain_dB':0,
         'FFB1_gain': 0,
         'FFB1_shift': 0,
         'FFB1_gain_dB':0,
         'FB1_gain': 0,
         'FB1_shift': 0,
         'FB1_gain_dB':0,
         }

# C type array to cache ANC parameter data from device
psdata = (ct.c_uint16 * ANC_DATA_NUM_BYTES)()
psdataTemp = len(ANC_PARAM_LIST) * [0]

###################################################################################################
####### Internal functions. Not to be used in external scripts to avoid wrong configurations
###################################################################################################

def connectToDevice(engine, transport, device='1'):
    """
    open connection to device. transports supported are TRB (128) and USBDBG (256)
    libDll      - handle to TestEngine.dll
    handle      - handle to device
    transport   - integer for TRB = 128 and USBDBG = 256
    """
    start_time = timeit.default_timer()
    transportDevice = ct.c_char_p(device.encode())
    dataRate = ct.c_uint32(0)
    retryTimeOut = ct.c_int32(1000)
    usbTimeOut = ct.c_int32(1000)
    handle = engine.openTestEngine(transport, transportDevice, dataRate, retryTimeOut, usbTimeOut)
    if handle:
        print('USB Connection succesful to handle ' + str(handle))
    else:
        raise ValueError('Could not connect to device')
    #print('Operation completed in ' + str((timeit.default_timer() - start_time)) + ' seconds')
    return handle


def getSourceID(engine, handle, device, instance, channel):
    '''
    get ID to source (ADC)
    engine      - handle to Testengine.dll
    handle      - handle to 8675 device
    device      - The stream device identifier, where: 3 = analog mic, 6 = digital mic
    instance    - hardware instance, can be 0,1,2
    channel     - 0 = left channel, 1 = right channel
    Returns     - sourceID stream ID
    Example: sourceID = getSourceID(engine, handle, DEVICE, ADC_INSTANCE, CHANNEL_A)
    '''
    if device == ANALOG_MIC:
        print('Using analog mic')
    elif device == DIGITAL_MIC:
        print('Using digital mic')
    success, sourceID = engine.teAudioGetSource(handle, ct.c_uint16(device), ct.c_uint16(instance), ct.c_uint16(channel))
    if not(success):
        raise ValueError('Source ID failed')
    return sourceID


def audioConfigure(engine, handle, streamID, key, value):
    '''
    helper function to call teAudioConfigure
    '''
    return engine.teAudioConfigure(handle, ct.c_uint16(streamID), ct.c_uint16(key), ct.c_uint32(value))


def smLPFEnable(engine, handle, anc_mic_path, enable):
    if (anc_mic_path.instance == None):
        if (enable):
            print anc_mic_path.name,' instance does not seem to be configured.'
        smLPFEnableOk = -1
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        smLPFEnableOk = audioConfigure(engine, handle, streamID, anc_mic_path.smLPF_enable, enable)
        if smLPFEnableOk == 1:
            if enable == 1:
                print('ANC smLPF enabled')
                if anc_mic_path.name == 'FFA0':
                    setParam('OFFSET_SMLPF_ENABLE_L',1)
                elif anc_mic_path.name == 'FFA1':
                    setParam('OFFSET_SMLPF_ENABLE_R',1)
            else:
                print('ANC smLPF disabled')
                if anc_mic_path.name == 'FFA0':
                    setParam('OFFSET_SMLPF_ENABLE_L',0)
                elif anc_mic_path.name == 'FFA1':
                    setParam('OFFSET_SMLPF_ENABLE_R',0)
        else:
            raise ValueError('Error when attempting to enable smLPF')
    return smLPFEnableOk


def smLPFShift(engine, handle, anc_mic_path, shift):
    if (anc_mic_path.instance == None):
        print anc_mic_path.name,' instance does not seem to be configured.'
        smLPFShiftOk = -1
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        smLPFShiftOk = audioConfigure(engine, handle, streamID, anc_mic_path.smLPF_shift, shift)
        if smLPFShiftOk == 1:
            print('ANC smLPF shift set = ' + str(shift))
            if anc_mic_path.name == 'FFA0':
                setParam('OFFSET_SM_LPF_SHIFT_L', shift)
            elif anc_mic_path.name == 'FFA1':
                setParam('OFFSET_SM_LPF_SHIFT_R', shift)
        else:
            raise ValueError('Error when attempting to set smLPF shift')
    return smLPFShiftOk


def dcFilter_Enable(engine, handle, anc_mic_path, enable):
    if (anc_mic_path.instance == None):
        if (enable):
            print anc_mic_path.name,' instance does not seem to be configured.'
        dcFilter_EnableOk = -1
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        dcFilter_EnableOk = audioConfigure(engine, handle, streamID, anc_mic_path.dcfilter_enable, enable)
        if dcFilter_EnableOk == 1:
            if enable == 1:
                print('ANC DC filter enabled')
                if anc_mic_path.name == 'FFA0':
                    setParam('OFFSET_FF_A_DCFLT_ENABLE_L',1)
                elif anc_mic_path.name == 'FFB0':
                    setParam('OFFSET_FF_B_DCFLT_ENABLE_L',1)
                elif anc_mic_path.name == 'FFA1':
                    setParam('OFFSET_FF_A_DCFLT_ENABLE_R',1)
                elif anc_mic_path.name == 'FFB1':
                    setParam('OFFSET_FF_B_DCFLT_ENABLE_R',1)
            else:
                print('ANC DC filter disabled')
                if anc_mic_path.name == 'FFA0':
                    setParam('OFFSET_FF_A_DCFLT_ENABLE_L',0)
                elif anc_mic_path.name == 'FFB0':
                    setParam('OFFSET_FF_B_DCFLT_ENABLE_L',0)
                elif anc_mic_path.name == 'FFA1':
                    setParam('OFFSET_FF_A_DCFLT_ENABLE_R',0)
                elif anc_mic_path.name == 'FFB1':
                    setParam('OFFSET_FF_B_DCFLT_ENABLE_R',0)
        else:
            raise ValueError('Error when attempting to enable DC filter')
    return dcFilter_EnableOk


def dcFilter_Shift(engine, handle, anc_mic_path, shift):
    if (anc_mic_path.instance == None):
        print anc_mic_path.name,' instance does not seem to be configured.'
        dcFilter_ShiftOk = -1
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        dcFilter_ShiftOk = audioConfigure(engine, handle, streamID, anc_mic_path.dcfilter_shift, shift)
        if dcFilter_ShiftOk == 1:
            print('ANC DC filter shift set = ' + str(shift))
            if anc_mic_path.name == 'FFA0':
                setParam('OFFSET_FF_A_DCFLT_SHIFT_L', shift)
            elif anc_mic_path.name == 'FFB0':
                setParam('OFFSET_FF_B_DCFLT_SHIFT_L', shift)
            elif anc_mic_path.name == 'FFA1':
                setParam('OFFSET_FF_A_DCFLT_SHIFT_R', shift)
            elif anc_mic_path.name == 'FFB1':
                setParam('OFFSET_FF_B_DCFLT_SHIFT_R', shift)
        else:
            raise ValueError('Error when attempting to set DC filter shift')
    return dcFilter_ShiftOk


def AncEnable(engine, handle, anc_left_enable, anc_right_enable):
    '''
    enable ANC path
    '''
    print('Enabling ANC with the following bit fields for left and right')
    print(anc_left_enable)
    print(anc_right_enable)
    success = engine.teAudioStreamAncEnable(handle, ct.c_uint16(anc_left_enable), ct.c_uint16(anc_right_enable))
    ###
    if success == 1:
        print('ANC enabled')
    else:
        raise ValueError('Error when attempting to enable ANC')
    return success


def AncGainConvertFromdB(gainDB):
    '''
    convert gain dB value to gain and shift values
    '''
    if type(gainDB) is str:
        shift = 0
        gain = 0
        return shift, gain

    # check for max gain
    if gainDB > MAX_GAIN_DB:
        print('Gain value too high, forcing to max value')
        gainDB = MAX_GAIN_DB
        shift = MAX_GAIN_SHIFT
        gain = MAX_GAIN
    # gain is between 0 and +42
    elif gainDB >= 0:
        # find nearest shift value
        a = gainDB / 6
        shift = int(a)
        gain_remain = gainDB - (shift * 6)
        gain = 128*(10**(gain_remain/20))
    #gain is negative
    elif gainDB < 0:
        # check for lowest shift value of -24
        if gainDB < -24:
            shift = -4 # set shift to lowest value of -24dB
        else:
            # find nearest shift value
            a = gainDB / 6
            shift = int(a)
        gain_remain = gainDB + (abs(shift)*6)
        gain = 128*(10**(gain_remain/20))
	#TimD
    print "shift:", shift, "gain hex:", hex(int(gain))
    return shift, int(gain)


def AncGainConvertToDb(shift, gain):
    '''
    LPF gain block inside the ANC block
    convert from shift and gain values to dB
    shift - LPF gain shift value (-4 to 6)
    gain - LPF gain value (0-255)
    '''
	#TimD
    #if shift >= 1 << 31:
     #   shift -= 1 << 32
    print('Converting to dB: shift of ' + str(shift) + ' and gain of ' + str(gain))
    print shift
    # gain table for shift values 0-15
    shift_gains = [0,6,12,18,24,30,36,0,0,0,0,0,-24,-18,-12,-6]
    shiftDB = shift_gains[shift] #gain shift lookup
    # calculate gain value
    if gain == 0:
        gaindB = -999
    else:
        gaindB = 20*math.log10(gain/128)
    # calculate total gain
    totalGainDb = shiftDB + gaindB
    return totalGainDb


def ConfigureStreams(engine, handle, streamID, gain, shift, gainDB):
    ''' Get Source ID then write gain and shift values to
        specified filter '''
    gainval = AncGainConvertFromdB(gainDB)
    gainOk = audioConfigure(engine, handle, streamID, gain, gainval[1])
    if not(gainOk):
        raise ValueError('Error configuring fine gain, please check that selected audio path is active')
    shiftOk = audioConfigure(engine, handle, streamID, shift, gainval[0])
    if not(shiftOk):
        raise ValueError('Error configuring coarse gain, please check that selected audio path is active')
    return gainOk, shiftOk, gainval


def SaveGains(gainOK, shiftOK, gainVal, gainDB, name):

    gain = name + '_gain'
    shift = name + '_shift'
    gain_db = name + '_gain_dB'

    if gainOK == TE_SUCCESS:
        GAINS[gain] = gainVal[1]
        if shiftOK == TE_SUCCESS:
            GAINS[shift] = gainVal[0]
            GAINS[gain_db] = gainDB
        else:
            raise ValueError('Error writing gains')
    else:
        raise ValueError('Error writing gains')


def AncSetGain(engine, handle, gainDB, anc_mic_path):
    start_time = timeit.default_timer()
    print('Setting gain on ' + anc_mic_path.name + ' to ' + str(gainDB) + ' dB')
    if anc_mic_path.instance == None:
        print 'Connection has not been configured'
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        gainOk, shiftOk, gainVal = ConfigureStreams(engine, handle, streamID, anc_mic_path.stream_gain, anc_mic_path.stream_gain_shift, gainDB)
        if anc_mic_path.name == 'FFA0':
        	setParam('OFFSET_ANC_FF_A_SHIFT_L', gainVal[0])
        elif anc_mic_path.name == 'FFB0':
        	setParam('OFFSET_ANC_FF_B_SHIFT_L', gainVal[0])
        elif anc_mic_path.name == 'FFA1':
        	setParam('OFFSET_ANC_FF_A_SHIFT_R', gainVal[0])
        elif anc_mic_path.name == 'FFB1':
        	setParam('OFFSET_ANC_FF_B_SHIFT_R', gainVal[0])

        if type(gainDB) is not str: # don't save gain when muting
            SaveGains(gainOk, shiftOk, gainVal, gainDB, anc_mic_path.name)


def AncSetGainFFA0(engine, handle, gainDB):
    '''
    write shift and gain values to FFA0 filter path
    '''
    shift, gain = AncGainConvertFromdB(gainDB)
    GAINS['FFA0_shift'] = shift
    GAINS['FFA0_gain'] = gain
    success = engine.teAudioGetSource(handle, ct.c_uint16(FFA0.device), ct.c_uint16(FFA0.instance), ct.c_uint16(FFA0.channel))
    if not(success):
        print 'FFA0 does not seem to be connected'
    else:
        updatePsdata()
        AncSetGain(engine, handle, gainDB, FFA0)


def AncSetGainFFB0(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    '''
    shift, gain = AncGainConvertFromdB(gainDB)
    GAINS['FFB0_shift'] = shift
    GAINS['FFB0_gain'] = gain
    success = engine.teAudioGetSource(handle, ct.c_uint16(FFB0.device), ct.c_uint16(FFB0.instance), ct.c_uint16(FFB0.channel))
    if not(success):
        print 'FFB0 does not seem to be connected'
    else:
    	updatePsdata()
        AncSetGain(engine, handle, gainDB, FFB0)



def AncSetGainFB0(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    FB path is associated with the FFA mic
    '''
    shift, gain = AncGainConvertFromdB(gainDB)
    GAINS['FB0_shift'] = shift
    GAINS['FB0_gain'] = gain
    success = engine.teAudioGetSource(handle, ct.c_uint16(FFA0.device), ct.c_uint16(FFA0.instance), ct.c_uint16(FFA0.channel))
    if not(success):
        print 'FFA0 does not seem to be connected'
    else:
    	updatePsdata()
        AncSetGain(engine, handle, gainDB, FB0)


def AncSetGainFFA1(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    '''
    shift, gain = AncGainConvertFromdB(gainDB)
    GAINS['FFA1_shift'] = shift
    GAINS['FFA1_gain'] = gain
    success = engine.teAudioGetSource(handle, ct.c_uint16(FFA1.device), ct.c_uint16(FFA1.instance), ct.c_uint16(FFA1.channel))
    if not(success):
        print 'FFA1 does not seem to be connected'
    else:
    	updatePsdata()
        AncSetGain(engine, handle, gainDB, FFA1)


def AncSetGainFFB1(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    '''
    shift, gain = AncGainConvertFromdB(gainDB)
    GAINS['FFB1_shift'] = shift
    GAINS['FFB1_gain'] = gain
    success = engine.teAudioGetSource(handle, ct.c_uint16(FFB1.device), ct.c_uint16(FFB1.instance), ct.c_uint16(FFB1.channel))
    if not(success):
        print 'FFB1 does not seem to be connected'
    else:
    	updatePsdata()
        AncSetGain(engine, handle, gainDB, FFB1)


def AncSetGainFB1(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    FB path is associated with the FFA mic
    '''
    shift, gain = AncGainConvertFromdB(gainDB)
    GAINS['FB1_shift'] = shift
    GAINS['FB1_gain'] = gain
    success = engine.teAudioGetSource(handle, ct.c_uint16(FB1.device), ct.c_uint16(FB1.instance), ct.c_uint16(FB1.channel))
    if not(success):
        print 'FB1 does not seem to be connected'
    else:
    	updatePsdata()
        AncSetGain(engine, handle, gainDB, FB1)


def checkForError(err_num):
    '''
    check return value for error
    '''
    if err_num == TE_FAIL:
        raise ValueError('Operation failed')
    elif err_num == TE_SUCCESS:
        print('Success')
    elif err_num == TE_UNSUPPORTED:
        raise ValueError('Unsupported function')
    else:
        raise ValueError('Unknown error')
    return err_num

def EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable):
    '''
    # General function that checks for instances that are not defined/connected and fails gracefully.
    '''
    if (anc0_enable & 0b0001) and (FFA0.instance == None):
        anc0_enable = anc0_enable & 0b1110
        setParam('OFFSET_FF_A_ENABLE_L',0)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        print 'FFA0 instance does not seem to be configured'

    if (anc0_enable & 0b0010) and (FFB0.instance == None):
        anc0_enable = anc0_enable & 0b1101
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',0)
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        print 'FFB0 instance does not seem to be configured'

    if (anc1_enable & 0b0001) and (FFA1.instance == None):
        anc1_enable = anc1_enable & 0b1110
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        setParam('OFFSET_FF_A_ENABLE_L',0)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        print 'FFA1 instance does not seem to be configured'

    if (anc1_enable & 0b0010) and (FFB1.instance == None):
        anc1_enable = anc1_enable & 0b1101
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',0)
        print 'FFB1 instance does not seem to be configured'

    print ''
    print 'FFA0',(anc0_enable & 1) != 0, '\t','FFB0',(anc0_enable & 2) != 0, '\t','FB0',(anc0_enable & 4) != 0
    print 'FFA1',(anc1_enable & 1) != 0, '\t','FFB1',(anc1_enable & 2) != 0, '\t','FB1',(anc1_enable & 4) != 0
    print ''

    '''
    The setParam functions (above) should be added below, and represent the final config that the ANC state ends up in
    '''
    if (FFA1.instance == None):
        if (anc1_enable & 1):
            print 'FFA1 instance does not seem to be configured.'
            print 'Attempting to configure Mono configuration'
        if (FFA0.instance == None):
            if (anc0_enable & 1):
                print 'FFA0 instance does not seem to be configured'
        else:
            success = engine.teAudioGetSource(handle, ct.c_uint16(FFA0.device), ct.c_uint16(FFA0.instance), ct.c_uint16(FFA0.channel))
            if not(success):
                print 'FFA0 instance does not seem to be configured.'
            else:
                AncEnable(engine, handle, anc0_enable, 0)
    else:
        success = engine.teAudioGetSource(handle, ct.c_uint16(FFA1.device), ct.c_uint16(FFA1.instance), ct.c_uint16(FFA1.channel))
        if not(success):
            print 'Attempting to configure Mono configuration'
            success = engine.teAudioGetSource(handle, ct.c_uint16(FFA0.device), ct.c_uint16(FFA0.instance), ct.c_uint16(FFA0.channel))
            if not(success):
                print 'FFA0 instance does not seem to be configured.'
            else:
                AncEnable(engine, handle, anc0_enable, 0)
        else:
            if (FFA0.instance == None):
                if anc0_enable & 1:
                    print 'FFA0 instance does not seem to be configured'
                    print 'Attempting to configure Mono configuration'
                success = AncEnable(engine, handle, 0, anc1_enable)
            else:
                success = engine.teAudioGetSource(handle, ct.c_uint16(FFA0.device), ct.c_uint16(FFA0.instance), ct.c_uint16(FFA0.channel))
                if not(success):
                    print 'FFA0 instance does not seem to be configured.'
                    print 'Attempting to configure Mono configuration'
                    success = AncEnable(engine, handle, 0, anc1_enable)
                else:
                    success = AncEnable(engine, handle, anc0_enable, anc1_enable)

def EnableDCFilter_withErrorCheck(engine, handle, anc_mic_path, enableFlag):
    '''
    # General function that checks for instances that are not defined/connected and fails gracefully.
    '''
    if (anc_mic_path.instance == None):
        if (enableFlag):
            print anc_mic_path.name,' instance does not seem to be configured.'
    else:
        dcFilter_Enable(engine, handle, anc_mic_path, enableFlag)

def SetLPF(engine, handle, anc_mic_path, LPF1, LPF2):
    if anc_mic_path.name == 'FFA0':
        ancInstance = ANC_INSTANCE_ANC0_ID
        pathId = ANC_PATH_FFA_ID
        setParam('OFFSET_ANC_FF_A_LPF_SHIFT0_L',LPF1)
        setParam('OFFSET_ANC_FF_A_LPF_SHIFT1_L',LPF2)
    else:
        if anc_mic_path.name == 'FFA1':
            ancInstance = ANC_INSTANCE_ANC1_ID
            pathId = ANC_PATH_FFA_ID
            setParam('OFFSET_ANC_FF_A_LPF_SHIFT0_R',LPF1)
            setParam('OFFSET_ANC_FF_A_LPF_SHIFT1_R',LPF2)
        else:
            if anc_mic_path.name == 'FFB0':
                ancInstance = ANC_INSTANCE_ANC0_ID
                pathId = ANC_PATH_FFB_ID
                setParam('OFFSET_ANC_FF_B_LPF_SHIFT0_L',LPF1)
                setParam('OFFSET_ANC_FF_B_LPF_SHIFT1_L',LPF2)
            else:
                if anc_mic_path.name == 'FFB1':
                    ancInstance = ANC_INSTANCE_ANC1_ID
                    pathId = ANC_PATH_FFB_ID
                    setParam('OFFSET_ANC_FF_B_LPF_SHIFT0_R',LPF1)
                    setParam('OFFSET_ANC_FF_B_LPF_SHIFT1_R',LPF2)
                else:
                    if anc_mic_path.name == 'FB0':
                        ancInstance = ANC_INSTANCE_ANC0_ID
                        pathId = ANC_PATH_FB_ID
                        setParam('OFFSET_ANC_FB_LPF_SHIFT0_L',LPF1)
                        setParam('OFFSET_ANC_FB_LPF_SHIFT1_L',LPF2)
                    else:
                        if anc_mic_path.name == 'FB1':
                            ancInstance = ANC_INSTANCE_ANC1_ID
                            pathId = ANC_PATH_FB_ID
                            setParam('OFFSET_ANC_FB_LPF_SHIFT0_R',LPF1)
                            setParam('OFFSET_ANC_FB_LPF_SHIFT1_R',LPF2)

    success = engine.teAudioSetAncLpfFilter(handle, ct.c_uint16(ancInstance), ct.c_uint16(pathId), ct.c_uint16(LPF1), ct.c_uint16(LPF2))
    if success == 1:
        print('ANC LPF shift values set')
    else:
        raise ValueError('Error when attempting to set LPF parameters')
    return success

def LPFSetShiftGain(engine, handle, gainShift, anc_mic_path):
    """
    """
    if anc_mic_path.instance == None:
        print 'Connection has not been configured'
        shiftOk = 0
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        shiftOk = audioConfigure(engine, handle, streamID, anc_mic_path.stream_gain_shift, gainShift)
        if not(shiftOk):
            raise ValueError('Error configuring coarse gain, please check that selected audio path is active')
        else:
            print anc_mic_path.name, ': Shift gain changed to ', gainShift
            if anc_mic_path.name == 'FFA0':
            	setParam('OFFSET_ANC_FF_A_SHIFT_L', gainShift)
            elif anc_mic_path.name == 'FFA1':
                setParam('OFFSET_ANC_FF_A_SHIFT_R', gainShift)
            elif anc_mic_path.name == 'FFB0':
                setParam('OFFSET_ANC_FF_B_SHIFT_L', gainShift)
            elif anc_mic_path.name == 'FFB1':
                setParam('OFFSET_ANC_FF_B_SHIFT_R', gainShift)
            elif anc_mic_path.name == 'FB0':
                setParam('OFFSET_ANC_FB_SHIFT_L', gainShift)
            elif anc_mic_path.name == 'FB1':
                setParam('OFFSET_ANC_FB_SHIFT_R', gainShift)

    return shiftOk

def LPFSetFineGain(engine, handle, gainFine, anc_mic_path):
    if anc_mic_path.instance == None:
        print 'Connection has not been configured'
        gainOk =  0
    else:
        streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
        gainOk = audioConfigure(engine, handle, streamID, anc_mic_path.stream_gain, gainFine)
        if not(gainOk):
            raise ValueError('Error configuring fine gain, please check that selected audio path is active')
        else:
            print anc_mic_path.name, ': Fine gain changed to ', gainFine
            if anc_mic_path.name == 'FFA0':
            	setParam('OFFSET_ANC_FF_A_GAIN_L', gainFine)
            elif anc_mic_path.name == 'FFA1':
                setParam('OFFSET_ANC_FF_A_GAIN_R', gainFine)
            elif anc_mic_path.name == 'FFB0':
                setParam('OFFSET_ANC_FF_B_GAIN_L', gainFine)
            elif anc_mic_path.name == 'FFB1':
                setParam('OFFSET_ANC_FF_B_GAIN_R', gainFine)
            elif anc_mic_path.name == 'FB0':
                setParam('OFFSET_ANC_FB_GAIN_L', gainFine)
            elif anc_mic_path.name == 'FB1':
                setParam('OFFSET_ANC_FB_GAIN_R', gainFine)

    return gainOk

'''
    Function :      int32 teAudioSetAncIirFilter(uint32 handle, uint16 ancInstance,
                                                 uint16 pathId,
                                                 const uint16* coefficients,
                                                 uint16 numCoeffs)

typedef enum {
    AUDIO_ANC_INSTANCE_0    = 0x0001,
    AUDIO_ANC_INSTANCE_1    = 0x0002
} audio_anc_instance;

/*!
  @brief audio_anc_mic_path_id used in AudioAncFilterIirSet() and AudioAncFilterLpfSet traps.
*/
typedef enum {
    AUDIO_ANC_PATH_ID_FFA    = 0x0001,
    AUDIO_ANC_PATH_ID_FFB    = 0x0002,
    AUDIO_ANC_PATH_ID_FB     = 0x0003,
} audio_anc_path_id;

coeffs = [0,0,0,0,0,0,0,0,0x200,0,0,0,0,0,0]

'''

def SetIIR_Coeffs_FFA0(engine, handle, coeffVec_float):
    # Coefficients are written to chip in 12-bit format
    coeffVec = coeffs_float2hex(coeffVec_float, 12)
    coeffVec = coeffVec[0]
    __coefficients = (ct.c_uint16 * len(coeffVec))(*coeffVec)
    success = engine.teAudioSetAncIirFilter(handle, ct.c_uint16(ANC_INSTANCE_ANC0_ID), ct.c_uint16(ANC_PATH_FFA_ID), __coefficients, ct.c_uint16(15))
    # Set coefficients in the local data structure in 16-bit format for human-readable text file
    coeffVec = coeffs_float2hex(coeffVec_float, 16)
    coeffVec = coeffVec[0]
    setParam('OFFSET_ANC_FF_A_DEN_COEFF0_L', coeffVec[0])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF1_L', coeffVec[1])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF2_L', coeffVec[2])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF3_L', coeffVec[3])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF4_L', coeffVec[4])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF5_L', coeffVec[5])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF6_L', coeffVec[6])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF0_L', coeffVec[7])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF1_L', coeffVec[8])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF2_L', coeffVec[9])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF3_L', coeffVec[10])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF4_L', coeffVec[11])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF5_L', coeffVec[12])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF6_L', coeffVec[13])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF7_L', coeffVec[14])
    if success == 1:
        print('ANC IIR filters set')
    else:
        raise ValueError('Error when attempting to set IIR parameters')
    return success


def SetIIR_Coeffs_FFA1(engine, handle, coeffVec_float):
    # Coefficients are written to chip in 12-bit format
    coeffVec = coeffs_float2hex(coeffVec_float, 12)
    coeffVec = coeffVec[0]
    __coefficients = (ct.c_uint16 * len(coeffVec))(*coeffVec)
    success = engine.teAudioSetAncIirFilter(handle, ct.c_uint16(ANC_INSTANCE_ANC1_ID), ct.c_uint16(ANC_PATH_FFA_ID), __coefficients, ct.c_uint16(15))
    # Set coefficients in the local data structure in 16-bit format for human-readable text file
    coeffVec = coeffs_float2hex(coeffVec_float, 16)
    coeffVec = coeffVec[0]
    setParam('OFFSET_ANC_FF_A_DEN_COEFF0_R', coeffVec[0])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF1_R', coeffVec[1])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF2_R', coeffVec[2])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF3_R', coeffVec[3])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF4_R', coeffVec[4])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF5_R', coeffVec[5])
    setParam('OFFSET_ANC_FF_A_DEN_COEFF6_R', coeffVec[6])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF0_R', coeffVec[7])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF1_R', coeffVec[8])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF2_R', coeffVec[9])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF3_R', coeffVec[10])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF4_R', coeffVec[11])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF5_R', coeffVec[12])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF6_R', coeffVec[13])
    setParam('OFFSET_ANC_FF_A_NUM_COEFF7_R', coeffVec[14])
    if success == 1:
        print('ANC IIR filters set')
    else:
        raise ValueError('Error when attempting to set IIR parameters')
    return success


def SetIIR_Coeffs_FFB0(engine, handle, coeffVec_float):
    # Coefficients are written to chip in 12-bit format
    coeffVec = coeffs_float2hex(coeffVec_float, 12)
    coeffVec = coeffVec[0]
    __coefficients = (ct.c_uint16 * len(coeffVec))(*coeffVec)
    success = engine.teAudioSetAncIirFilter(handle, ct.c_uint16(ANC_INSTANCE_ANC0_ID), ct.c_uint16(ANC_PATH_FFB_ID), __coefficients, ct.c_uint16(15))
    # Set coefficients in the local data structure in 16-bit format for human-readable text file
    coeffVec = coeffs_float2hex(coeffVec_float, 16)
    coeffVec = coeffVec[0]
    setParam('OFFSET_ANC_FF_B_DEN_COEFF0_L', coeffVec[0])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF1_L', coeffVec[1])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF2_L', coeffVec[2])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF3_L', coeffVec[3])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF4_L', coeffVec[4])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF5_L', coeffVec[5])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF6_L', coeffVec[6])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF0_L', coeffVec[7])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF1_L', coeffVec[8])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF2_L', coeffVec[9])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF3_L', coeffVec[10])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF4_L', coeffVec[11])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF5_L', coeffVec[12])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF6_L', coeffVec[13])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF7_L', coeffVec[14])
    if success == 1:
        print('ANC IIR filters set')
    else:
        raise ValueError('Error when attempting to set IIR parameters')
    return success


def SetIIR_Coeffs_FFB1(engine, handle, coeffVec_float):
    # Coefficients are written to chip in 12-bit format
    coeffVec = coeffs_float2hex(coeffVec_float, 12)
    coeffVec = coeffVec[0]
    __coefficients = (ct.c_uint16 * len(coeffVec))(*coeffVec)
    success = engine.teAudioSetAncIirFilter(handle, ct.c_uint16(ANC_INSTANCE_ANC1_ID), ct.c_uint16(ANC_PATH_FFB_ID), __coefficients, ct.c_uint16(15))
    # Set coefficients in the local data structure in 16-bit format for human-readable text file
    coeffVec = coeffs_float2hex(coeffVec_float, 16)
    coeffVec = coeffVec[0]
    setParam('OFFSET_ANC_FF_B_DEN_COEFF0_R', coeffVec[0])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF1_R', coeffVec[1])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF2_R', coeffVec[2])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF3_R', coeffVec[3])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF4_R', coeffVec[4])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF5_R', coeffVec[5])
    setParam('OFFSET_ANC_FF_B_DEN_COEFF6_R', coeffVec[6])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF0_R', coeffVec[7])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF1_R', coeffVec[8])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF2_R', coeffVec[9])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF3_R', coeffVec[10])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF4_R', coeffVec[11])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF5_R', coeffVec[12])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF6_R', coeffVec[13])
    setParam('OFFSET_ANC_FF_B_NUM_COEFF7_R', coeffVec[14])
    if success == 1:
        print('ANC IIR filters set')
    else:
        raise ValueError('Error when attempting to set IIR parameters')
    return success


def SetIIR_Coeffs_FB0(engine, handle, coeffVec_float):
    # Coefficients are written to chip in 12-bit format
    coeffVec = coeffs_float2hex(coeffVec_float, 12)
    coeffVec = coeffVec[0]
    __coefficients = (ct.c_uint16 * len(coeffVec))(*coeffVec)
    success = engine.teAudioSetAncIirFilter(handle, ct.c_uint16(ANC_INSTANCE_ANC0_ID), ct.c_uint16(ANC_PATH_FB_ID), __coefficients, ct.c_uint16(15))
    # Set coefficients in the local data structure in 16-bit format for human-readable text file
    coeffVec = coeffs_float2hex(coeffVec_float, 16)
    coeffVec = coeffVec[0]
    setParam('OFFSET_ANC_FB_DEN_COEFF0_L', coeffVec[0])
    setParam('OFFSET_ANC_FB_DEN_COEFF1_L', coeffVec[1])
    setParam('OFFSET_ANC_FB_DEN_COEFF2_L', coeffVec[2])
    setParam('OFFSET_ANC_FB_DEN_COEFF3_L', coeffVec[3])
    setParam('OFFSET_ANC_FB_DEN_COEFF4_L', coeffVec[4])
    setParam('OFFSET_ANC_FB_DEN_COEFF5_L', coeffVec[5])
    setParam('OFFSET_ANC_FB_DEN_COEFF6_L', coeffVec[6])
    setParam('OFFSET_ANC_FB_NUM_COEFF0_L', coeffVec[7])
    setParam('OFFSET_ANC_FB_NUM_COEFF1_L', coeffVec[8])
    setParam('OFFSET_ANC_FB_NUM_COEFF2_L', coeffVec[9])
    setParam('OFFSET_ANC_FB_NUM_COEFF3_L', coeffVec[10])
    setParam('OFFSET_ANC_FB_NUM_COEFF4_L', coeffVec[11])
    setParam('OFFSET_ANC_FB_NUM_COEFF5_L', coeffVec[12])
    setParam('OFFSET_ANC_FB_NUM_COEFF6_L', coeffVec[13])
    setParam('OFFSET_ANC_FB_NUM_COEFF7_L', coeffVec[14])
    if success == 1:
        print('ANC IIR filters set')
    else:
        raise ValueError('Error when attempting to set IIR parameters')
    return success


def SetIIR_Coeffs_FB1(engine, handle, coeffVec_float):
    # Coefficients are written to chip in 12-bit format
    coeffVec = coeffs_float2hex(coeffVec_float, 12)
    coeffVec = coeffVec[0]
    __coefficients = (ct.c_uint16 * len(coeffVec))(*coeffVec)
    success = engine.teAudioSetAncIirFilter(handle, ct.c_uint16(ANC_INSTANCE_ANC1_ID), ct.c_uint16(ANC_PATH_FB_ID), __coefficients, ct.c_uint16(15))
    # Set coefficients in the local data structure in 16-bit format for human-readable text file
    coeffVec = coeffs_float2hex(coeffVec_float, 16)
    coeffVec = coeffVec[0]
    setParam('OFFSET_ANC_FB_DEN_COEFF0_R', coeffVec[0])
    setParam('OFFSET_ANC_FB_DEN_COEFF1_R', coeffVec[1])
    setParam('OFFSET_ANC_FB_DEN_COEFF2_R', coeffVec[2])
    setParam('OFFSET_ANC_FB_DEN_COEFF3_R', coeffVec[3])
    setParam('OFFSET_ANC_FB_DEN_COEFF4_R', coeffVec[4])
    setParam('OFFSET_ANC_FB_DEN_COEFF5_R', coeffVec[5])
    setParam('OFFSET_ANC_FB_DEN_COEFF6_R', coeffVec[6])
    setParam('OFFSET_ANC_FB_NUM_COEFF0_R', coeffVec[7])
    setParam('OFFSET_ANC_FB_NUM_COEFF1_R', coeffVec[8])
    setParam('OFFSET_ANC_FB_NUM_COEFF2_R', coeffVec[9])
    setParam('OFFSET_ANC_FB_NUM_COEFF3_R', coeffVec[10])
    setParam('OFFSET_ANC_FB_NUM_COEFF4_R', coeffVec[11])
    setParam('OFFSET_ANC_FB_NUM_COEFF5_R', coeffVec[12])
    setParam('OFFSET_ANC_FB_NUM_COEFF6_R', coeffVec[13])
    setParam('OFFSET_ANC_FB_NUM_COEFF7_R', coeffVec[14])
    if success == 1:
        print('ANC IIR filters set')
    else:
        raise ValueError('Error when attempting to set IIR parameters')
    return success


#########################################################
###### External functions. Okay to be used in scripts.
#########################################################


#################################################
############ Miscellaneous functions ############
#################################################

def readHTF(htf_file, key=ANC_PSKEY_ACTIVE):
    '''
    read data from HTF file
    '''
    # scan trhough HTF file and pull out the line with persistence data
    success = False
    with open(htf_file) as f:
        for line in f:
            if line.strip().startswith(format(key, '#08x')):
                print('ANC key data found!')
                htfdata = line.split()
                success = True
    if not(success):
        raise ValueError('ANC parameters not found in HTF file for key ' + format(key, '#08x'))
    data = []
    htfdata = htfdata[3:-1] # htfdata should be a list that starts with elements '0x204100', '=', '[', '01', '10', '00', '00', 'B8', '00', '00', etc...
    if len(htfdata) == ANC_DATA_NUM_BYTES*2:
        for i in range(0, ANC_DATA_NUM_BYTES*2, 2):
            data.append(int((htfdata[i+1] + htfdata[i]),16))
    else:
        print('htfdata len' + str(len(htfdata)))
        raise ValueError('Data length mismatch in HTF file')

    for count in range(0, ANC_DATA_NUM_BYTES):
        #print('Setting param ' + param + ' from HTF as ' + hex(data[count]))
        psdata[count] = data[count]

    print('Number of parameters from HTF file ' + str(len(htfdata)))
    print('Total number of ANC parameters ' + str(len(ANC_PARAM_LIST)))
    return htfdata, data


def writeHTF(filename='saved_ANC_params.htf', key=ANC_PSKEY_ACTIVE):
    '''
    write data to HTF file
    '''
    htfdata = []
    for param in psdata:
        hex_string = '{:04X}'.format(param)
        htfdata.append(hex_string[2:4])
        htfdata.append(hex_string[0:2])
    htf_line = format(key, '#08x') + ' = [ ' + ' '.join(htfdata) + ' ]'
    f = open(filename, 'w')
    f.write('file = audio\n')
    f.write(htf_line)
    f.close()
    return htfdata


def readAudioKey(engine, handle, key=ANC_PSKEY_ACTIVE):
    '''
    read ANC parameters from audio persistence
    '''
    print('Reading ANC parameters from device')
    success, readlen = engine.tePsAudioRead(handle, key, ANC_DATA_NUM_BYTES, psdata)
    print('read length ' + str(readlen))
    print('ANC_PARAM_LIST ' + str(len(ANC_PARAM_LIST)))
    print('persistence data length ' + str(len(psdata)))
    if success == 1:
        print('ANC parameters read from device')
    else:
        raise ValueError('Error reading pskey from device')


def writeAudioKey(engine, handle, key=ANC_PSKEY_ACTIVE):
    '''
    write ANC parameters to audio persistence
    '''
    print('Writing ANC parameters to device')
    success = engine.tePsAudioWrite(handle, key, ANC_DATA_NUM_BYTES, psdata)
    if success == 1:
        print('ANC parameters written to device')
    else:
        print('Error writing to device')
    return success


def displayAudioKeyData():
    '''
    display the parameters
    '''
    index = 3
    for param in ANC_PARAM_LIST:
        paramvalue = hex(getParam(param))
        print(param + ' = ' + paramvalue)
        index = index + 2


def setParam(param, value):
    '''
    update parameter value in local psdata array
    '''
    index = (ANC_PARAM_LIST.index(param)*2) + OFFSET_PSDATA_HEADER
    psdata[index] = value >> 16
    psdata[index+1] = value & 0x0000FFFF


def getParam(param):
    '''
    retrieve specific parameter from local psdata array
    '''
    index = (ANC_PARAM_LIST.index(param)*2) + OFFSET_PSDATA_HEADER
    value = psdata[index+1] + (psdata[index] << 16)
    return value


def updatePsdata():
    '''
    merge updated values in GAINS into psdata, in preperation for writing to device
    '''
    #left gain parameters
    setParam('OFFSET_ANC_FF_A_GAIN_L', GAINS['FFA0_gain'])
    setParam('OFFSET_ANC_FF_A_SHIFT_L', GAINS['FFA0_shift'])
    setParam('OFFSET_ANC_FF_B_GAIN_L', GAINS['FFB0_gain'])
    setParam('OFFSET_ANC_FF_B_SHIFT_L', GAINS['FFB0_shift'])
    setParam('OFFSET_ANC_FB_GAIN_L', GAINS['FB0_gain'])
    setParam('OFFSET_ANC_FB_SHIFT_L', GAINS['FB0_shift'])
    #right gain parameters
    setParam('OFFSET_ANC_FF_A_GAIN_R', GAINS['FFA1_gain'])
    setParam('OFFSET_ANC_FF_A_SHIFT_R', GAINS['FFA1_shift'])
    setParam('OFFSET_ANC_FF_B_GAIN_R', GAINS['FFB1_gain'])
    setParam('OFFSET_ANC_FF_B_SHIFT_R', GAINS['FFB1_shift'])
    setParam('OFFSET_ANC_FB_GAIN_R', GAINS['FB1_gain'])
    setParam('OFFSET_ANC_FB_SHIFT_R', GAINS['FB1_shift'])


def updateGains():
    '''
    update GAINS with values from psdata, used to initialize GAINS after reading from device
    '''
    #left gain parameters
    GAINS['FFA0_gain'] = getParam('OFFSET_ANC_FF_A_GAIN_L')
    GAINS['FFA0_shift'] = getParam('OFFSET_ANC_FF_A_SHIFT_L')
    GAINS['FFB0_gain'] = getParam('OFFSET_ANC_FF_B_GAIN_L')
    GAINS['FFB0_shift'] = getParam('OFFSET_ANC_FF_B_SHIFT_L')
    GAINS['FB0_gain'] = getParam('OFFSET_ANC_FB_GAIN_L')
    GAINS['FB0_shift'] = getParam('OFFSET_ANC_FB_SHIFT_L')
    #right gain parameters
    GAINS['FFA1_gain'] = getParam('OFFSET_ANC_FF_A_GAIN_R')
    GAINS['FFA1_shift'] = getParam('OFFSET_ANC_FF_A_SHIFT_R')
    GAINS['FFB1_gain'] = getParam('OFFSET_ANC_FF_B_GAIN_R')
    GAINS['FFB1_shift'] = getParam('OFFSET_ANC_FF_B_SHIFT_R')
    GAINS['FB1_gain'] = getParam('OFFSET_ANC_FB_GAIN_R')
    GAINS['FB1_shift'] = getParam('OFFSET_ANC_FB_SHIFT_R')
    #dB calc
    GAINS['FFA0_gain_dB'] = AncGainConvertToDb(GAINS['FFA0_shift'], GAINS['FFA0_gain'])
    GAINS['FFB0_gain_dB'] = AncGainConvertToDb(GAINS['FFB0_shift'], GAINS['FFB0_gain'])
    GAINS['FB0_gain_dB'] = AncGainConvertToDb(GAINS['FB0_shift'], GAINS['FB0_gain'])
    GAINS['FFA1_gain_dB'] = AncGainConvertToDb(GAINS['FFA1_shift'], GAINS['FFA1_gain'])
    GAINS['FFB1_gain_dB'] = AncGainConvertToDb(GAINS['FFB1_shift'], GAINS['FFB1_gain'])
    GAINS['FB1_gain_dB'] = AncGainConvertToDb(GAINS['FB1_shift'], GAINS['FB1_gain'])


def getChipName(engine, handle):
    '''
    Query chip name
    '''
    success, name = engine.teGetChipDisplayName(handle, ct.c_uint32(255))
    if success == 1:
        print('Device name: ' + name)
    else:
        raise ValueError('Could not query chip name')
    return name

def updateStorage(filename, data):
    '''
    update STORAGE temp file with current gain
    '''
    try:
        f=open(filename,'w')
        f.write(str(data))
    except:
        print "File Doesn't Exist"
        f=open(filename, 'w')
        f.write(str(data))
        f.close()
        return -1
        exit()

    f.close()
    return 1

def readfromStorage(filename):
    '''
    returns saved gain values from STORAGE file as integers
    '''
    try:
        f=open(filename, 'r')
    except:
        print filename + " Doesn't Exist"
        return -1
        exit()

    data = []
    for line in f:
        data.append(float(line))

    f.close()
    return data


def PrintGAINS():
    '''
    Output pertinent Hybrid Headset Gains
    '''
    print('FFA0 = ' + str(GAINS['FFA0_gain_dB']))
    print('FFB0 = ' + str(GAINS['FFB0_gain_dB']))
    print('FB0 = ' + str(GAINS['FB0_gain_dB']))
    print('FFA1 = ' + str(GAINS['FFA1_gain_dB']))
    print('FFB1 = ' + str(GAINS['FFB1_gain_dB']))
    print('FB1 = ' + str(GAINS['FB1_gain_dB']))
    return


def removeFile(filename):
    import os

    try:
        f=open(filename,'r')
    except:
        print(filename + " File Doesn't Exist")
        return

    f.close()
    os.remove(filename)
    print(filename + " Removed")

    return


def cv2Gains(cv_list):
    GAINS['FFA0_gain_dB'] = cv_list[0]
    GAINS['FFB0_gain_dB'] = cv_list[1]
    GAINS['FB0_gain_dB'] = cv_list[2]

    GAINS['FFA0_shift'], GAINS['FFA0_gain'] = AncGainConvertFromdB(GAINS['FFA0_gain_dB'])
    GAINS['FFB0_shift'], GAINS['FFB0_gain'] = AncGainConvertFromdB(GAINS['FFB0_gain_dB'])
    GAINS['FB0_shift'], GAINS['FB0_gain'] = AncGainConvertFromdB(GAINS['FB0_gain_dB'])

    return


def translateAFEGain(dB):
    #Translate gain in dB to U4.5 representation
    #dB is integer or float input, 16bit
    den_bits = 5
    denom = pow(2, -den_bits)

    #Test for max
    if dB > 16:
        return 0x1FF
        exit

    int_dB = int(dB)
    frac_dB = dB - int_dB
    fractional = int(round(frac_dB / denom))

    return int_dB<<5 | fractional


def SetAFEGain(engine, handle, gainDB, anc_mic_path):
    #engine
    #handle
    #gainDB is 0 to 42 in 3dB steps, (i.e. 0x0 is 0dB, 0x2 = +6dB)
    #anc_mic_path is mic path to set the AFE Gain on
    DIG_GAIN_0 = 0x8020
    gainINT = translateAFEGain(float(gainDB))

    start_time = timeit.default_timer()
    print('Setting AFE gain on ' + anc_mic_path.name + ' to ' + str(gainDB) + ' dB')
    streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
    audioConfigure(engine, handle, streamID, STREAM_CODEC_RAW_INPUT_GAIN, gainINT | DIG_GAIN_0)

    print('Operation completed in ' + str((timeit.default_timer() - start_time)) + ' seconds')


def SetAFEGainDebug(engine, handle, gainDB, anc_mic_path):
    #STREAM_CODEC_INPUT_GAIN = 0x0302
    #STREAM_CODEC_RAW_INPUT_GAIN = 0x0304

    #gainINT = int(gainDB/3)
    DIG_GAIN_0 = 0x8020
    a_gain = translateAFEGain(float(gainDB))

    denom = pow(2, -5)
    int_part = a_gain>>5
    dec_part = (a_gain & 0x1F) * denom
    checked_value = (a_gain>>5) + ((a_gain & 0x1F) * denom)

    gain = int(a_gain)<<16 | DIG_GAIN_0
    start_time = timeit.default_timer()
    print('Setting AFE gain on ' + anc_mic_path.name + ' to ' + str(checked_value) + ' dB')
    streamID = getSourceID(engine, handle, anc_mic_path.device, anc_mic_path.instance, anc_mic_path.channel)
    audioConfigure(engine, handle, streamID, STREAM_CODEC_RAW_INPUT_GAIN, gain)

    print('Operation completed in ' + str((timeit.default_timer() - start_time)) + ' seconds')




def coeffs_float2hex(coeffs, Nbits):
    np.asarray(coeffs)
    Q_num = 3;
    regSize = Nbits;
    NegShift = 2**regSize
    FracOne = 2**(regSize-Q_num)

    tmp_inds = [ii for ii, e in enumerate(coeffs) if e == 2**(Q_num-1)]
    tmp_vals = np.zeros(len(coeffs))
    tmp_vals[tmp_inds] = 1
    coeffs_tmp  = coeffs - tmp_vals/FracOne

    tmp_inds2 = [ii for ii, e in enumerate(coeffs) if e < 0]
    tmp_vals2 = np.zeros(len(coeffs))
    tmp_vals2[tmp_inds2] = 1
    coeffs_tmp2 = NegShift*tmp_vals2 + FracOne*coeffs_tmp

    coeffs_rounded = np.zeros(len(coeffs))
    coeffs_hex = len(coeffs_rounded)*['null']
    coeffs_rounded_int = len(coeffs_rounded)*['null']
    for ii in range(0,len(coeffs_tmp2)):
        coeffs_rounded[ii] = round(coeffs_tmp2[ii])
        coeffs_rounded_int[ii] = int(coeffs_rounded[ii])    #convert to integer
        coeffs_hex[ii] = str(hex(coeffs_rounded[ii]))
        if coeffs_hex[ii][-1] == "L":
            coeffs_hex[ii] = coeffs_hex[ii][:-1]

    coeffs_rounded = coeffs_rounded.tolist()

    return coeffs_rounded_int , coeffs_hex


#################################################
############ Standardizing functions ############
#################################################

def EnableANC(engine, handle, enableFlag):
    """
    # Enables both ANC channels, for all paths (FFa, FFb, FB)
    # bits: ANC | FB | FFb | FFa
    """
    if enableFlag:
        anc0_enable = 0b1111
        anc1_enable = 0b1111
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',1)
        setParam('OFFSET_FF_A_ENABLE_L',1)
        setParam('OFFSET_FF_B_ENABLE_R',1)
    else:
        anc0_enable = 0b0
        anc1_enable = 0b0
        setParam('OFFSET_FF_A_ENABLE_L',0)
        setParam('OFFSET_FF_B_ENABLE_R',0)
        setParam('OFFSET_FF_A_ENABLE_L',0)
        setParam('OFFSET_FF_B_ENABLE_R',0)

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)
    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFa(engine, handle, enableFlag):
    """
    # Enables both ANC channels, for FFa. Keep state of other Enables.
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1001 | anc0CurrentEnableBits
        anc1_enable = 0b1001 | anc1CurrentEnableBits

    else:
        anc0_enable = 0b1110 & anc0CurrentEnableBits
        anc1_enable = 0b1110 & anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFb(engine, handle, enableFlag):
    """
    # Enables both ANC channels, for FFb
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1010 | anc0CurrentEnableBits
        anc1_enable = 0b1010 | anc1CurrentEnableBits
    else:
        anc0_enable = 0b1101 & anc0CurrentEnableBits
        anc1_enable = 0b1101 & anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FB(engine, handle, enableFlag):
    """
    # Enables both ANC channels, for FB
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1100 | anc0CurrentEnableBits
        anc1_enable = 0b1100 | anc1CurrentEnableBits
    else:
        anc0_enable = 0b1011 & anc0CurrentEnableBits
        anc1_enable = 0b1011 & anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFa0(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for FFa
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1001 | anc0CurrentEnableBits
        anc1_enable = anc1CurrentEnableBits
    else:
        anc0_enable = 0b1110 & anc0CurrentEnableBits
        anc1_enable = anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFb0(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for FFb
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1010 | anc0CurrentEnableBits
        anc1_enable = anc1CurrentEnableBits
    else:
        anc0_enable = 0b1101 & anc0CurrentEnableBits
        anc1_enable = anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FB0(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for FB
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1100 | anc0CurrentEnableBits
        anc1_enable = anc1CurrentEnableBits
    else:
        anc0_enable = 0b1011 & anc0CurrentEnableBits
        anc1_enable = anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFa1(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for FFa
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1001 | anc1CurrentEnableBits
    else:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1110 & anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFb1(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for FFb
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1010 | anc1CurrentEnableBits
    else:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1101 & anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FB1(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for FB
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1100 | anc1CurrentEnableBits
    else:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1011 & anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_ANC0(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for all paths (FFa, FFb, FB)
    # bits: ANC | FB | FFb | FFa
    """
    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = 0b1111
        anc1_enable = anc1CurrentEnableBits
    else:
        anc0_enable = 0b1000
        anc1_enable = anc1CurrentEnableBits

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_ANC1(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for all paths (FFa, FFb, FB)
    # bits: ANC | FB | FFb | FFa
    """

    anc0CurrentEnableBits, anc1CurrentEnableBits = readCurrentEnableState(psdataTemp)
    if enableFlag:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1111
    else:
        anc0_enable = anc0CurrentEnableBits
        anc1_enable = 0b1000

    updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable)

    EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)


def EnableANC_FFaLeft(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for FFa
    """
    EnableANC_FFa0(engine, handle, enableFlag)


def EnableANC_FFbLeft(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for FFb
    """
    EnableANC_FFb0(engine, handle, enableFlag)


def EnableANC_FBLeft(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for FB
    """
    EnableANC_FB0(engine, handle, enableFlag)


def EnableANC_FFaRight(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for FFa
    """
    EnableANC_FFa1(engine, handle, enableFlag)


def EnableANC_FFbRight(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for FFb
    """
    EnableANC_FFb1(engine, handle, enableFlag)


def EnableANC_FBRight(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for FB
    """
    EnableANC_FB1(engine, handle, enableFlag)


def EnableANC_ANCLeft(engine, handle, enableFlag):
    """
    # Enables ANC0 channel, for all paths (FFa, FFb, FB)
    """
    EnableANC_ANC0(engine, handle, enableFlag)


def EnableANC_ANCRight(engine, handle, enableFlag):
    """
    # Enables ANC1 channel, for all paths (FFa, FFb, FB)
    """
    EnableANC_ANC1(engine, handle, enableFlag)


def EnableANC_HYMono(engine, handle, enableFlag):
    """
    # Enables FFa0, FFb0, FB0
    """
    EnableANC_ANC0(engine, handle, enableFlag)


def EnableANC_FFMono(engine, handle, enableFlag):
    """
    # Enables FFa0
    """
    EnableANC_FFa0(engine, handle, enableFlag)


def EnableANC_FBMono(engine, handle, enableFlag):
    """
    # Enables FFa0
    """
    EnableANC_FFa0(engine, handle, enableFlag)


def EnableANC_ECMono(engine, handle, enableFlag):
    """
    # Enables FFa0, FB0
    """
    EnableANC_FFa0(engine, handle, enableFlag)
    EnableANC_FB0(engine, handle, enableFlag)


def EnableANC_HYStereo(engine, handle, enableFlag):
    """
    # Enables FFa0, FFb0, FB0, FFa1, FFb1, FB1
    """
    EnableANC(engine, handle, enableFlag)


def EnableANC_FFStereo(engine, handle, enableFlag):
    """
    # Enables FFa0, FFa1
    """
    EnableANC_FFa(engine, handle, enableFlag)


def EnableANC_FBStereo(engine, handle, enableFlag):
    """
    # Enables FFa0, FFa1
    """
    EnableANC_FFa(engine, handle, enableFlag)


def EnableANC_ECStereo(engine, handle, enableFlag):
    """
    # Enables FFa0, FB0, FFa1, FB1
    """
    EnableANC_FBStereo(engine, handle, enableFlag)
    EnableANC_FB(engine, handle, enableFlag)


def DisableANC_FFa(engine, handle):
    """
    # Disables both ANC channels, for FFa
    """
    EnableANC_FFa(engine, handle, 0)


def DisableANC_FFb(engine, handle):
    """
    # Disables both ANC channels, for FFb
    """
    EnableANC_FFb(engine, handle, 0)


def DisableANC_FB(engine, handle):
    """
    # Disables both ANC channels, for FB
    """
    EnableANC_FB(engine, handle, 0)


def DisableANC_FFa0(engine, handle):
    """
    # Disables ANC0 channel, for FFa
    """
    EnableANC_FFa0(engine, handle, 0)


def DisableANC_FFb0(engine, handle):
    """
    # Disables ANC0 channel, for FFb
    """
    EnableANC_FFb0(engine, handle, 0)


def DisableANC_FB0(engine, handle):
    """
    # Disables ANC0 channel, for FB
    """
    EnableANC_FB0(engine, handle, 0)


def DisableANC_FFa1(engine, handle):
    """
    # Disables ANC1 channel, for FFa
    """
    EnableANC_FFa1(engine, handle, 0)


def DisableANC_FFb1(engine, handle):
    """
    # Disables ANC1 channel, for FFb
    """
    EnableANC_FFb1(engine, handle, 0)

def DisableANC_FB1(engine, handle):
    """
    # Disables ANC1 channel, for FB
    """
    EnableANC_FB1(engine, handle, 0)


def DisableANC_ANC0(engine, handle):
    """
    # Disables ANC0 channel, for all paths (FFa, FFb, FB)
    """
    EnableANC_ANC0(engine, handle, 0)


def DisableANC_ANC1(engine, handle):
    """
    # Disables ANC1 channel, for all paths (FFa, FFb, FB)
    """
    EnableANC_ANC1(engine, handle, 0)


def DisableANC_FFaLeft(engine, handle):
    """
    # Disables ANC0 channel, for FFa
    """
    EnableANC_FFa0(engine, handle, 0)


def DisableANC_FFbLeft(engine, handle):
    """
    # Disables ANC0 channel, for FFb
    """
    EnableANC_FFb0(engine, handle, 0)


def DisableANC_FBLeft(engine, handle):
    """
    # Disables ANC0 channel, for FB
    """
    EnableANC_FB0(engine, handle, 0)


def DisableANC_FFaRight(engine, handle):
    """
    # Disables ANC1 channel, for FFa
    """
    EnableANC_FFa1(engine, handle, 0)


def DisableANC_FFbRight(engine, handle):
    """
    # Disables ANC1 channel, for FFb
    """
    EnableANC_FFb1(engine, handle, 0)


def DisableANC_FBRight(engine, handle):
    """
    # Disables ANC1 channel, for FB
    """
    EnableANC_FB1(engine, handle, 0)


def DisableANC_ANCLeft(engine, handle):
    """
    # Disables ANC0 channel, for all paths (FFa, FFb, FB)
    """
    EnableANC_ANC0(engine, handle, 0)


def DisableANC_ANCRight(engine, handle):
    """
    # Disables ANC1 channel, for all paths (FFa, FFb, FB)
    """
    EnableANC_ANC1(engine, handle, 0)


def DisableANC_HYMono(engine, handle):
    """
    # Disables FFa0, FFb0, FB0
    """
    EnableANC_ANC0(engine, handle, 0)


def DisableANC_FFMono(engine, handle):
    """
    # Disables FFa0
    """
    EnableANC_FFa0(engine, handle, 0)


def DisableANC_FBMono(engine, handle):
    """
    # Disables FFa0
    """
    EnableANC_FFa0(engine, handle, 0)


def DisableANC_ECMono(engine, handle):
    """
    # Disables FFa0, FB0
    """
    EnableANC_FB0(engine, handle, 0)


def DisableANC_HYStereo(engine, handle):
    """
    # Disables FFa0, FFb0, FB0, FFa1, FFb1, FB1
    """
    EnableANC(engine, handle, 0)


def DisableANC_FFStereo(engine, handle):
    """
    # Disables FFa0, FFa1
    """
    EnableANC_FFa(engine, handle, 0)


def DisableANC_FBStereo(engine, handle):
    """
    # Disables FFa0, FFa1
    """
    EnableANC_FFa(engine, handle, 0)


def DisableANC_ECStereo(engine, handle):
    """
    # Disables FFa0, FB0, FFa1, FB1
    """
    EnableANC_FB(engine, handle, 0)


'''
###################
################### Enable individual blocks functions ################
###################
'''

def EnableDCFilter(engine, handle, enableFlag):
    """
    # Enables DC Filter for all ANC paths (FFa0, FFa1, FFb0, FFb1)
    """
    dcFilter_Enable(engine, handle, FFA0, enableFlag)
    dcFilter_Enable(engine, handle, FFB0, enableFlag)
    dcFilter_Enable(engine, handle, FFA1, enableFlag)
    dcFilter_Enable(engine, handle, FFB1, enableFlag)


def EnableDCFilter_FFa(engine, handle, enableFlag):
    """
    # Enables DC Filter for all FFa ANC paths (FFa0, FFa1)
    """
    dcFilter_Enable(engine, handle, FFA0, enableFlag)
    dcFilter_Enable(engine, handle, FFA1, enableFlag)


def EnableDCFilter_FFb(engine, handle, enableFlag):
    """
    # Enables DC Filter for all FFb ANC paths (FFb0, FFb1)
    """
    dcFilter_Enable(engine, handle, FFB0, enableFlag)
    dcFilter_Enable(engine, handle, FFB1, enableFlag)


def EnableDCFilter_FFa0(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFa0 path
    """
    dcFilter_Enable(engine, handle, FFA0, enableFlag)


def EnableDCFilter_FFb0(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFb0 path
    """
    dcFilter_Enable(engine, handle, FFB0, enableFlag)


def EnableDCFilter_FFa1(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFa1 path
    """
    dcFilter_Enable(engine, handle, FFA1, enableFlag)


def EnableDCFilter_FFb1(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFb1 path
    """
    dcFilter_Enable(engine, handle, FFB1, enableFlag)


def EnableDCFilter_Left(engine, handle, enableFlag):
    """
    # Enables DC Filter for all ANC0 paths (FFa0, FFb0)
    """
    dcFilter_Enable(engine, handle, FFA0, enableFlag)
    dcFilter_Enable(engine, handle, FFB0, enableFlag)


def EnableDCFilter_Right(engine, handle, enableFlag):
    """
    # Enables DC Filter for all ANC1 paths (FFa0, FFb0)
    """
    dcFilter_Enable(engine, handle, FFA1, enableFlag)
    dcFilter_Enable(engine, handle, FFB1, enableFlag)


def EnableDCFilter_FFaLeft(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFa0 path
    """
    dcFilter_Enable(engine, handle, FFA0, enableFlag)


def EnableDCFilter_FFbLeft(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFb0 path
    """
    dcFilter_Enable(engine, handle, FFB0, enableFlag)


def EnableDCFilter_FFaRight(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFa1 path
    """
    dcFilter_Enable(engine, handle, FFA1, enableFlag)


def EnableDCFilter_FFbRight(engine, handle, enableFlag):
    """
    # Enables DC Filter for FFb1 path
    """
    dcFilter_Enable(engine, handle, FFB1, enableFlag)


def EnableSMLPF(engine, handle, enableFlag):
    """
    # Enables SMLPF block for all paths (ANC0, ANC1)
    """
    smLPFEnable(engine, handle, FFA0, enableFlag)
    smLPFEnable(engine, handle, FFA1, enableFlag)


def EnableSMLPF_ANC0(engine, handle, enableFlag):
    """
    # Enables SMLPF block for ANC0 path
    """
    smLPFEnable(engine, handle, FFA0, enableFlag)


def EnableSMLPF_ANC1(engine, handle, enableFlag):
    """
    # Enables SMLPF block for ANC1 path
    """
    smLPFEnable(engine, handle, FFA1, enableFlag)


def EnableSMLPF_Left(engine, handle, enableFlag):
    """
    # Enables SMLPF block for ANC0 path
    """
    EnableSMLPF_ANC0(engine, handle, enableFlag)


def EnableSMLPF_Right(engine, handle, enableFlag):
    """
    # Enables SMLPF block for ANC1 path
    """
    EnableSMLPF_ANC1(engine, handle, enableFlag)


'''
###################
################### DC filter cutoff frequency functions ################
###################
'''

def SetDCFilter_Shift(engine, handle, shift):
    """
    #set DC Filter frequency for both ANC0 and ANC1
    """
    dcFilter_Shift(engine, handle, FFA0, shift)
    dcFilter_Shift(engine, handle, FFB0, shift)
    dcFilter_Shift(engine, handle, FFA1, shift)
    dcFilter_Shift(engine, handle, FFB1, shift)


def SetDCFilter_Shift_FFa0(engine, handle, shift):
    """
    #set DC Filter frequency for FFa0
    """
    dcFilter_Shift(engine, handle, FFA0, shift)


def SetDCFilter_Shift_FFb0(engine, handle, shift):
    """
    #set DC Filter frequency for FFb0
    """
    dcFilter_Shift(engine, handle, FFB0, shift)


def SetDCFilter_Shift_FFa1(engine, handle, shift):
    """
    #set DC Filter frequency for FFa1
    """
    dcFilter_Shift(engine, handle, FFA1, shift)


def SetDCFilter_Shift_FFb1(engine, handle, shift):
    """
    #set DC Filter frequency for FFb1
    """
    dcFilter_Shift(engine, handle, FFB1, shift)


def SetDCFilter_Shift_FFaLeft(engine, handle, shift):
    """
    #set DC Filter frequency for FFa0
    """
    SetDCFilter_Shift_FFa0(engine, handle, shift)


def SetDCFilter_Shift_FFbLeft(engine, handle, shift):
    """
    #set DC Filter frequency for FFb0
    """
    SetDCFilter_Shift_FFb0(engine, handle, shift)


def SetDCFilter_Shift_FFaRight(engine, handle, shift):
    """
    #set DC Filter frequency for FFa1
    """
    SetDCFilter_Shift_FFa1(engine, handle, shift)


def SetDCFilter_Shift_FFbRight(engine, handle, shift):
    """
    #set DC Filter frequency for FFb1
    """
    SetDCFilter_Shift_FFb1(engine, handle, shift)


def SetDCFilter_Shift_ANC0(engine, handle, shift):
    """
    #set DC Filter frequency for ANC0
    """
    dcFilter_Shift(engine, handle, FFA0, shift)
    dcFilter_Shift(engine, handle, FFB0, shift)


def SetDCFilter_Shift_ANC1(engine, handle, shift):
    """
    #set DC Filter frequency for ANC1
    """
    dcFilter_Shift(engine, handle, FFA1, shift)
    dcFilter_Shift(engine, handle, FFB1, shift)


def SetDCFilter_Shift_Left(engine, handle, shift):
    """
    #set DC Filter frequency for ANC0
    """
    SetDCFilter_Shift_ANC0(engine, handle, shift)


def SetDCFilter_Shift_Right(engine, handle, shift):
    """
    #set DC Filter frequency for ANC1
    """
    SetDCFilter_Shift_ANC1(engine, handle, shift)


'''
###################
################### smLPF cutoff frequency functions ################
###################
'''

def SetSMLPF_Shift(engine, handle, shift):
    """
    #set SMLPF frequency for ANC0 and ANC1
    """
    smLPFShift(engine, handle, FFA0, shift)
    smLPFShift(engine, handle, FFA1, shift)


def SetSMLPF_Shift_ANC0(engine, handle, shift):
    """
    #set SMLPF frequency for ANC0
    """
    smLPFShift(engine, handle, FFA0, shift)


def SetSMLPF_Shift_ANC1(engine, handle, shift):
    """
    #set SMLPF frequency for ANC1
    """
    smLPFShift(engine, handle, FFA1, shift)


def SetSMLPF_Shift_Left(engine, handle, shift):
    """
    #set SMLPF frequency for ANC0
    """
    smLPFShift(engine, handle, FFA0, shift)


def SetSMLPF_Shift_Right(engine, handle, shift):
    """
    #set SMLPF frequency for ANC1
    """
    smLPFShift(engine, handle, FFA1, shift)


'''
###################
################### LPF cutoff frequency functions ################
###################
'''

def SetLPF_Shift(engine, handle, LPF1, LPF2):
    """
    #set LPF frequency for ANC0 and ANC1
    """
    SetLPF(engine, handle, FFA0, LPF1, LPF2)
    SetLPF(engine, handle, FFB0, LPF1, LPF2)
    SetLPF(engine, handle, FFA1, LPF1, LPF2)
    SetLPF(engine, handle, FFB1, LPF1, LPF2)


def SetLPF_Shift_FFa0(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFa0
    """
    SetLPF(engine, handle, FFA0, shift1, shift2)


def SetLPF_Shift_FFb0(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFb0
    """
    SetLPF(engine, handle, FFB0, shift1, shift2)


def SetLPF_Shift_FB0(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FB0
    """
    SetLPF(engine, handle, FB0, shift1, shift2)


def SetLPF_Shift_FFa1(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFa1
    """
    SetLPF(engine, handle, FFA1, shift1, shift2)


def SetLPF_Shift_FFb1(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFb1
    """
    SetLPF(engine, handle, FFB1, shift1, shift2)


def SetLPF_Shift_FB1(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FB0
    """
    SetLPF(engine, handle, FB1, shift1, shift2)


def SetLPF_Shift_FFaLeft(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFa0
    """
    SetLPF(engine, handle, FFA0, shift1, shift2)


def SetLPF_Shift_FFbLeft(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFb0
    """
    SetLPF(engine, handle, FFB0, shift1, shift2)


def SetLPF_Shift_FFaRight(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFa1
    """
    SetLPF(engine, handle, FFA1, shift1, shift2)


def SetLPF_Shift_FFbRight(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FFb1
    """
    SetLPF(engine, handle, FFB1, shift1, shift2)


def SetLPF_Shift_FBLeft(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FB0
    """
    SetLPF(engine, handle, FB0, shift1, shift2)


def SetLPF_Shift_FBRight(engine, handle, shift1, shift2):
    """
    #set LPF frequency for FB1
    """
    SetLPF(engine, handle, FB1, shift1, shift2)

'''
###################
################### Gain (Shift) functions ################
###################
'''

def SetGain_Shift_FFa0(engine, handle, gainShift):
    """
    # Set shift value for FFa0
    """
    LPFSetShiftGain(engine, handle, gainShift, FFA0)

def SetGain_Shift_FFb0(engine, handle, gainShift):
    """
    # Set shift value for FFb0
    """
    LPFSetShiftGain(engine, handle, gainShift, FFB0)

def SetGain_Shift_FB0(engine, handle, gainShift):
    """
    # Set shift value for FB0
    """
    LPFSetShiftGain(engine, handle, gainShift, FB0)

def SetGain_Shift_FFa1(engine, handle, gainShift):
    """
    # Set shift value for FFa1
    """
    LPFSetShiftGain(engine, handle, gainShift, FFA1)

def SetGain_Shift_FFb1(engine, handle, gainShift):
    """
    # Set shift value for FFb1
    """
    LPFSetShiftGain(engine, handle, gainShift, FFB1)

def SetGain_Shift_FB1(engine, handle, gainShift):
    """
    # Set shift value for FB1
    """
    LPFSetShiftGain(engine, handle, gainShift, FB1)

def SetGain_Shift_FFaLeft(engine, handle, gainShift):
    """
    # Set shift value for FFa0
    """
    SetGain_Shift_FFa0(engine, handle, gainShift)

def SetGain_Shift_FFbLeft(engine, handle, gainShift):
    """
    # Set shift value for FFb0
    """
    SetGain_Shift_FFb0(engine, handle, gainShift)

def SetGain_Shift_FBLeft(engine, handle, gainShift):
    """
    # Set shift value for FB0
    """
    SetGain_Shift_FB0(engine, handle, gainShift)

def SetGain_Shift_FFaRight(engine, handle, gainShift):
    """
    # Set shift value for FFa1
    """
    SetGain_Shift_FFa1(engine, handle, gainShift)

def SetGain_Shift_FFbRight(engine, handle, gainShift):
    """
    # Set shift value for FFb1
    """
    SetGain_Shift_FFb1(engine, handle, gainShift)

def SetGain_Shift_FBRight(engine, handle, gainShift):
    """
    # Set shift value for FB1
    """
    SetGain_Shift_FB1(engine, handle, gainShift)

'''
###################
################### Gain (Fine) functions ################
###################
'''

def SetGain_Fine_FFa0(engine, handle, gainFine):
    """
    # Set fine value for FFa0
    """
    LPFSetFineGain(engine, handle, gainFine, FFA0)

def SetGain_Fine_FFb0(engine, handle, gainFine):
    """
    # Set fine value for FFb0
    """
    LPFSetFineGain(engine, handle, gainFine, FFB0)

def SetGain_Fine_FB0(engine, handle, gainFine):
    """
    # Set fine value for FB0
    """
    LPFSetFineGain(engine, handle, gainFine, FB0)

def SetGain_Fine_FFa1(engine, handle, gainFine):
    """
    # Set fine value for FFa1
    """
    LPFSetFineGain(engine, handle, gainFine, FFA1)

def SetGain_Fine_FFb1(engine, handle, gainFine):
    """
    # Set fine value for FFb1
    """
    LPFSetFineGain(engine, handle, gainFine, FFB1)

def SetGain_Fine_FB1(engine, handle, gainFine):
    """
    # Set fine value for FB1
    """
    LPFSetFineGain(engine, handle, gainFine, FB1)

def SetGain_Fine_FFaLeft(engine, handle, gainFine):
    """
    # Set fine value for FFa0
    """
    SetGain_Fine_FFa0(engine, handle, gainFine)

def SetGain_Fine_FFbLeft(engine, handle, gainFine):
    """
    # Set fine value for FFb0
    """
    SetGain_Fine_FFb0(engine, handle, gainFine)

def SetGain_Fine_FBLeft(engine, handle, gainFine):
    """
    # Set fine value for FB0
    """
    SetGain_Fine_FB0(engine, handle, gainFine)

def SetGain_Fine_FFaRight(engine, handle, gainFine):
    """
    # Set fine value for FFa1
    """
    SetGain_Fine_FFa1(engine, handle, gainFine)

def SetGain_Fine_FFbRight(engine, handle, gainFine):
    """
    # Set fine value for FFb1
    """
    SetGain_Fine_FFb1(engine, handle, gainFine)

def SetGain_Fine_FBRight(engine, handle, gainFine):
    """
    # Set fine value for FB1
    """
    SetGain_Fine_FB1(engine, handle, gainFine)

'''
###################
################### Gain dB functions ################
###################
'''

def SetGain_dB_FFa0(engine, handle, gainDB):
    """
    # Set gain in dB for FFa0
    """
    AncSetGain(engine, handle, gainDB, FFA0)

def SetGain_dB_FFb0(engine, handle, gainDB):
    """
    # Set gain in dB for FFb0
    """
    AncSetGain(engine, handle, gainDB, FFB0)

def SetGain_dB_FB0(engine, handle, gainDB):
    """
    # Set gain in dB for FB0
    """
    AncSetGain(engine, handle, gainDB, FB0)

def SetGain_dB_FFa1(engine, handle, gainDB):
    """
    # Set gain in dB for FFa1
    """
    AncSetGain(engine, handle, gainDB, FFA1)

def SetGain_dB_FFb1(engine, handle, gainDB):
    """
    # Set gain in dB for FFb1
    """
    AncSetGain(engine, handle, gainDB, FFB1)

def SetGain_dB_FB1(engine, handle, gainDB):
    """
    # Set gain in dB for FB1
    """
    AncSetGain(engine, handle, gainDB, FB1)

def SetGain_dB_FFaLeft(engine, handle, gainDB):
    """
    # Set gain in dB for FFa0
    """
    SetGain_dB_FFa0(engine, handle, gainDB)

def SetGain_dB_FFbLeft(engine, handle, gainDB):
    """
    # Set gain in dB for FFb0
    """
    SetGain_dB_FFb0(engine, handle, gainDB)

def SetGain_dB_FBLeft(engine, handle, gainDB):
    """
    # Set gain in dB for FF0
    """
    SetGain_dB_FB0(engine, handle, gainDB)

def SetGain_dB_FFaRight(engine, handle, gainDB):
    """
    # Set gain in dB for FFa1
    """
    SetGain_dB_FFa1(engine, handle, gainDB)

def SetGain_dB_FFbRight(engine, handle, gainDB):
    """
    """
    # Set gain in dB for FFb1
    SetGain_dB_FFb1(engine, handle, gainDB)

def SetGain_dB_FBRight(engine, handle, gainDB):
    """
    # Set gain in dB for FB1
    """
    SetGain_dB_FB1(engine, handle, gainDB)

'''
###################
################### Gain (Coarse, Fine) functions ################
###################
'''

def SetGain_CoarseFine_FFa0(engine, handle, gainShift, gainFine):
    SetGain_Shift_FFa0(engine, handle, gainShift)
    SetGain_Fine_FFa0(engine, handle, gainFine)

def SetGain_CoarseFine_FFb0(engine, handle, gainShift, gainFine):
    SetGain_Shift_FFb0(engine, handle, gainShift)
    SetGain_Fine_FFb0(engine, handle, gainFine)

def SetGain_CoarseFine_FB0(engine, handle, gainShift, gainFine):
    SetGain_Shift_FB0(engine, handle, gainShift)
    SetGain_Fine_FB0(engine, handle, gainFine)

def SetGain_CoarseFine_FFa1(engine, handle, gainShift, gainFine):
    SetGain_Shift_FFa1(engine, handle, gainShift)
    SetGain_Fine_FFa1(engine, handle, gainFine)

def SetGain_CoarseFine_FFb1(engine, handle, gainShift, gainFine):
    SetGain_Shift_FFb1(engine, handle, gainShift)
    SetGain_Fine_FFb1(engine, handle, gainFine)

def SetGain_CoarseFine_FB1(engine, handle, gainShift, gainFine):
    SetGain_Shift_FB1(engine, handle, gainShift)
    SetGain_Fine_FB1(engine, handle, gainFine)

'''
###################
################### IIR Filter functions ################
###################
'''

def SetIIRFilter_Coeffs_FFa0(engine, handle, coeffs):
    """
    # Set IIR filter for FFa0
    """
    SetIIR_Coeffs_FFA0(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFb0(engine, handle, coeffs):
    """
    # Set IIR filter for FFb0
    """
    SetIIR_Coeffs_FFB0(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FB0(engine, handle, coeffs):
    """
    # Set IIR filter for FB0
    """
    SetIIR_Coeffs_FB0(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFa1(engine, handle, coeffs):
    """
    # Set IIR filter for FFa1
    """
    SetIIR_Coeffs_FFA1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFb1(engine, handle, coeffs):
    """
    # Set IIR filter for FFb1
    """
    SetIIR_Coeffs_FFB1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FB1(engine, handle, coeffs):
    """
    # Set IIR filter for FB1
    """
    SetIIR_Coeffs_FB1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFaLeft(engine, handle, coeffs):
    """
    # Set IIR filter for FFa0
    """
    SetIIR_Coeffs_FFA0(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFbLeft(engine, handle, coeffs):
    """
    # Set IIR filter for FFb0
    """
    SetIIR_Coeffs_FFB1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FBLeft(engine, handle, coeffs):
    """
    # Set IIR filter for FB0
    """
    SetIIR_Coeffs_FB1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFaRight(engine, handle, coeffs):
    """
    # Set IIR filter for FFa1
    """
    SetIIR_Coeffs_FFA1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FFbRight(engine, handle, coeffs):
    """
    # Set IIR filter for FFb1
    """
    SetIIR_Coeffs_FFB1(engine, handle, coeffs)

def SetIIRFilter_Coeffs_FBRight(engine, handle, coeffs):
    """
    # Set IIR filter for FB1
    """
    SetIIR_Coeffs_FB1(engine, handle, coeffs)

def ANCConnectDUT( BluesuitePath, TRANSPORT):
    """
    Connects to the Device Under Test using the provided transport
    Inputs:
        BluesuitePath: Location for the ADK/BlueSuite files
        Transport: TRANSPORT_USBDBG = 256, TRANSPORT_TRB = 128
    Outputs:
        Engine: Pointer to TestEngine DLL
        handle: Handle to device
    """
    engine = TestEngine(BluesuitePath)
    handle = connectToDevice(engine, TRANSPORT)
    getChipName(engine, handle)
    return engine, handle

'''
Temporary parameter structure
'''

def setTempParam(psdataTemp, param, value):
    '''
    update parameter value in local psdata array
    '''
    index = (ANC_PARAM_LIST.index(param))
    psdataTemp[index] = value

def getTempParam(psdataTemp, param):
    '''
    retrieve specific parameter from local psdata array
    '''
    index = (ANC_PARAM_LIST.index(param))
    value = psdataTemp[index]
    return value

def copyParamArray(engine, handle):
    global psdataTemp
    readAudioKey(engine, handle)
    psdataTemp = len(ANC_PARAM_LIST) * [0]
    for i in xrange(len(ANC_PARAM_LIST)):
        psdataTemp[i] = getParam(ANC_PARAM_LIST[i])

    return psdataTemp

def readCurrentEnableState(psdataTemp):
    '''
    # bits: ANC | FB | FFb | FFa
    '''
    '''
    anc0 =        getTempParam(psdataTemp, 'OFFSET_FFA_IN_ENABLE_L') << 0;
    anc0 = anc0 | getTempParam(psdataTemp, 'OFFSET_FFB_IN_ENABLE_L') << 1
    anc0 = anc0 | getTempParam(psdataTemp, 'OFFSET_FB_ENABLE_L') << 2
    anc0 = anc0 | getTempParam(psdataTemp, 'OFFSET_FF_OUT_ENABLE_L') << 3

    anc1 =        getTempParam(psdataTemp, 'OFFSET_FFA_IN_ENABLE_R') << 0;
    anc1 = anc1 | getTempParam(psdataTemp, 'OFFSET_FFB_IN_ENABLE_R') << 1
    anc1 = anc1 | getTempParam(psdataTemp, 'OFFSET_FB_ENABLE_R') << 2
    anc1 = anc1 | getTempParam(psdataTemp, 'OFFSET_FF_OUT_ENABLE_R') << 3
    '''
    anc0 =        getParam('OFFSET_FF_A_ENABLE_L') << 0;
    anc0 = anc0 | getParam('OFFSET_FF_B_ENABLE_L') << 1
    anc0 = anc0 | getParam('OFFSET_FB_ENABLE_L') << 2
    anc0 = anc0 | getParam('OFFSET_FF_OUT_ENABLE_L') << 3

    anc1 =        getParam('OFFSET_FF_A_ENABLE_R') << 0;
    anc1 = anc1 | getParam('OFFSET_FF_B_ENABLE_R') << 1
    anc1 = anc1 | getParam('OFFSET_FB_ENABLE_R') << 2
    anc1 = anc1 | getParam('OFFSET_FF_OUT_ENABLE_R') << 3

    return anc0, anc1

def updateCurrentEnableState(psdataTemp, anc0_enable, anc1_enable):
    setTempParam(psdataTemp, 'OFFSET_FF_A_ENABLE_L', int(anc0_enable & 0b0001 >= 1))
    setTempParam(psdataTemp, 'OFFSET_FF_B_ENABLE_L', int(anc0_enable & 0b0010 >= 1))
    setTempParam(psdataTemp, 'OFFSET_FB_ENABLE_L', int(anc0_enable & 0b0100 >= 1))
    setTempParam(psdataTemp, 'OFFSET_FF_OUT_ENABLE_L', int(anc0_enable & 0b1000 >= 1))

    setTempParam(psdataTemp, 'OFFSET_FF_A_ENABLE_R', int(anc1_enable & 0b0001 >= 1))
    setTempParam(psdataTemp, 'OFFSET_FF_B_ENABLE_R', int(anc1_enable & 0b0010 >= 1))
    setTempParam(psdataTemp, 'OFFSET_FB_ENABLE_R', int(anc1_enable & 0b0100 >= 1))
    setTempParam(psdataTemp, 'OFFSET_FF_OUT_ENABLE_R', int(anc1_enable & 0b1000 >= 1))

    setParam('OFFSET_FF_A_ENABLE_L', int(anc0_enable & 0b0001 >= 1))
    setParam('OFFSET_FF_B_ENABLE_L', int(anc0_enable & 0b0010 >= 1))
    setParam('OFFSET_FB_ENABLE_L', int(anc0_enable & 0b0100 >= 1))
    setParam('OFFSET_FF_OUT_ENABLE_L', int(anc0_enable & 0b1000 >= 1))

    setParam('OFFSET_FF_A_ENABLE_R', int(anc1_enable & 0b0001 >= 1))
    setParam('OFFSET_FF_B_ENABLE_R', int(anc1_enable & 0b0010 >= 1))
    setParam('OFFSET_FB_ENABLE_R', int(anc1_enable & 0b0100 >= 1))
    setParam('OFFSET_FF_OUT_ENABLE_R', int(anc1_enable & 0b1000 >= 1))


'''
pull the ANC params from a specific filter location and save to human readable text file and .htf
'''
def pullANCparams(engine, handle, filename, filter_number):
    # must use readAudioKey() just before calling this
    TOTAL_NUM_FILTERS = 10
    if int(filter_number) <= TOTAL_NUM_FILTERS:
        key = 0x204100
        keyIndex = (int(filter_number) - 1) * 2
        key = key + keyIndex
        print 'Reading key', hex(key)

    else:
        print '%s is not a valid filter location. The valid range is 1-%d.' % ( filter_number, TOTAL_NUM_FILTERS )

    readAudioKey(engine, handle, key)
    registerList = ['']
    registerList.append('ANC_ENABLE_REGISTERS')
    registerList.append('  Left_FF     : 0x%08X' % (getParam('OFFSET_FF_B_ENABLE_L')))
    registerList.append('  Left_FB     : 0x%08X' % (getParam('OFFSET_FF_A_ENABLE_L')))
    registerList.append('  Left_EC     : 0x%08X' % (getParam('OFFSET_FB_ENABLE_L')))
    registerList.append('  Left_path   : 0x%08X' % (getParam('OFFSET_FF_OUT_ENABLE_L')))
    registerList.append('  Right_FF    : 0x%08X' % (getParam('OFFSET_FF_B_ENABLE_R')))
    registerList.append('  Right_FB    : 0x%08X' % (getParam('OFFSET_FF_A_ENABLE_R')))
    registerList.append('  Right_EC    : 0x%08X' % (getParam('OFFSET_FB_ENABLE_R')))
    registerList.append('  Right_path  : 0x%08X' % (getParam('OFFSET_FF_OUT_ENABLE_R')))

    registerList.append('')
    registerList.append('LEFT_FEEDFORWARD_(FF)_FILTER_BLOCK_PARAMETERS')
    registerList.append('Left_FF_DC_Block')
    registerList.append('  Enable      : 0x%08X' % (getParam('OFFSET_FF_B_DCFLT_ENABLE_L')))
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_FF_B_DCFLT_SHIFT_L')))
    registerList.append('Left_FF_Gain')
    registerList.append('  Course_gain : 0x%08X' % (getParam('OFFSET_ANC_FF_B_SHIFT_L')))
    registerList.append('  Fine_gain   : 0x%08X' % (getParam('OFFSET_ANC_FF_B_GAIN_L')))
    registerList.append('Left_FF_LPF')
    registerList.append('  Shift_0     : 0x%08X' % (getParam('OFFSET_ANC_FF_B_LPF_SHIFT0_L')))
    registerList.append('  Shift_1     : 0x%08X' % (getParam('OFFSET_ANC_FF_B_LPF_SHIFT1_L')))
    registerList.append('Left_FF_Coefficients')
    registerList.append('  DEN_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF0_L')))
    registerList.append('  DEN_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF1_L')))
    registerList.append('  DEN_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF2_L')))
    registerList.append('  DEN_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF3_L')))
    registerList.append('  DEN_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF4_L')))
    registerList.append('  DEN_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF5_L')))
    registerList.append('  DEN_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF6_L')))
    registerList.append('  NUM_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF0_L')))
    registerList.append('  NUM_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF1_L')))
    registerList.append('  NUM_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF2_L')))
    registerList.append('  NUM_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF3_L')))
    registerList.append('  NUM_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF4_L')))
    registerList.append('  NUM_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF5_L')))
    registerList.append('  NUM_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF6_L')))
    registerList.append('  NUM_COEFF7  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF7_L')))

    registerList.append('')
    registerList.append('LEFT_FEEDBACK_(FB)_FILTER_BLOCK_PARAMETERS')
    registerList.append('Left_smLPF')
    registerList.append('  Enable      : 0x%08X' % (getParam('OFFSET_SMLPF_ENABLE_L')))
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_SM_LPF_SHIFT_L')))
    registerList.append('Left_FB_DC_Block')
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_FF_A_DCFLT_ENABLE_L')))
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_FF_A_DCFLT_SHIFT_L')))
    registerList.append('Left_FB_Gain')
    registerList.append('  Course_gain : 0x%08X' % (getParam('OFFSET_ANC_FF_A_SHIFT_L')))
    registerList.append('  Fine_gain   : 0x%08X' % (getParam('OFFSET_ANC_FF_A_GAIN_L')))
    registerList.append('Left_FB_LPF')
    registerList.append('  Shift_0     : 0x%08X' % (getParam('OFFSET_ANC_FF_A_LPF_SHIFT0_L')))
    registerList.append('  Shift_1     : 0x%08X' % (getParam('OFFSET_ANC_FF_A_LPF_SHIFT1_L')))
    registerList.append('Left_FB_Coefficients')
    registerList.append('  DEN_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF0_L')))
    registerList.append('  DEN_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF1_L')))
    registerList.append('  DEN_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF2_L')))
    registerList.append('  DEN_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF3_L')))
    registerList.append('  DEN_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF4_L')))
    registerList.append('  DEN_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF5_L')))
    registerList.append('  DEN_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF6_L')))
    registerList.append('  NUM_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF0_L')))
    registerList.append('  NUM_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF1_L')))
    registerList.append('  NUM_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF2_L')))
    registerList.append('  NUM_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF3_L')))
    registerList.append('  NUM_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF4_L')))
    registerList.append('  NUM_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF5_L')))
    registerList.append('  NUM_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF6_L')))
    registerList.append('  NUM_COEFF7  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF7_L')))

    registerList.append('')
    registerList.append('LEFT_ECHO_CANCELLATION_(EC)_FILTER_BLOCK_PARAMETERS')
    registerList.append('Left_EC_Gain')
    registerList.append('  Course_gain : 0x%08X' % (getParam('OFFSET_ANC_FB_SHIFT_L')))
    registerList.append('  Fine_gain   : 0x%08X' % (getParam('OFFSET_ANC_FB_GAIN_L')))
    registerList.append('Left_EC_LPF')
    registerList.append('  Shift_0     : 0x%08X' % (getParam('OFFSET_ANC_FB_LPF_SHIFT0_L')))
    registerList.append('  Shift_1     : 0x%08X' % (getParam('OFFSET_ANC_FB_LPF_SHIFT1_L')))
    registerList.append('Left_EC_Coefficients')
    registerList.append('  DEN_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF0_L')))
    registerList.append('  DEN_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF1_L')))
    registerList.append('  DEN_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF2_L')))
    registerList.append('  DEN_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF3_L')))
    registerList.append('  DEN_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF4_L')))
    registerList.append('  DEN_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF5_L')))
    registerList.append('  DEN_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF6_L')))
    registerList.append('  NUM_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF0_L')))
    registerList.append('  NUM_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF1_L')))
    registerList.append('  NUM_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF2_L')))
    registerList.append('  NUM_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF3_L')))
    registerList.append('  NUM_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF4_L')))
    registerList.append('  NUM_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF5_L')))
    registerList.append('  NUM_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF6_L')))
    registerList.append('  NUM_COEFF7  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF7_L')))

    registerList.append('')
    registerList.append('RIGHT_FEEDFORWARD_(FF)_FILTER_BLOCK_PARAMETERS')
    registerList.append('Right_FF_DC_Block')
    registerList.append('  Enable      : 0x%08X' % (getParam('OFFSET_FF_B_DCFLT_ENABLE_R')))
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_FF_B_DCFLT_SHIFT_R')))
    registerList.append('Right_FF_Gain')
    registerList.append('  Course_gain : 0x%08X' % (getParam('OFFSET_ANC_FF_B_SHIFT_R')))
    registerList.append('  Fine_gain   : 0x%08X' % (getParam('OFFSET_ANC_FF_B_GAIN_R')))
    registerList.append('Right_FF_LPF')
    registerList.append('  Shift_0     : 0x%08X' % (getParam('OFFSET_ANC_FF_B_LPF_SHIFT0_R')))
    registerList.append('  Shift_1     : 0x%08X' % (getParam('OFFSET_ANC_FF_B_LPF_SHIFT1_R')))
    registerList.append('Right_FF_Coefficients')
    registerList.append('  DEN_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF0_R')))
    registerList.append('  DEN_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF1_R')))
    registerList.append('  DEN_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF2_R')))
    registerList.append('  DEN_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF3_R')))
    registerList.append('  DEN_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF4_R')))
    registerList.append('  DEN_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF5_R')))
    registerList.append('  DEN_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_DEN_COEFF6_R')))
    registerList.append('  NUM_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF0_R')))
    registerList.append('  NUM_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF1_R')))
    registerList.append('  NUM_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF2_R')))
    registerList.append('  NUM_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF3_R')))
    registerList.append('  NUM_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF4_R')))
    registerList.append('  NUM_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF5_R')))
    registerList.append('  NUM_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF6_R')))
    registerList.append('  NUM_COEFF7  : 0x%08X' % (getParam('OFFSET_ANC_FF_B_NUM_COEFF7_R')))

    registerList.append('')
    registerList.append('RIGHT_FEEDBACK_(FB)_FILTER_BLOCK_PARAMETERS')
    registerList.append('Right_smLPF')
    registerList.append('  Enable      : 0x%08X' % (getParam('OFFSET_SMLPF_ENABLE_R')))
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_SM_LPF_SHIFT_R')))
    registerList.append('Right_FB_DC_Block')
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_FF_A_DCFLT_ENABLE_R')))
    registerList.append('  Shift       : 0x%08X' % (getParam('OFFSET_FF_A_DCFLT_SHIFT_R')))
    registerList.append('Right_FB_Gain')
    registerList.append('  Course_gain : 0x%08X' % (getParam('OFFSET_ANC_FF_A_SHIFT_R')))
    registerList.append('  Fine_gain   : 0x%08X' % (getParam('OFFSET_ANC_FF_A_GAIN_R')))
    registerList.append('Right_FB_LPF')
    registerList.append('  Shift_0     : 0x%08X' % (getParam('OFFSET_ANC_FF_A_LPF_SHIFT0_R')))
    registerList.append('  Shift_1     : 0x%08X' % (getParam('OFFSET_ANC_FF_A_LPF_SHIFT1_R')))
    registerList.append('Right_FB_Coefficients')
    registerList.append('  DEN_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF0_R')))
    registerList.append('  DEN_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF1_R')))
    registerList.append('  DEN_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF2_R')))
    registerList.append('  DEN_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF3_R')))
    registerList.append('  DEN_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF4_R')))
    registerList.append('  DEN_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF5_R')))
    registerList.append('  DEN_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_DEN_COEFF6_R')))
    registerList.append('  NUM_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF0_R')))
    registerList.append('  NUM_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF1_R')))
    registerList.append('  NUM_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF2_R')))
    registerList.append('  NUM_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF3_R')))
    registerList.append('  NUM_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF4_R')))
    registerList.append('  NUM_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF5_R')))
    registerList.append('  NUM_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF6_R')))
    registerList.append('  NUM_COEFF7  : 0x%08X' % (getParam('OFFSET_ANC_FF_A_NUM_COEFF7_R')))

    registerList.append('')
    registerList.append('RIGHT_ECHO_CANCELLATION_(EC)_FILTER_BLOCK_PARAMETERS')
    registerList.append('Right_EC_Gain')
    registerList.append('  Course_gain : 0x%08X' % (getParam('OFFSET_ANC_FB_SHIFT_R')))
    registerList.append('  Fine_gain   : 0x%08X' % (getParam('OFFSET_ANC_FB_GAIN_R')))
    registerList.append('Right_EC_LPF')
    registerList.append('  Shift_0     : 0x%08X' % (getParam('OFFSET_ANC_FB_LPF_SHIFT0_R')))
    registerList.append('  Shift_1     : 0x%08X' % (getParam('OFFSET_ANC_FB_LPF_SHIFT1_R')))
    registerList.append('Right_EC_Coefficients')
    registerList.append('  DEN_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF0_R')))
    registerList.append('  DEN_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF1_R')))
    registerList.append('  DEN_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF2_R')))
    registerList.append('  DEN_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF3_R')))
    registerList.append('  DEN_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF4_R')))
    registerList.append('  DEN_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF5_R')))
    registerList.append('  DEN_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FB_DEN_COEFF6_R')))
    registerList.append('  NUM_COEFF0  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF0_R')))
    registerList.append('  NUM_COEFF1  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF1_R')))
    registerList.append('  NUM_COEFF2  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF2_R')))
    registerList.append('  NUM_COEFF3  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF3_R')))
    registerList.append('  NUM_COEFF4  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF4_R')))
    registerList.append('  NUM_COEFF5  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF5_R')))
    registerList.append('  NUM_COEFF6  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF6_R')))
    registerList.append('  NUM_COEFF7  : 0x%08X' % (getParam('OFFSET_ANC_FB_NUM_COEFF7_R')))

    print 'Creating file', filename
    fileOut = open(filename+'.txt','w')
    for listEntry in registerList:
        fileOut.write(listEntry + '\n')
    fileOut.close()
    writeHTF(filename+'.htf', key)


'''
Push a human-readable filter file to the chip and save .htf
'''
def pushANCparams(engine, handle, filename, filter_number):
    TOTAL_NUM_FILTERS = 10
    if int(filter_number) <= TOTAL_NUM_FILTERS:
        key = 0x204100
        keyIndex = (int(filter_number) - 1) * 2
        key = key + keyIndex
        print 'Reading key',hex(key)

    else:
        print '%s is not a valid filter location. The valid range is 1-%d.' % ( filter_number, TOTAL_NUM_FILTERS )

    def read_line_from_file():
        return int(str(next(fileIn,'').split(':',1)[1]).strip(),16)

    fileIn = open(filename+'.txt', 'r')

    readAudioKey(engine, handle, key)
    for line in fileIn:
	    if 'ANC_ENABLE_REGISTERS' in line:
	        setParam('OFFSET_FF_B_ENABLE_L',   read_line_from_file())
	        setParam('OFFSET_FF_A_ENABLE_L',   read_line_from_file())
	        setParam('OFFSET_FB_ENABLE_L',     read_line_from_file())
	        setParam('OFFSET_FF_OUT_ENABLE_L', read_line_from_file())
	        setParam('OFFSET_FF_B_ENABLE_L',   read_line_from_file())
	        setParam('OFFSET_FF_A_ENABLE_L',   read_line_from_file())
	        setParam('OFFSET_FB_ENABLE_L',     read_line_from_file())
	        setParam('OFFSET_FF_OUT_ENABLE_L', read_line_from_file())

	    if 'LEFT_FEEDFORWARD_(FF)_FILTER_BLOCK_PARAMETERS' in line:
	        next(fileIn,'')
	        setParam('OFFSET_FF_B_DCFLT_ENABLE_L', read_line_from_file())
	        setParam('OFFSET_FF_B_DCFLT_SHIFT_L',  read_line_from_file())
	        next(fileIn,'')
	        GAINS['FFB0_shift'] = read_line_from_file()
	        GAINS['FFB0_gain']  = read_line_from_file()
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_B_LPF_SHIFT0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_LPF_SHIFT1_L', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF1_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF2_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF3_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF4_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF5_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF6_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF1_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF2_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF3_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF4_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF5_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF6_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF7_L', read_line_from_file())

	    if 'LEFT_FEEDBACK_(FB)_FILTER_BLOCK_PARAMETERS' in line:
	        next(fileIn,'')
	        setParam('OFFSET_SMLPF_ENABLE_L', read_line_from_file())
	        setParam('OFFSET_SM_LPF_SHIFT_L', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_FF_A_DCFLT_ENABLE_L', read_line_from_file())
	        setParam('OFFSET_FF_A_DCFLT_SHIFT_L',  read_line_from_file())
	        next(fileIn,'')
	        GAINS['FFA0_shift'] = read_line_from_file()
	        GAINS['FFA0_gain']  = read_line_from_file()
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_A_LPF_SHIFT0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_LPF_SHIFT1_L', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF1_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF2_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF3_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF4_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF5_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF6_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF1_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF2_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF3_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF4_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF5_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF6_L', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF7_L', read_line_from_file())

	    if 'LEFT_ECHO_CANCELLATION_(EC)_FILTER_BLOCK_PARAMETERS' in line:
	        next(fileIn,'')
	        GAINS['FB0_shift'] = read_line_from_file()
	        GAINS['FB0_gain']  = read_line_from_file()
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FB_LPF_SHIFT0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_LPF_SHIFT1_L', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FB_DEN_COEFF0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF1_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF2_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF3_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF4_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF5_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF6_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF0_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF1_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF2_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF3_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF4_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF5_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF6_L', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF7_L', read_line_from_file())

	    if 'RIGHT_FEEDFORWARD_(FF)_FILTER_BLOCK_PARAMETERS' in line:
	        next(fileIn,'')
	        setParam('OFFSET_FF_B_DCFLT_ENABLE_R', read_line_from_file())
	        setParam('OFFSET_FF_B_DCFLT_SHIFT_R',  read_line_from_file())
	        next(fileIn,'')
	        GAINS['FFB1_shift'] = read_line_from_file()
	        GAINS['FFB1_gain']  = read_line_from_file()
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_B_LPF_SHIFT0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_LPF_SHIFT1_R', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF1_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF2_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF3_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF4_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF5_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_DEN_COEFF6_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF1_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF2_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF3_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF4_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF5_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF6_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_B_NUM_COEFF7_R', read_line_from_file())

	    if 'RIGHT_FEEDBACK_(FB)_FILTER_BLOCK_PARAMETERS' in line:
	        next(fileIn,'')
	        setParam('OFFSET_SMLPF_ENABLE_R', read_line_from_file())
	        setParam('OFFSET_SM_LPF_SHIFT_R', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_FF_A_DCFLT_ENABLE_R', read_line_from_file())
	        setParam('OFFSET_FF_A_DCFLT_SHIFT_R',  read_line_from_file())
	        next(fileIn,'')
	        GAINS['FFA1_shift'] = read_line_from_file()
	        GAINS['FFA1_gain']  = read_line_from_file()
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_A_LPF_SHIFT0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_LPF_SHIFT1_R', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF1_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF2_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF3_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF4_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF5_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_DEN_COEFF6_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF1_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF2_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF3_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF4_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF5_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF6_R', read_line_from_file())
	        setParam('OFFSET_ANC_FF_A_NUM_COEFF7_R', read_line_from_file())

	    if 'RIGHT_ECHO_CANCELLATION_(EC)_FILTER_BLOCK_PARAMETERS' in line:
	        next(fileIn,'')
	        GAINS['FB1_shift'] = read_line_from_file()
	        GAINS['FB1_gain']  = read_line_from_file()
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FB_LPF_SHIFT0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_LPF_SHIFT1_R', read_line_from_file())
	        next(fileIn,'')
	        setParam('OFFSET_ANC_FB_DEN_COEFF0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF1_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF2_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF3_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF4_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF5_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_DEN_COEFF6_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF0_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF1_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF2_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF3_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF4_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF5_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF6_R', read_line_from_file())
	        setParam('OFFSET_ANC_FB_NUM_COEFF7_R', read_line_from_file())
	        print 'Num0 Coeff:', getParam('OFFSET_ANC_FB_NUM_COEFF0_R')

    fileIn.close()
    updatePsdata()
    writeAudioKey(engine, handle, key)
    writeHTF(filename+'.htf')


def saveANCparams(engine, handle, filename, filter_number):
    TOTAL_NUM_FILTERS = 10
    if int(filter_number) <= TOTAL_NUM_FILTERS:
        key = 0x204100
        keyIndex = (int(filter_number) - 1) * 2
        key = key + keyIndex
        print 'Reading key',hex(key)

    else:
        print '%s is not a valid filter location. The valid range is 1-%d.' % ( filter_number, TOTAL_NUM_FILTERS )

    writeAudioKey(engine, handle, key)
    pullANCparams(engine, handle, filename, filter_number)



################ Main ################
if __name__ == '__main__':

    # current file path of latest version of BlueSuite
    BluesuitePath = r'C:\qtil\ADK_QCC512x_QCC302x_WIN_6.3.0.154\tools\bin'
    #TRANSPORT_USBDBG = 256
    #TRANSPORT_TRB = 128
    engine, handle = ANCConnectDUT( BluesuitePath, TRANSPORT_USBDBG)

    readAudioKey(engine, handle, ANC_PSKEY_ACTIVE) # read ANC parameters from device into psdata cache
    psdataTemp = copyParamArray(engine, handle);
    updateGains()   # update local gains cache with values from psdata
    displayAudioKeyData() # display parameters in psdata cache

    print('place test code here to adjust gain values')
    # example calls to gain adjustment API shown below
    SetGain_dB_FFa0(engine, handle, 5)
    SetGain_dB_FFa1(engine, handle, 12)
    #SetGain_dB_FB0(engine, handle, -3)

    updatePsdata() # write data from gains cache into psdata
    writeAudioKey(engine, handle, ANC_PSKEY_ACTIVE) # write psdata cache to device
    writeHTF() # save HTF file

    # close connection to device
    engine.closeTestEngine(handle)

    '''
    ANC on off examples

    EnableANC(engine, handle, 1)
    EnableANC(engine, handle, 0)
    DisableANC(engine, handle)
    EnableANC_ANCLeft(engine, handle, 1)
    EnableANC_ANCFFa0(engine, handle, 0)

    SetGain_dB_FFa0(engine, handle, -6)
    SetGain_Fine_FFa1(engine, handle, 101)
    SetGain_Shift_FFa0(engine, handle, -1)

    coeffs = [0,0,0,0,0,0,0,0,0x200,0,0,0,0,0,0]
    SetIIRFilter_Coeffs_FFa1(engine, handle, coeffs)
    psdataTemp = copyParamArray(engine, handle);
    getTempParam(psdataTemp, 'OFFSET_FFA_IN_ENABLE_L')
    '''
