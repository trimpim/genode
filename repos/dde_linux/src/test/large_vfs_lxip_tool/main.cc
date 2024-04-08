

/* Genode includes */
#include <base/log.h>
#include <libc/component.h>
#include <util/string.h>

/* local includes */
#include "mqtt_client.h"


void Libc::Component::construct(Libc::Env &env)
{
	int result = -1;

	Libc::with_libc([&] () {

		mosqpp::lib_init();

		// TODO:: is this correct?
		static vfs_lxip_test::Mqtt_client mqtt_client("Mqtt_client", "10.0.2.1", 1883);
		result = mqtt_client.loop_forever();

		mosqpp::lib_cleanup();
	});

	env.parent().exit(result);
}
