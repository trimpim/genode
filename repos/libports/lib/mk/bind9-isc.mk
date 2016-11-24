BIND9_DIR := $(call select_from_ports,bind9)/src/lib/bind9

FILTER_OUT_C += entropy.c
FILTER_OUT_C += $(notdir $(wildcard $(BIND9_DIR)/lib/isc/*_api.c))

FILTER_OUT_U += $(notdir $(wildcard $(BIND9_DIR)/lib/isc/unix/ifiter_*.c))

SRC_C += $(filter-out $(FILTER_OUT_C),$(notdir $(wildcard $(BIND9_DIR)/lib/isc/*.c)))
SRC_C += $(filter-out $(FILTER_OUT_U),$(notdir $(wildcard $(BIND9_DIR)/lib/isc/unix/*.c)))

vpath %.c $(BIND9_DIR)/lib/isc/unix
vpath %.c $(BIND9_DIR)/lib/isc

INC_DIR += $(REP_DIR)/src/lib/bind9
INC_DIR += $(BIND9_DIR)/lib/isc/include
INC_DIR += $(BIND9_DIR)/lib/isc/unix/include
INC_DIR += $(BIND9_DIR)/lib/isc/nothreads/include
INC_DIR += $(BIND9_DIR)/lib/isc/noatomic/include
INC_DIR += $(BIND9_DIR)/lib/dns/include

VERSION = "9.11"
LIBINTERFACE = 165
LIBREVISION = 1
LIBAGE = 5

CC_C_OPT += -w -std=gnu99
CC_C_OPT += -DVERSION=\"${VERSION}\"
CC_C_OPT += -DLIBINTERFACE=${LIBINTERFACE}
CC_C_OPT += -DLIBREVISION=${LIBREVISION}
CC_C_OPT += -DLIBAGE=${LIBAGE}

LIBS += libc libc-inet lxip zlib
