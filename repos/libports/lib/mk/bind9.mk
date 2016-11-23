BIND9_DIR := $(call select_from_ports,bind9)/src/lib/bind9

SRC_C = $(notdir $(wildcard $(BIND9_DIR)/lib/bind9/*.c))

vpath %.c $(BIND9_DIR)/lib/bind9

INC_DIR += $(REP_DIR)/src/lib/bind9
INC_DIR += $(BIND9_DIR)/lib/bind9/include

LIBS += libc isc
