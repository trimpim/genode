/*
 * \brief  OpenSSL ARM CPU capabilities (with NEON support)
 * \author Pirmin Duss
 * \date   2021-02-17
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */


#include <stdint.h>
#include "arm_arch.h"


unsigned int OPENSSL_armcap_P = ARMV7_NEON;
unsigned int OPENSSL_armv8_rsa_neonized = 0;
unsigned int OPENSSL_arm_midr = 0;

void OPENSSL_cpuid_setup(void) { OPENSSL_armv8_rsa_neonized = 1; }

uint32_t OPENSSL_rdtsc(void) { return 0; }
