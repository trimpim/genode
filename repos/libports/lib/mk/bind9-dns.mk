BIND9_DIR := $(call select_from_ports,bind9)/src/lib/bind9

FILTER_OUT += dnstap.c
FILTER_OUT += gen.c
FILTER_OUT += $(notdir $(wildcard $(BIND9_DIR)/lib/dns/spnego*))

SRC_C += $(filter-out $(FILTER_OUT),$(notdir $(wildcard $(BIND9_DIR)/lib/dns/*.c)))

vpath %.c $(BIND9_DIR)/lib/dns

INC_DIR += $(REP_DIR)/src/lib/bind9-dns
INC_DIR += $(BIND9_DIR)/lib/dns
INC_DIR += $(BIND9_DIR)/lib/dns/include
INC_DIR += $(BIND9_DIR)/lib/isc/include
INC_DIR += $(BIND9_DIR)/lib/isc/unix/include
INC_DIR += $(BIND9_DIR)/lib/isc/noatomic/include
INC_DIR += $(BIND9_DIR)/lib/isc/nothreads/include

VERSION = 9.11
MAJOR = 9.11
MAPAPI = 1.0
LIBINTERFACE = 166
LIBREVISION = 2
LIBAGE = 0

CC_C_OPT = -w -std=gnu99 -include stddef.h
CC_C_OPT += -DVERSION=\"${VERSION}\"
CC_C_OPT += -DMAJOR=\"${MAJOR}\"
CC_C_OPT += -DMAPAPI=\"${MAPAPI}\"
CC_C_OPT += -DLIBINTERFACE=${LIBINTERFACE}
CC_C_OPT += -DLIBREVISION=${LIBREVISION}
CC_C_OPT += -DLIBAGE=${LIBAGE}

LIBS += libc
