/*
 * \brief  Test program for the xoroshiro_terminal
 * \author Pirmin Duss
 * \date   -----------
 */

/*
 * Copyright (C) 2008-2016 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/log.h>

#include <fstream>

int main(int argc, char* argv[])
{
	std::string const file_name("/dev/random");
	std::ifstream file(file_name);
	if (!file)
	{
		Genode::error("unable to open ", file_name.c_str());
		return -1;
	}

	return 0;
}
