TARGET := test-rand_status_test

SRC_C  += test/rand_status_test.c \
          test/testutil/test_cleanup.c \
          test/testutil/test_options.c \
          # end SRC_C

include $(REP_DIR)/src/test/openssl/target.inc
