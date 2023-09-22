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
#include <base/log.h>
#include <libc/component.h>

/* openssl includes */
#include <openssl/sha.h>

/* licc includes */
#include <stdlib.h>
#include <stdio.h>


void Libc::Component::construct(Libc::Env &)
{
	unsigned char const buf_in[]    { "This is a SHA256 test" };
	unsigned char       buf_out[32] { };

	bool result { false };

	Libc::with_libc([&] () {
		result = SHA256(buf_in, sizeof(buf_in), buf_out);
	});

	if (!result) {
		Genode::log("nok sha256_test failed");
		exit(1);
	}

	Genode::log("ok sha256_test");
	exit(0);
}
