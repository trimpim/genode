BIND9_DIR := $(call select_from_ports,bind9)/src/lib/bind9

FILTER_OUT = entropy.c

SRC_C += $(filter-out $(FILTER_OUT),$(notdir $(wildcard $(BIND9_DIR)/lib/isc/*.c)))
SRC_C += $(notdir $(wildcard $(BIND9_DIR)/lib/isc/unix/*.c))

#$(info $$SRC_C is [${SRC_C}])

vpath %.c $(BIND9_DIR)/lib/isc/unix
vpath %.c $(BIND9_DIR)/lib/isc

INC_DIR += $(REP_DIR)/src/lib/isc
INC_DIR += $(BIND9_DIR)/lib/isc/include
INC_DIR += $(BIND9_DIR)/lib/isc/unix/include
INC_DIR += $(BIND9_DIR)/lib/isc/nothreads/include
INC_DIR += $(BIND9_DIR)/lib/isc/noatomic/include

CC_C_OPT = -w -std=gnu99

LIBS += libc libm zlib
