TARGET := test-sha256_test

LIBS   += base libc libcrypto posix

CC_OPT += -Wno-error=conversion

SRC_CC  = test.cc
