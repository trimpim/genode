TARGET := test-bntest

SRC_C  += test/bntest.c \
          # end SRC_C

include $(REP_DIR)/src/test/openssl/target.inc
