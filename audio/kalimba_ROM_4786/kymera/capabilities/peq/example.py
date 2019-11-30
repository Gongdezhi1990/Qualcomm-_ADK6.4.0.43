############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
import sys

sys.path.append('c:\\p4det\\dspsw\\main\\kalimba\\kymera\\tools\\kalcmd')
sys.path.append('C:\\klab22a\\pythontools')
sys.path.append('C:\\p4det\\dspsw\\main\\kalsim\\tools')

import kalspi
import kalcmd_arch3
import batonKymera

kal = kalspi.KalSpi()
kal.connect('kalsim')

kalcmd = kalcmd_arch3.kalcmd()
b = batonKymera.batonKymera(kalcmd)

kal.run()

# create the operator (0x01=basic_mono, 0x49=PEQ)
print "creating op"
peq_opid = b.cop_c(0x49, 0, 0)

# OpMsg to override the processing mode (1=MUTE/standby, 2=Full_PROC, 3=pass-thru)
#b.opmsg(peq_opid,0x2002,1,1,0,3)

# OpMsg to overwrite the core type parameter (0=single_feedback(default) 1=first_order_error_feedback, 2=double_feedback)
#  id=0x2005, blocks=1, param_Start=1, nparam=1, params[1] = 0x000002
#b.opmsg(peq_opid,0x2005,1,1,1,0x0000,0x0200,0x0000)

# OpMsg to override coefficients (1stage lowpass)
#b.opmsg(peq_opid,1,1,0x0000,0x0001,0x0040,0x0000,0x0000,0x9fbc,0x0001,0x3f78,0x0000,0x9fbc,0x002f,
#0x6af4,0x0093,0x13fd,0x0000,0x00001)

# OpMsg to Create a filter by setting parameters (NBAND=1, MGAIN=0, type=lowpass_2ndorder, FC=1000Hz, Gain=0.0db, Q=0.707106781) configuration by overwriting six parameters
#  id=0x2005, blocks=1, param_Start=2, nparam=6, params[6] = 0x000001,0x000000,0x000007,0x003E80,0x000000,0x00B505
b.opmsg(peq_opid,0x2005,1,2,6,0x0000,0x0100,0x0000,0x0000,0x0700,0x3E80,0x0000,0x0000,0xB505)



ep_typ = 3

# Create endpoint for Source (3==PCM, 10==FILE)
b.create_source(ep_typ, [0, 0], 8000)
b.create_source(ep_typ, [1, 0], 8000)

# Create endpoint for Sink (3==PCM, 10==FILE)
b.create_sink(ep_typ, [0, 0], 8000)
b.create_sink(ep_typ, [1, 0], 8000)

# Create connections def scon(self, sourceOperator, sourceNumber, sinkOperator, sinkNumber):
inMono1 = b.scon(0, 0, peq_opid, 0)
inMono2 = b.scon(0, 1, peq_opid, 1)

outMono1 = b.scon(peq_opid, 0, 0, 0)
outMono2 = b.scon(peq_opid, 1, 0, 1)

# Sync the endpoints : The fields are: op_id, terminal_id, op_id, terminal_id
#b.sync_connections(peq_opid, 0x0000, peq_opid, 0x0001)
#b.sync_connections(peq_opid, 0x8000, peq_opid, 0x8001)

# Start operator
b.startop(peq_opid)



#end


#********** SAMPLE OPERATOR MESSAGES ****************

# OpMsg to set the sample rate to 48k
b.opmsg(peq_opid,0x200E,0x780)

# OpMsg to override coefficients (1stage lowpass)
b.opmsg(peq_opid,1,1,0x0000,0x0001,0x0040,0x0000,0x0000,0x9fbc,0x0001,0x3f78,0x0000,0x9fbc,0x002f,
0x6af4,0x0093,0x13fd,0x0000,0x00001)

# OpMsg to override coefficients 3stage peaking ("prasad" problem filter)
b.opmsg(peq_opid,1,3,
0x0000,0x0001,0x0040,0x0000,
0x003F,0x5E28,0x0080,0x6da4,0x0040,0x35C5,0x003F,0x93EE,0x0080,0x6DA4,0x0000,0x00001,
0x003F,0x2F66,0x0080,0xA433,0x0040,0x2F37,0x003F,0x5E95,0x0080,0xA433,0x0000,0x00001,
0x0038,0x73D5,0x0088,0x25D0,0x0040,0x6F15,0x0038,0xE2F3,0x0088,0x25D0,0x0000,0x00001)

# OpMsg to override the processing mode (1=MUTE, 2=Full_PROC, 3=pass-thru)
b.opmsg(peq_opid,0x2002,1,1,0,3)

# OpMsg to Create a filter by setting parameters (NBAND=1, MGAIN=0, type=lowpass_2ndorder, FC=1000Hz, Gain=0.0db, Q=0.707106781) configuration by overwriting six parameters
#  id=0x2005, blocks=1, param_Start=2, nparam=6, params[6] = 0x000001,0x000000,0x000007,0x003E80,0x000000,0x00B505
# b.opmsg(peq_opid,0x2005,1,2,6,0x0000,0x0100,0x0000,0x0000,0x0700,0x3E80,0x0000,0x0000,0xB505)

# OpMsg to overwrite the core type parameter (0=single_feedback(default) 1=first_order_error_feedback, 2=double_feedback)
#  id=0x2005, blocks=1, param_Start=1, nparam=1, params[1] = 0x000002
b.opmsg(peq_opid,0x2005,1,1,1,0x0000,0x0200,0x0000)

#disconnect test
#b.disconnect(inMono1)
#b.disconnect(inMono2)
#b.disconnect(inMono3)
#b.disconnect(inMono4)
#b.disconnect(outMono1)
#b.disconnect(outMono2)
#b.disconnect(outMono3)
#b.disconnect(outMono4)


###########################################################################33333
# 4ch

# Create endpoint for Source (3==PCM, 10==FILE)
b.create_source(ep_typ, [0, 0], 8000)
b.create_source(ep_typ, [1, 0], 8000)
b.create_source(ep_typ, [2, 0], 8000)
b.create_source(ep_typ, [3, 0], 8000)

# Create endpoint for Sink (3==PCM, 10==FILE)
b.create_sink(ep_typ, [0, 0], 8000)
b.create_sink(ep_typ, [1, 0], 8000)
b.create_sink(ep_typ, [2, 0], 8000)
b.create_sink(ep_typ, [3, 0], 8000)

# Create connections def scon(self, sourceOperator, sourceNumber, sinkOperator, sinkNumber):
inMono1 = b.scon(0, 0, peq_opid, 0)
inMono2 = b.scon(0, 1, peq_opid, 1)
inMono3 = b.scon(0, 2, peq_opid, 2)
inMono4 = b.scon(0, 3, peq_opid, 3)

outMono1 = b.scon(peq_opid, 0, 0, 0)
outMono2 = b.scon(peq_opid, 1, 0, 1)
outMono3 = b.scon(peq_opid, 2, 0, 2)
outMono4 = b.scon(peq_opid, 3, 0, 3)



###########################################################################33333
# 8ch

# Create endpoint for Source (3==PCM, 10==FILE)
b.create_source(ep_typ, [0, 0], 8000)
b.create_source(ep_typ, [1, 0], 8000)
b.create_source(ep_typ, [2, 0], 8000)
b.create_source(ep_typ, [3, 0], 8000)
b.create_source(ep_typ, [4, 0], 8000)
b.create_source(ep_typ, [5, 0], 8000)
b.create_source(ep_typ, [6, 0], 8000)
b.create_source(ep_typ, [7, 0], 8000)

# Create endpoint for Sink (3==PCM, 10==FILE)
b.create_sink(ep_typ, [0, 0], 8000)
b.create_sink(ep_typ, [1, 0], 8000)
b.create_sink(ep_typ, [2, 0], 8000)
b.create_sink(ep_typ, [3, 0], 8000)
b.create_sink(ep_typ, [4, 0], 8000)
b.create_sink(ep_typ, [5, 0], 8000)
b.create_sink(ep_typ, [6, 0], 8000)
b.create_sink(ep_typ, [7, 0], 8000)

# Create connections def scon(self, sourceOperator, sourceNumber, sinkOperator, sinkNumber):
inMono1 = b.scon(0, 0, peq_opid, 0)
inMono2 = b.scon(0, 1, peq_opid, 1)
inMono3 = b.scon(0, 2, peq_opid, 2)
inMono4 = b.scon(0, 3, peq_opid, 3)
inMono5 = b.scon(0, 4, peq_opid, 4)
inMono6 = b.scon(0, 5, peq_opid, 5)
inMono7 = b.scon(0, 6, peq_opid, 6)
inMono8 = b.scon(0, 7, peq_opid, 7)

outMono1 = b.scon(peq_opid, 0, 0, 0)
outMono2 = b.scon(peq_opid, 1, 0, 1)
outMono3 = b.scon(peq_opid, 2, 0, 2)
outMono4 = b.scon(peq_opid, 3, 0, 3)
outMono5 = b.scon(peq_opid, 4, 0, 4)
outMono6 = b.scon(peq_opid, 5, 0, 5)
outMono7 = b.scon(peq_opid, 6, 0, 6)
outMono8 = b.scon(peq_opid, 7, 0, 7)





###########################################################################33333
# 2ch

# Create endpoint for Source (3==PCM, 10==FILE)
b.create_source(10, [0, 0], 8000)
b.create_source(10, [1, 0], 8000)

# Create endpoint for Sink (3==PCM, 10==FILE)
b.create_sink(10, [0, 0], 8000)
b.create_sink(10, [1, 0], 8000)

# Create connections def scon(self, sourceOperator, sourceNumber, sinkOperator, sinkNumber):
inMono1 = b.scon(0, 0, peq_opid, 0)
inMono2 = b.scon(0, 1, peq_opid, 1)

outMono1 = b.scon(peq_opid, 0, 0, 0)
outMono2 = b.scon(peq_opid, 1, 0, 1)


