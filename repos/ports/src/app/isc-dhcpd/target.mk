TARGET := isc-dhcpd

ISC_DHCP_DIR := $(call select_from_ports,isc-dhcp)/src/app/isc-dhcp

SRC_CC += $(notdir $(wildcard $(ISC_DHCP_DIR)/server/*.c))

vpath %.c $(ISC_DHCP_DIR)/server

INC_DIR += $(PRG_DIR)
INC_DIR += $(ISC_DHCP_DIR)/includes
INC_DIR += $(ISC_DHCP_DIR)/includes/omapip
INC_DIR += $(ISC_DHCP_DIR)/bind/include

#LIBS += libc bind9 bind9-isc bind9-dns
LIBS += libc bind9
