################################################################################
#
#  TestEngine_ANC_API.py
#
#  Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.
#  All Rights Reserved.
#  Qualcomm Technologies International, Ltd. Confidential and Proprietary.
#
#  This API is an example for using Testengine.dll to calibrate ANC gains on
#  the production line.
#
################################################################################

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
    print('Operation completed in ' + str((timeit.default_timer() - start_time)) + ' seconds')
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


def AncDisable(engine, handle):
    '''
    disable ANC path
    '''
    success = engine.teAudioStreamAncEnable(handle, ct.c_uint16(0), ct.c_uint16(0))
    if success == 1:
        print('ANC disabled')
    else:
        raise ValueError('Error when attempting to disable ANC')
    return success


def AncEnable(engine, handle, anc_left_enable, anc_right_enable):
    '''
    enable ANC path
    '''
    print('Enabling ANC with the following bit fields for left and right')
    print(anc_left_enable)
    print(anc_right_enable)
    success = engine.teAudioStreamAncEnable(handle, ct.c_uint16(anc_left_enable), ct.c_uint16(anc_right_enable))
    if success == 1:
        print('ANC enabled')
    else:
        raise ValueError('Error when attempting to enable ANC')
    return success


def AncEnableFF(engine, handle):
    AncSetGainFFA0(engine,handle, 'mute') # mute FB path
    anc_left_enable = 0b1111
    anc_right_enable = 0
    AncEnable(engine, handle, anc_left_enable, anc_right_enable)
    

def AncEnableFB(engine, handle):
    anc_left_enable = 0b1001
    anc_right_enable = 0
    AncSetGainFFA0(engine, handle, GAINS['FFA0_gain_dB']) # restore FB path gain in case it was muted
    AncEnable(engine, handle, anc_left_enable, anc_right_enable)


def AncEnableFBEC(engine, handle):
    anc_left_enable = 0b1101
    anc_right_enable = 0
    AncSetGainFFA0(engine, handle, GAINS['FFA0_gain_dB'])
    AncEnable(engine, handle, anc_left_enable, anc_right_enable)


def AncEnableHY(engine, handle):
    anc_left_enable = 0b1111
    anc_right_enable = 0
    AncSetGainFFA0(engine, handle, GAINS['FFA0_gain_dB'])
    AncEnable(engine, handle, anc_left_enable, anc_right_enable)


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
    return shift, int(gain)


def AncGainConvertToDb(shift, gain):
    '''
    LPF gain block inside the ANC block
    convert from shift and gain values to dB
    shift - LPF gain shift value (0-15)
    gain - LPF gain value (0-255)
    '''
    print('Converting to dB: shift of ' + str(shift) + ' and gain of ' + str(gain))
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


def AncSetGain(engine, handle, gainDB, anc_path):
    start_time = timeit.default_timer()
    print('Setting gain on ' + anc_path.name + ' to ' + str(gainDB) + ' dB')
    streamID = getSourceID(engine, handle, anc_path.device, anc_path.instance, anc_path.channel)
    gainOk, shiftOk, gainVal = ConfigureStreams(engine, handle, streamID, anc_path.stream_gain, anc_path.stream_gain_shift, gainDB)
    if type(gainDB) is not str: # don't save gain when muting
        SaveGains(gainOk, shiftOk, gainVal, gainDB, anc_path.name)

    print('Operation completed in ' + str((timeit.default_timer() - start_time)) + ' seconds')


def AncSetGainFFA0(engine, handle, gainDB):
    '''
    write shift and gain values to FFA0 filter path
    '''
    AncSetGain(engine, handle, gainDB, FFA0)


def AncSetGainFFB0(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    '''
    AncSetGain(engine, handle, gainDB, FFB0)


def AncSetGainFB0(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    FB path is associated with the FFA mic
    '''
    AncSetGain(engine, handle, gainDB, FB0)


def AncSetGainFFA1(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    '''
    AncSetGain(engine, handle, gainDB, FFA1)


def AncSetGainFFB1(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    '''
    AncSetGain(engine, handle, gainDB, FFB1)


def AncSetGainFB1(engine, handle, gainDB):
    '''
    write shift and gain values to specified filter path
    FB path is associated with the FFA mic
    '''
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
        paramvalue_hex = hex(getParam(param))
        paramvalue_int = str(convertFromQACTformat(getParam(param)))
        print(param + ' = ' + paramvalue_hex + ' (QACT format) ' + paramvalue_int + ' (int) ')
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
    setParam('OFFSET_ANC_FF_A_GAIN_L', convertToQACTformat(GAINS['FFA0_gain']))
    setParam('OFFSET_ANC_FF_A_SHIFT_L', convertToQACTformat(GAINS['FFA0_shift']))
    setParam('OFFSET_ANC_FF_B_GAIN_L', convertToQACTformat(GAINS['FFB0_gain']))
    setParam('OFFSET_ANC_FF_B_SHIFT_L', convertToQACTformat(GAINS['FFB0_shift']))
    setParam('OFFSET_ANC_FB_GAIN_L', convertToQACTformat(GAINS['FB0_gain']))
    setParam('OFFSET_ANC_FB_SHIFT_L', convertToQACTformat(GAINS['FB0_shift']))
    #right gain parameters
    setParam('OFFSET_ANC_FF_A_GAIN_R', convertToQACTformat(GAINS['FFA1_gain']))
    setParam('OFFSET_ANC_FF_A_SHIFT_R', convertToQACTformat(GAINS['FFA1_shift']))
    setParam('OFFSET_ANC_FF_B_GAIN_R', convertToQACTformat(GAINS['FFB1_gain']))
    setParam('OFFSET_ANC_FF_B_SHIFT_R', convertToQACTformat(GAINS['FFB1_shift']))
    setParam('OFFSET_ANC_FB_GAIN_R', convertToQACTformat(GAINS['FB1_gain']))
    setParam('OFFSET_ANC_FB_SHIFT_R', convertToQACTformat(GAINS['FB1_shift']))


def updateGains():
    '''
    update GAINS with values from psdata, used to initialize GAINS after reading from device
    '''
    #left gain parameters
    GAINS['FFA0_gain'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_A_GAIN_L'))
    GAINS['FFA0_shift'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_A_SHIFT_L'))
    GAINS['FFB0_gain'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_B_GAIN_L'))
    GAINS['FFB0_shift'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_B_SHIFT_L'))
    GAINS['FB0_gain'] = convertFromQACTformat(getParam('OFFSET_ANC_FB_GAIN_L'))
    GAINS['FB0_shift'] = convertFromQACTformat(getParam('OFFSET_ANC_FB_SHIFT_L'))
    #right gain parameters
    GAINS['FFA1_gain'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_A_GAIN_R'))
    GAINS['FFA1_shift'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_A_SHIFT_R'))
    GAINS['FFB1_gain'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_B_GAIN_R'))
    GAINS['FFB1_shift'] = convertFromQACTformat(getParam('OFFSET_ANC_FF_B_SHIFT_R'))
    GAINS['FB1_gain'] = convertFromQACTformat(getParam('OFFSET_ANC_FB_GAIN_R'))
    GAINS['FB1_shift'] = convertFromQACTformat(getParam('OFFSET_ANC_FB_SHIFT_R'))
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


def convertFromQACTformat(num):
    '''
    input a 32 bit intger and it interprets it as 2's complement
    where 0xFFFFFE00 = -512 (instead of 4294966784L)
    '''
    if num > (2**31 - 1):
        out = num - 2**32
    else:
        out = num
    return int(out)


def convertToQACTformat(num):
    '''
    convert python 32 bit signed int to QACT friendly 32 bit format Q13.19
    '''
    if num < 0:
        out = num + 2**32
    else:
        out = num
    return int(out)


if __name__ == '__main__':
    # current file path of latest version of BlueSuite
    BluesuitePath = r'C:\Program Files (x86)\QTIL\BlueSuite 3.1.1'
    engine = TestEngine(BluesuitePath)
    handle = connectToDevice(engine, TRANSPORT_USBDBG)
    getChipName(engine, handle)
    readAudioKey(engine, handle, ANC_PSKEY_ACTIVE) # read ANC parameters from device into psdata cache
    updateGains()   # update local gains cache with values from psdata
    displayAudioKeyData() # display parameters in psdata cache

    print('place test code here to adjust gain values')
    # example calls to gain adjustment API shown below
    AncSetGainFFA0(engine, handle, 5)
    AncSetGainFFA1(engine, handle, 12)
    #AncSetGainFB0(engine, handle, -3)
    #AncSetGainFFA1(engine, handle, -15)
    #AncSetGainFFB1(engine, handle, 22.5)
    #AncSetGainFB1(engine, handle, 36)

    updatePsdata() # write data from gains cache into psdata
    writeAudioKey(engine, handle, ANC_PSKEY_ACTIVE) # write psdata cache to device
    writeHTF() # save HTF file

    # close connection to device
    engine.closeTestEngine(handle)
