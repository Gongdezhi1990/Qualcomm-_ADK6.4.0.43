# Makefile for copying the kalima shared header files with VM libraries

TOP ?= ../../..
include $(TOP)/Makefile.path

DEST ?= $(BL3)

# Generate kalimba_standard_messages.h from the copy in
# vm/libs/kalimba_standard_messages
# (change # to ., defines to CONSTs with different prefix, and add an include be
#fore final endif)

inc_dir := $(wildcard  $(TOP)/vm/libs/kalimba_standard_messages/*.h)

all :: $(patsubst $(TOP)/vm/libs/kalimba_standard_messages/%, $(DEST)/kalimba/external_includes/vm/%, $(inc_dir))


$(DEST)/kalimba/external_includes/vm/%.h : $(TOP)/vm/libs/kalimba_standard_messages/%.h
	$(domkdir)
	sed -e 's/^#define[     ]*KALIMBA_MSG_\(.*\)/.CONST    $$MESSAGE_\1;/' \
	-e 's/^\#endif//' <$^ \
	| $(fixcopyright) | $(fixversion) >$@
	echo '#include "kalimba_messages.h"' >>$@
	echo '#endif' >>$@
	echo '' >>$@

