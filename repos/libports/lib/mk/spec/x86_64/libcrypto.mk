# has to be the first path because it includes openssl/configuration.h
INC_DIR += $(REP_DIR)/src/lib/openssl/spec/64bit

CC_OPT += -DOPENSSL_CPUID_OBJ -DOPENSSL_IA32_SSE2 -DOPENSSL_BN_ASM_MONT
CC_OPT += -DOPENSSL_BN_ASM_MONT5 -DOPENSSL_BN_ASM_GF2m -DSHA1_ASM -DSHA256_ASM
CC_OPT += -DSHA512_ASM -DKECCAK1600_ASM -DRC4_ASM -DMD5_ASM -DAESNI_ASM -DVPAES_ASM
CC_OPT += -DGHASH_ASM -DECP_NISTZ256_ASM -DX25519_ASM -DPOLY1305_ASM

SRC_C = \
	crypto/bf/bf_enc.c \
	crypto/bn/asm/x86_64-gcc.c \
	crypto/camellia/cmll_misc.c \
	crypto/des/des_enc.c \
	crypto/des/fcrypt_b.c \
	crypto/ec/curve448/arch_64/f_impl64.c \
	crypto/ec/ecp_nistp224.c \
	crypto/ec/ecp_nistp256.c \
	crypto/ec/ecp_nistp521.c \
	crypto/ec/ecp_nistputil.c \
	crypto/ec/ecp_nistz256.c \
	providers/implementations/rands/seeding/rand_cpu_x86.c \
	# end of SRC_C

SRC_S = \
	crypto/x86_64cpuid.s \
	crypto/aes/asm/aes-x86_64.s \
	crypto/aes/asm/aesni-mb-x86_64.s \
	crypto/aes/asm/aesni-sha1-x86_64.s \
	crypto/aes/asm/aesni-sha256-x86_64.s \
	crypto/aes/asm/aesni-x86_64.s \
	crypto/aes/asm/bsaes-x86_64.s \
	crypto/aes/asm/vpaes-x86_64.s \
	crypto/bn/asm/rsaz-avx2.s \
	crypto/bn/asm/rsaz-x86_64.s \
	crypto/bn/asm/rsaz-2k-avx512.s \
	crypto/bn/asm/rsaz-3k-avx512.s \
	crypto/bn/asm/rsaz-4k-avx512.s \
	crypto/bn/asm/x86_64-gf2m.s \
	crypto/bn/asm/x86_64-mont.s \
	crypto/bn/asm/x86_64-mont5.s \
	crypto/camellia/asm/cmll-x86_64.s \
	crypto/chacha/asm/chacha-x86_64.s \
	crypto/ec/asm/ecp_nistz256-x86_64.s \
	crypto/ec/asm/x25519-x86_64.s \
	crypto/md5/asm/md5-x86_64.s \
	crypto/modes/asm/aesni-gcm-x86_64.s \
	crypto/modes/asm/ghash-x86_64.s \
	crypto/poly1305/asm/poly1305-x86_64.s \
	crypto/rc4/asm/rc4-md5-x86_64.s \
	crypto/rc4/asm/rc4-x86_64.s \
	crypto/sha/asm/keccak1600-x86_64.s \
	crypto/sha/asm/sha1-mb-x86_64.s \
	crypto/sha/asm/sha1-x86_64.s \
	crypto/sha/asm/sha256-mb-x86_64.s \
	crypto/sha/asm/sha256-x86_64.s \
	crypto/sha/asm/sha512-x86_64.s \
	crypto/whrlpool/asm/wp-x86_64.s \
	engines/asm/e_padlock-x86_64.s
	# end of SRC_S

OPENSSL_DIR := $(call select_from_ports,openssl)

vpath %.s $(OPENSSL_DIR)/src/lib/openssl

include $(REP_DIR)/lib/mk/libcrypto.inc

CC_CXX_WARN_STRICT =
