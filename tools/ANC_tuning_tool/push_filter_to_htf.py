'''
########################################################################################
 The purpose of this script is to 'push' a filter data set to the file system in QCC512x

       Usage: python push_filter_to_htf.py X Y ['samplefile.htf']
       Where: X is a text file holding the filter location parameters
              Y is the ANC filter location to write to
            Y = 1 -> 1st ANC filter'
            Y = 2 -> 2nd ANC filter'
            Y = 3 -> 3rd ANC filter'
            Y = 4 -> 4th ANC filter'
            Y = 5 -> 5th ANC filter'
    The third argument, if present, denotes the name of the file where the data will be saved if HTF format.

    To execute from within iPython (e.g. Anaconda environment):
        >>> import sys
        >>> sys.argv = ['push_filter_to_htf.py','sampleFilter.txt','1','samplefile.htf']
        >>> execfile(sys.argv[0])

Set the verboseFlag to 1 to print on screen the results of the internal functions
########################################################################################
'''
import sys, os, time, subprocess, collections, math
import ctypes as ct
from TestEngine_ANC_API import *
from TestEngine_ANC_constants import *
#import numpy as np

verboseFlag = 1

exitFlag = 0

if len(sys.argv) < 3:
  print 'Usage: python push_filter.py X Y'
  print 'Where X is a text file holding the filter location parameters'
  print 'and where Y is the ANC filter location to write to'
  print 'Y = 1 -> 1st ANC filter'
  print 'Y = 2 -> 2nd ANC filter'
  print 'Y = 3 -> 3rd ANC filter'
  print 'X = ... -> ...th ANC filter'
  print 'X = %d -> %dth ANC filter' % (TOTAL_NUM_FILTERS, TOTAL_NUM_FILTERS)
  exitFlag = 1

if exitFlag != 1:
    if int(sys.argv[2]) <= TOTAL_NUM_FILTERS:
        key = 0x204100
        keyIndex = (int(sys.argv[2]) - 1) * 2
        key = key + keyIndex
        print 'Writing key',hex(key)

    else:
        print '%s is not a valid filter location. The valid range is 1-%d.' % ( sys.argv[2], TOTAL_NUM_FILTERS )
        exitFlag = 1

if exitFlag != 1:
    try:
      file_name = sys.argv[1]
      fileIn = open(file_name, 'r')
    except:
      print 'Error opening file: %s  Please check that the file exists.' % (file_name)
      exitFlag = 1

if exitFlag != 1:

    CURDIR = os.path.dirname(os.path.realpath(sys.argv[0]))

    #----------------------------------------------------------------------------------------
    # IMPORTANT to update this path variable per version and location of ADK tools location
    # location of TestEngine.dll
    #----------------------------------------------------------------------------------------
    BluesuitePath = r'C:\qtil\ADK_QCC512x_QCC302x_WIN_6.3.0.154\tools\bin'

    #TRANSPORT_USBDBG = 256
    #TRANSPORT_TRB = 128

    TRANSPORT = TRANSPORT_TRB
    if TRANSPORT == TRANSPORT_TRB:
        TransportText = 'TRB'
    else:
        if TRANSPORT == TRANSPORT_USBDBG:
            TransportText = 'USB'

    try:
        #--------------------------------------------------------------------------------------------------------------
        # Connect to QCC via USB or TRB and Read audio key data but suppress the print to screen messages to keep things neat.
        #--------------------------------------------------------------------------------------------------------------
        old_stdout = sys.stdout
        if verboseFlag == 0:
            sys.stdout = open(os.devnull, 'w')
        engine, handle = ANCConnectDUT( BluesuitePath, TRANSPORT)
    except:
        sys.stdout = old_stdout
        print "QCC device not detected on",TransportText
        print "First connect to USB, then Power On, then ANC ON"
        handle = -1


    #--------------------------------------------------------------------------------------------------------------------------------------------
    # This function advances to the next line in the  then reads and parses the data of interest which is then returned to the calling point
    #--------------------------------------------------------------------------------------------------------------------------------------------
    def read_line_from_file():
      return int(str(next(fileIn,'').split(':',1)[1]).strip(),16)

    #-----------------------------------------------------------------------------
    # Read from filter parameter text file and write contents into parameter array
    #-----------------------------------------------------------------------------
    if handle != -1:
        sys.stdout = old_stdout
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
            #setParam('OFFSET_ANC_FF_B_SHIFT_L', read_line_from_file())
            #setParam('OFFSET_ANC_FF_B_GAIN_L',  read_line_from_file())
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
            #setParam('OFFSET_ANC_FF_A_SHIFT_L', read_line_from_file())
            #setParam('OFFSET_ANC_FF_A_GAIN_L',  read_line_from_file())
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
            #setParam('OFFSET_ANC_FB_SHIFT_L', read_line_from_file())
            #setParam('OFFSET_ANC_FB_GAIN_L',  read_line_from_file())
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
            #setParam('OFFSET_ANC_FF_B_SHIFT_R', read_line_from_file())
            #setParam('OFFSET_ANC_FF_B_GAIN_R',  read_line_from_file())
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
            #setParam('OFFSET_ANC_FF_A_SHIFT_R', read_line_from_file())
            #setParam('OFFSET_ANC_FF_A_GAIN_R',  read_line_from_file())
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
            #setParam('OFFSET_ANC_FB_SHIFT_R', read_line_from_file())
            #setParam('OFFSET_ANC_FB_GAIN_R',  read_line_from_file())
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

        fileIn.close()
        updatePsdata()
        writeAudioKey(engine, handle, key)
        if len(sys.argv) > 3:
            writeHTF(filename= sys.argv[3])

        engine.closeTestEngine(handle)
        sys.stdout = old_stdout
        print 'Process finished'
