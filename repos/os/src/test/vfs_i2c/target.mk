TARGET    = test-vfs_i2c

GTEST_DIR := $(call select_from_ports,googletest)/src/lib/googletest/googletest

SRC_CC   += tests.cc

LIBS += base
LIBS += libc
LIBS += stdcxx
LIBS += vfs

CC_CXX_WARN_STRICT = -Wextra -Werror
