include $(call select_from_repositories,lib/import/import-bind9.inc)

INC_DIR += include/bind9
INC_DIR += $(BIND9_SRC_PORT_DIR)/lib/irs/include
INC_DIR += $(BIND9_PORT_DIR)/include/bind9
