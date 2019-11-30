'''
###########################################################################################
 The purpose of this script is to 'push' a filter data set to QCC512x hardware registers

   Usage: python push_filter_to_hw_registers.py X
   Where X is a text file holding the filter location parameters

   Important-1: The filter file must be produced from the 'pull_filter.py' script
   Important-2: Data written to the QCC512x hardware registers will be lost upon power down

    To execute from within iPython (e.g. Anaconda environment):
        >>> import sys
        >>> sys.argv = ['push_filter_to_hw_registers.py','sampleFilter.txt']
        >>> execfile(sys.argv[0])

Set the verboseFlag to 1 to print on screen the results of the internal functions
###########################################################################################
'''
import sys, os, time, subprocess, collections, math
import ctypes as ct
from TestEngine_ANC_API import *
import numpy as np

verboseFlag = 1

exitFlag = 0
if len(sys.argv) < 2:
    print 'Usage: python push_filter_to_hw_registers.py X'
    print 'Where X is a text file holding the filter location parameters'
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
    TRANSPORT = TRANSPORT_USBDBG
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
    # This function advances to the next line in the fileIn, then reads and parses the data of interest which is then returned to the calling point
    #--------------------------------------------------------------------------------------------------------------------------------------------
    def read_line_from_file():
      return int(str(next(fileIn,'').split(':',1)[1]).strip(),16)

    #-----------------------------------------------------------------------------
    # Read from filter parameter text file and write contents into parameter array
    #-----------------------------------------------------------------------------
    if handle != -1:
        for line in fileIn:

          if 'ANC_ENABLE_REGISTERS' in line:
            setParam('OFFSET_FF_B_ENABLE_L',   read_line_from_file())
            setParam('OFFSET_FF_A_ENABLE_L',   read_line_from_file())
            setParam('OFFSET_FB_ENABLE_L',     read_line_from_file())
            setParam('OFFSET_FF_OUT_ENABLE_L', read_line_from_file())
            setParam('OFFSET_FF_B_ENABLE_R',   read_line_from_file())
            setParam('OFFSET_FF_A_ENABLE_R',   read_line_from_file())
            setParam('OFFSET_FB_ENABLE_R',     read_line_from_file())
            setParam('OFFSET_FF_OUT_ENABLE_R', read_line_from_file())
            anc0_enable, anc1_enable = readCurrentEnableState(None)
            EnableANC_withErrorCheck(engine, handle, anc0_enable, anc1_enable)

          if 'LEFT_FEEDFORWARD_(FF)_FILTER_BLOCK_PARAMETERS' in line:
            next(fileIn,'')
            enable = read_line_from_file()
            EnableDCFilter_FFb0(engine, handle, enable)

            shift = read_line_from_file()
            SetDCFilter_Shift_FFb0(engine, handle, shift)

            next(fileIn,'')
            coarse = read_line_from_file()
            fine = read_line_from_file()
            SetGain_CoarseFine_FFb0(engine, handle, coarse, fine)

            next(fileIn,'')
            LPF1 = read_line_from_file()
            LPF2 = read_line_from_file()
            SetLPF_Shift_FFb0(engine, handle, LPF1, LPF2)

            next(fileIn,'')
            coeffs     = [0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0]
            # Note: format for the coefficient vector: coeffs = [a0,a1,a2,a3,a4,a5,a6,  b0,b1,b2,b3,b4,b5,b6,b7]
            coeffs[0]  = read_line_from_file()>>4
            coeffs[1]  = read_line_from_file()>>4
            coeffs[2]  = read_line_from_file()>>4
            coeffs[3]  = read_line_from_file()>>4
            coeffs[4]  = read_line_from_file()>>4
            coeffs[5]  = read_line_from_file()>>4
            coeffs[6]  = read_line_from_file()>>4
            coeffs[7]  = read_line_from_file()>>4
            coeffs[8]  = read_line_from_file()>>4
            coeffs[9]  = read_line_from_file()>>4
            coeffs[10] = read_line_from_file()>>4
            coeffs[11] = read_line_from_file()>>4
            coeffs[12] = read_line_from_file()>>4
            coeffs[13] = read_line_from_file()>>4
            coeffs[14] = read_line_from_file()>>4
#            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            SetIIRFilter_Coeffs_FFb0(engine, handle, coeffs)

          if 'LEFT_FEEDBACK_(FB)_FILTER_BLOCK_PARAMETERS' in line:
            next(fileIn,'')
            enable = read_line_from_file()
            EnableSMLPF_Left(engine, handle, enable)

            shift = read_line_from_file()
            SetSMLPF_Shift_Left(engine, handle, shift)

            next(fileIn,'')
            enable = read_line_from_file()
            EnableDCFilter_FFa0(engine, handle, enable)

            shift = read_line_from_file()
            SetDCFilter_Shift_FFa0(engine, handle, shift)

            next(fileIn,'')
            coarse = read_line_from_file()
            fine = read_line_from_file()
            SetGain_CoarseFine_FFa0(engine, handle, coarse, fine)

            next(fileIn,'')
            LPF1 = read_line_from_file()
            LPF2 = read_line_from_file()
            SetLPF_Shift_FFa0(engine, handle, LPF1, LPF2)

            next(fileIn,'')
            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            # Note: format for the coefficient vector: coeffs = [a0,a1,a2,a3,a4,a5,a6,  b0,b1,b2,b3,b4,b5,b6,b7]
            coeffs[0]  = read_line_from_file()>>4
            coeffs[1]  = read_line_from_file()>>4
            coeffs[2]  = read_line_from_file()>>4
            coeffs[3]  = read_line_from_file()>>4
            coeffs[4]  = read_line_from_file()>>4
            coeffs[5]  = read_line_from_file()>>4
            coeffs[6]  = read_line_from_file()>>4
            coeffs[7]  = read_line_from_file()>>4
            coeffs[8]  = read_line_from_file()>>4
            coeffs[9]  = read_line_from_file()>>4
            coeffs[10] = read_line_from_file()>>4
            coeffs[11] = read_line_from_file()>>4
            coeffs[12] = read_line_from_file()>>4
            coeffs[13] = read_line_from_file()>>4
            coeffs[14] = read_line_from_file()>>4
 #           coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            SetIIRFilter_Coeffs_FFa0(engine, handle, coeffs)

          if 'LEFT_ECHO_CANCELLATION_(EC)_FILTER_BLOCK_PARAMETERS' in line:
            next(fileIn,'')
            coarse = read_line_from_file()
            fine = read_line_from_file()
            SetGain_CoarseFine_FB0(engine, handle, coarse, fine)

            next(fileIn,'')
            LPF1 = read_line_from_file()
            LPF2 = read_line_from_file()
            SetLPF_Shift_FB0(engine, handle, LPF1, LPF2)

            next(fileIn,'')
            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            # Note: format for the coefficient vector: coeffs = [a0,a1,a2,a3,a4,a5,a6,  b0,b1,b2,b3,b4,b5,b6,b7]
            coeffs[0]  = read_line_from_file()>>4
            coeffs[1]  = read_line_from_file()>>4
            coeffs[2]  = read_line_from_file()>>4
            coeffs[3]  = read_line_from_file()>>4
            coeffs[4]  = read_line_from_file()>>4
            coeffs[5]  = read_line_from_file()>>4
            coeffs[6]  = read_line_from_file()>>4
            coeffs[7]  = read_line_from_file()>>4
            coeffs[8]  = read_line_from_file()>>4
            coeffs[9]  = read_line_from_file()>>4
            coeffs[10] = read_line_from_file()>>4
            coeffs[11] = read_line_from_file()>>4
            coeffs[12] = read_line_from_file()>>4
            coeffs[13] = read_line_from_file()>>4
            coeffs[14] = read_line_from_file()>>4
  #          coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            SetIIRFilter_Coeffs_FB0(engine, handle, coeffs)

          if 'RIGHT_FEEDFORWARD_(FF)_FILTER_BLOCK_PARAMETERS' in line:
            next(fileIn,'')
            enable = read_line_from_file()
            EnableDCFilter_FFb1(engine, handle, enable)

            shift = read_line_from_file()
            SetDCFilter_Shift_FFb1(engine, handle, shift)

            next(fileIn,'')
            coarse = read_line_from_file()
            fine = read_line_from_file()
            SetGain_CoarseFine_FFb1(engine, handle, coarse, fine)

            next(fileIn,'')
            LPF1 = read_line_from_file()
            LPF2 = read_line_from_file()
            SetLPF_Shift_FFb1(engine, handle, LPF1, LPF2)

            next(fileIn,'')
            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            # Note: format for the coefficient vector: coeffs = [a0,a1,a2,a3,a4,a5,a6,  b0,b1,b2,b3,b4,b5,b6,b7]
            coeffs[0]  = read_line_from_file()>>4
            coeffs[1]  = read_line_from_file()>>4
            coeffs[2]  = read_line_from_file()>>4
            coeffs[3]  = read_line_from_file()>>4
            coeffs[4]  = read_line_from_file()>>4
            coeffs[5]  = read_line_from_file()>>4
            coeffs[6]  = read_line_from_file()>>4
            coeffs[7]  = read_line_from_file()>>4
            coeffs[8]  = read_line_from_file()>>4
            coeffs[9]  = read_line_from_file()>>4
            coeffs[10] = read_line_from_file()>>4
            coeffs[11] = read_line_from_file()>>4
            coeffs[12] = read_line_from_file()>>4
            coeffs[13] = read_line_from_file()>>4
            coeffs[14] = read_line_from_file()>>4
#            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            SetIIRFilter_Coeffs_FFb1(engine, handle, coeffs)

          if 'RIGHT_FEEDBACK_(FB)_FILTER_BLOCK_PARAMETERS' in line:
            next(fileIn,'')
            enable = read_line_from_file()
            EnableSMLPF_Right(engine, handle, enable)

            shift = read_line_from_file()
            SetSMLPF_Shift_Right(engine, handle, shift)

            next(fileIn,'')
            enable = read_line_from_file()
            EnableDCFilter_FFa1(engine, handle, enable)

            shift = read_line_from_file()
            SetDCFilter_Shift_FFa1(engine, handle, shift)

            next(fileIn,'')
            coarse = read_line_from_file()
            fine = read_line_from_file()
            SetGain_CoarseFine_FFa1(engine, handle, coarse, fine)

            next(fileIn,'')
            LPF1 = read_line_from_file()
            LPF2 = read_line_from_file()
            SetLPF_Shift_FFa1(engine, handle, LPF1, LPF2)

            next(fileIn,'')
            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            # Note: format for the coefficient vector: coeffs = [a0,a1,a2,a3,a4,a5,a6,  b0,b1,b2,b3,b4,b5,b6,b7]
            coeffs[0]  = read_line_from_file()>>4
            coeffs[1]  = read_line_from_file()>>4
            coeffs[2]  = read_line_from_file()>>4
            coeffs[3]  = read_line_from_file()>>4
            coeffs[4]  = read_line_from_file()>>4
            coeffs[5]  = read_line_from_file()>>4
            coeffs[6]  = read_line_from_file()>>4
            coeffs[7]  = read_line_from_file()>>4
            coeffs[8]  = read_line_from_file()>>4
            coeffs[9]  = read_line_from_file()>>4
            coeffs[10] = read_line_from_file()>>4
            coeffs[11] = read_line_from_file()>>4
            coeffs[12] = read_line_from_file()>>4
            coeffs[13] = read_line_from_file()>>4
            coeffs[14] = read_line_from_file()>>4
 #           coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            SetIIRFilter_Coeffs_FFa1(engine, handle, coeffs)

          if 'RIGHT_ECHO_CANCELLATION_(EC)_FILTER_BLOCK_PARAMETERS' in line:
            next(fileIn,'')
            coarse = read_line_from_file()
            fine = read_line_from_file()
            SetGain_CoarseFine_FB1(engine, handle, coarse, fine)

            next(fileIn,'')
            LPF1 = read_line_from_file()
            LPF2 = read_line_from_file()
            SetLPF_Shift_FB1(engine, handle, LPF1, LPF2)

            next(fileIn,'')
            coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            # Note: format for the coefficient vector: coeffs = [a0,a1,a2,a3,a4,a5,a6,  b0,b1,b2,b3,b4,b5,b6,b7]
            coeffs[0]  = read_line_from_file()>>4
            coeffs[1]  = read_line_from_file()>>4
            coeffs[2]  = read_line_from_file()>>4
            coeffs[3]  = read_line_from_file()>>4
            coeffs[4]  = read_line_from_file()>>4
            coeffs[5]  = read_line_from_file()>>4
            coeffs[6]  = read_line_from_file()>>4
            coeffs[7]  = read_line_from_file()>>4
            coeffs[8]  = read_line_from_file()>>4
            coeffs[9]  = read_line_from_file()>>4
            coeffs[10] = read_line_from_file()>>4
            coeffs[11] = read_line_from_file()>>4
            coeffs[12] = read_line_from_file()>>4
            coeffs[13] = read_line_from_file()>>4
            coeffs[14] = read_line_from_file()>>4
 #           coeffs     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
            SetIIRFilter_Coeffs_FB1(engine, handle, coeffs)

        fileIn.close()
        sys.stdout = old_stdout
        print 'Process finished'

        engine.closeTestEngine(handle)