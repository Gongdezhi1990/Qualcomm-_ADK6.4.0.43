LIB=libconnection.a

HDRS=connection.h connection_enum.h

# DEFS+= -DDEBUG_PRINT_ENABLED
DEFS += -DCONNECTION_DEBUG_LIB 


OBJS=init.o \
	bluestack_handler.o \
    common.o \
    connection_manager.o \
    dm_baseband.o \
    dm_baseband_handler.o \
	dm_dut.o \
    dm_info.o \
    dm_info_handler.o \
    dm_init.o \
    dm_inquiry.o \
	dm_inquiry_handler.o \
    dm_link_policy.o \
    dm_qos.o \
    dm_sco.o \
    dm_sco_handler.o \
	dm_security_init.o \
	dm_security.o \
	dm_security_auth.o \
    dm_security_handler.o \
	l2cap_init.o \
	l2cap.o \
    l2cap_handler.o \
    rfc_init.o \
	rfc.o \
	rfc_handler.o \
	sdp_init.o \
	sdp.o \
    sdp_handler.o \
	tcp_init.o \
	tcp.o \
	tcp_handler.o \
	udp_init.o \
	udp.o \
	udp_handler.o
	
 
DOCS= connection.html

GIFS = connection.png


include ../Makefile.inc
