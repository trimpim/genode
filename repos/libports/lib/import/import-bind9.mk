include $(call select_from_repositories,lib/import/import-bind9.inc)

INC_DIR += $(BIND9_PORT_DIR)/include/bind9
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/dns
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/dst
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/irs
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/isc
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/isc/unix
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/isc/platform
INC_DIR += $(BIND9_PORT_DIR)/include/bind9/isc/nothreads
