# has to be the first path because it includes openssl/configuration.h
INC_DIR += $(REP_DIR)/src/lib/openssl/spec/32bit

CC_OPT += -DOPENSSL_CPUID_OBJ -DOPENSSL_BN_ASM_MONT -DSHA1_ASM -DSHA256_ASM
CC_OPT += -DSHA512_ASM -DKECCAK1600_ASM -DECP_NISTZ256_ASM
CC_OPT += -DPOLY1305_ASM -DOPENSSL_NO_EC_NISTP_64_GCC_128

SRC_C = \
	armcap_genode.c \
	crypto/aes/aes_cbc.c \
	crypto/bf/bf_enc.c \
	crypto/bn/bn_asm.c \
	crypto/camellia/cmll_cbc.c \
	crypto/camellia/cmll_misc.c \
	crypto/des/des_enc.c \
	crypto/des/fcrypt_b.c \
	crypto/ec/curve448/arch_32/f_impl32.c \
	crypto/ec/ecp_nistz256.c \
	# end of SRC_C

SRC_S = \
	aes/asm/aesv8-arm32.S \
	aes/asm/aes-armv4.S \
	aes/asm/bsaes-armv7.S \
	bn/asm/armv4-gf2m.S \
	bn/asm/armv4-mont.S \
	chacha/asm/chacha-armv4.S \
	ec/asm/ecp_nistz256-armv4.S \
	armv4cpuid.S \
	modes/asm/ghash-armv4.S \
	modes/asm/ghashv8-arm32.S \
	poly1305/asm/poly1305-armv4.S \
	sha/asm/keccak1600-armv4.S \
	sha/asm/sha1-armv4-large.S \
	sha/asm/sha256-armv4.S \
	sha/asm/sha512-armv4.S \
	# end of SRC_S

OPENSSL_DIR := $(call select_from_ports,openssl)

vpath %.S $(OPENSSL_DIR)/src/lib/openssl/crypto

ifeq ($(filter-out $(SPECS),neon),)
	vpath armcap_genode.c $(REP_DIR)/src/lib/openssl/crypto/spec/neon
else
	vpath armcap_genode.c $(REP_DIR)/src/lib/openssl/crypto/spec/arm
endif

include $(REP_DIR)/lib/mk/libcrypto.inc

CC_CXX_WARN_STRICT =
