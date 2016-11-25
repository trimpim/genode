BIND9_DIR := $(call select_from_ports,bind9)/src/lib/bind9

SRC_C = $(notdir $(wildcard $(BIND9_DIR)/lib/bind9/*.c))

vpath %.c $(BIND9_DIR)/lib/bind9

INC_DIR += $(REP_DIR)/src/lib/bind9
INC_DIR += $(BIND9_DIR)/lib/bind9/include

INC_DIR += $(REP_DIR)/src/lib/bind9
INC_DIR += $(BIND9_DIR)/lib/isc/include
INC_DIR += $(BIND9_DIR)/lib/isc/unix/include
INC_DIR += $(BIND9_DIR)/lib/isc/nothreads/include
INC_DIR += $(BIND9_DIR)/lib/isc/noatomic/include
INC_DIR += $(BIND9_DIR)/lib/dns/include
INC_DIR += $(BIND9_DIR)/lib/isccfg/include/

VERSION = "9.11.0"
LIBINTERFACE = 160
LIBREVISION = 2
LIBAGE = 0

CC_C_OPT += -w -std=gnu99
CC_C_OPT += -DVERSION=\"${VERSION}\"
CC_C_OPT += -DLIBINTERFACE=${LIBINTERFACE}
CC_C_OPT += -DLIBREVISION=${LIBREVISION}
CC_C_OPT += -DLIBAGE=${LIBAGE}

LIBS += libc bind9-isc bind9-dns
