LIBS  := libc libcrypto libssl posix

SRC_C += \
         apps/lib/opt.c \
         test/testutil/apps_shims.c \
         test/testutil/basic_output.c \
         test/testutil/cb.c \
         test/testutil/driver.c \
         test/testutil/fake_random.c \
         test/testutil/format_output.c \
         test/testutil/load.c \
         test/testutil/main.c \
         test/testutil/options.c \
         test/testutil/output.c \
         test/testutil/provider.c \
         test/testutil/random.c \
         test/testutil/stanza.c \
         test/testutil/tests.c \
         test/testutil/testutil_init.c \
         # end of SRC_C

OPENSSL_DIR := $(call select_from_ports,openssl)

vpath  %.c  $(OPENSSL_DIR)/src/lib/openssl/

INC_DIR += $(REP_DIR)/src/lib/openssl
INC_DIR += $(OPENSSL_DIR)/include
INC_DIR += $(OPENSSL_DIR)/src/lib/openssl/include
INC_DIR += $(OPENSSL_DIR)/src/lib/openssl/test
INC_DIR += $(OPENSSL_DIR)/src/lib/openssl/apps/include
INC_DIR += $(OPENSSL_DIR)/src/lib/openssl/providers/common/include
INC_DIR += $(OPENSSL_DIR)/src/lib/openssl/crypto/rsa

# vim: ft=make
