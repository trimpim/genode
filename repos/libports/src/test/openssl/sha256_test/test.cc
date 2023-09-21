/**
 * \brief  openssl SHA256 test
 * \author Pirmin Duss
 * \date   2023-09-20
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <libc/args.h>
#include <libc/component.h>

/* openssl includes */
#include <openssl/sha.h>

/* licc includes */
#include <stdlib.h>
#include <stdio.h>


void Libc::Component::construct(Libc::Env &env)
{
	unsigned char const buf_in[]    { "This is a SHA256 test" };
	unsigned char       buf_out[32] { };

	bool result { false };

	Libc::with_libc([&] () {
		result = SHA256(buf_in, sizeof(buf_in), buf_out);
	});

	if (!result) {
		printf("nok sha256_test failed\n");
		exit(1);
	}

	printf("ok sha256_test\n");
}
