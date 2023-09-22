TARGET := test-sha256_test

LIBS   += base libc libcrypto

CC_OPT += -Wno-error=conversion -Wconversion

SRC_CC  = test.cc
