/*
 * \brief  Test spi driver with a shift register
 * \author Jean-Adrien Domage <jean-adrien.domage@gapfruit.com>
 * \date   2021-04-28
 */

/*
 * Copyright (C) 2013-2021 Genode Labs GmbH
 * Copyright (C) 2021 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <libc/component.h>

/* libc includes */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

//#include <stdio.h>  // printf()
//#include <stdlib.h> // exit()
//#include <string.h> // strlen()

namespace I2c_test {

	using namespace Genode;

	class Main;
}


class I2c_test::Main
{
	private:

		char const *_device_name = "/dev/i2c/rtc";

		Env        &_env;
		int         _device_fd;

//		void _print_dir(char const *name, String<32> prefix_str = { })
//		{
//			DIR           *dp { nullptr };
//			struct dirent *ep { nullptr };
//
//	log(name);
//			Libc::with_libc([&dp, &name] {
//				dp = opendir("dev");
//				if (dp == nullptr) {
//					error("failed to open directory '", name, "'");
//					exit(1);
//				}
//			});
//
//			while ((ep = readdir(dp)) != nullptr) {
//
//				if (ep->d_type == DT_DIR) {
//					log(prefix_str, "  dir  : ", Cstring { ep->d_name });
//					_print_dir(String<128> { name, "/", Cstring { ep->d_name} }.string(),
//					           String<32>  { prefix_str, "  "} );
//				} else  {
//					log(prefix_str, "  file : ", Cstring { ep->d_name });
//				}
//			}
//
//			Libc::with_libc([dp] {
//				closedir (dp);
//			});
//		}

	public:

		Main(Env &env)
		:
			_env { env }
		{
//			_print_dir("dev");
//			Libc::with_libc([] {
//				auto dir1_fd = opendir("dev");
//				if (dir1_fd == nullptr) {
//					error("failed to open i2c File");
//					exit(1);
//				}
//				struct dirent *ep;
//				while ((ep = readdir(dir1_fd)) != nullptr)
//					log(Cstring { ep->d_name });
//				(void) closedir (dir1_fd);
//			});

			open_device();
			if (_device_fd < 0) {
				error("failed to open i2c File");
				exit(1);
			}
		}

		void open_device()
		{
			Libc::with_libc([this] {

				int flags = O_WRONLY | O_CREAT;
				mode_t mode = FREAD | FWRITE ;

				_device_fd = open(_device_name, flags, mode);
			});
		}
};

void Libc::Component::construct(Libc::Env &env)
{
	static I2c_test::Main main { env };
}
