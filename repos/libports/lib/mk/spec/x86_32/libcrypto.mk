# has to be the first path because it includes openssl/configuration.h
INC_DIR += $(REP_DIR)/src/lib/openssl/spec/32bit

CC_OPT += -DAES_ASM -DCMLL_ASM -DDES_ASM -DECP_NISTZ256_ASM -DGHASH_ASM -DMD5_ASM
CC_OPT += -DOPENSSL_BN_ASM_GF2m -DOPENSSL_BN_ASM_MONT -DOPENSSL_BN_ASM_PART_WORDS
CC_OPT += -DOPENSSL_CPUID_OBJ -DOPENSSL_IA32_SSE2 -DPOLY1305_ASM -DRMD160_ASM
CC_OPT += -DSHA1_ASM -DSHA256_ASM -DSHA512_ASM -DVPAES_ASM -DWHIRLPOOL_ASM
CC_OPT += -DOPENSSL_NO_EC_NISTP_64_GCC_128 -DAESNI_ASM -DRC4_ASM

SRC_C = \
	crypto/ec/curve448/arch_32/f_impl32.c \
	crypto/ec/ecp_nistz256.c \
	providers/implementations/rands/seeding/rand_cpu_x86.c \
	# end of SRC_C

SRC_S = \
	crypto/aes/asm/aes-586.s \
	crypto/aes/asm/aesni-x86.s \
	crypto/aes/asm/vpaes-x86.s \
	crypto/bf/asm/bf-586.s \
	crypto/bn/asm/bn-586.s \
	crypto/bn/asm/co-586.s \
	crypto/bn/asm/x86-gf2m.s \
	crypto/bn/asm/x86-mont.s \
	crypto/camellia/asm/cmll-x86.s \
	crypto/chacha/asm/chacha-x86.s \
	crypto/des/asm/crypt586.s \
	crypto/des/asm/des-586.s \
	crypto/ec/asm/ecp_nistz256-x86.s \
	crypto/x86cpuid.s \
	crypto/md5/asm/md5-586.s \
	crypto/modes/asm/ghash-x86.s \
	crypto/poly1305/asm/poly1305-x86.s \
	crypto/rc4/asm/rc4-586.s \
	crypto/ripemd/asm/rmd-586.s \
	crypto/sha/asm/sha1-586.s \
	crypto/sha/asm/sha256-586.s \
	crypto/sha/asm/sha512-586.s \
	crypto/whrlpool/asm/wp-mmx.s \
	engines/asm/e_padlock-x86.s \
	# end of SRC_S

OPENSSL_DIR := $(call select_from_ports,openssl)

vpath %.s $(OPENSSL_DIR)/src/lib/openssl

include $(REP_DIR)/lib/mk/libcrypto.inc

CC_CXX_WARN_STRICT =
