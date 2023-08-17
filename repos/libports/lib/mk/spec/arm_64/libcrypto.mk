# has to be the first path because it includes openssl/configuration.h
INC_DIR += $(REP_DIR)/src/lib/openssl/spec/64bit

CC_OPT += -DBSAES_ASM -DECP_NISTZ256_ASM -DKECCAK1600_ASM -DMD5_ASM
CC_OPT += -DOPENSSL_BN_ASM_MONT -DOPENSSL_CPUID_OBJ -DOPENSSL_SM3_ASM
CC_OPT += -DPOLY1305_ASM -DSHA1_ASM -DSHA256_ASM -DSHA512_ASM -DSM4_ASM
CC_OPT += -DVPAES_ASM -DVPSM4_ASM


SRC_C = \
	armcap_genode.c \
	crypto/aes/aes_cbc.c \
	crypto/aes/aes_core.c \
	crypto/bf/bf_enc.c \
	crypto/bn/bn_asm.c \
	crypto/camellia/cmll_cbc.c \
	crypto/camellia/cmll_misc.c \
	crypto/des/des_enc.c \
	crypto/des/fcrypt_b.c \
	crypto/ec/curve448/arch_64/f_impl64.c \
	crypto/ec/ecp_nistp224.c \
	crypto/ec/ecp_nistp256.c \
	crypto/ec/ecp_nistp521.c \
	crypto/ec/ecp_nistputil.c \
	crypto/ec/ecp_nistz256.c \
	providers/implementations/rands/seeding/rand_cpu_arm64.c \
	# end of SRC_C

SRC_S = \
	aes/asm/aesv8-arm64.s \
	aes/asm/bsaes-armv8.s \
	aes/asm/vpaes-armv8.s \
	arm64cpuid.s \
	bn/asm/armv8-mont.s \
	chacha/asm/chacha-armv8-sve.s \
	chacha/asm/chacha-armv8.s \
	ec/asm/ecp_nistz256-armv8.s \
	modes/asm/aes-gcm-armv8-unroll8_64.s \
	modes/asm/aes-gcm-armv8_64.s \
	modes/asm/ghashv8-arm64.s \
	poly1305/asm/poly1305-armv8.s \
	sha/asm/keccak1600-armv8.s \
	sha/asm/sha1-armv8.s \
	sha/asm/sha256-armv8.s \
	sha/asm/sha512-armv8.s \
	sm3/asm/sm3-armv8.s \
	sm4/asm/sm4-armv8.s \
	sm4/asm/vpsm4-armv8.s \
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
