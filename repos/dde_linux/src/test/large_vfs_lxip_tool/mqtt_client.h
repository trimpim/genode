
/**
 * MQTT client implementation of the SSM machine adapter
 */

/* Genode includes */
#include <base/log.h>

/* stdcxx includes */
#include <cstring>

/* mosquittopp includes */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <mosquittopp.h>
#pragma GCC diagnostic pop  /* restore -Weffc++ warnings */

/* local includes */
#include "echo_msg.h"


namespace vfs_lxip_test
{
	using namespace Genode;

	struct Mqtt_client;
}


struct vfs_lxip_test::Mqtt_client : public mosqpp::mosquittopp
{
	Mqtt_client(const char *id, const char *host, int port) :
		mosquittopp { id }
	{
		connect(host, port);
	}

	~Mqtt_client() = default;

	void on_connect(int rc) override
	{
		log("MQTT client connected with code ", rc);
		if (rc == 0) {
			/* only attempt to subscribe on a successful connect. */
//			subscribe(nullptr, "SSM/+/+/Status/+");
//			subscribe(nullptr, "SSM/+/+/Metric/+");
//			subscribe(nullptr, "to_device/#");
		}
	}

	void on_message(const struct mosquitto_message *message) override
	{
		/* construct zero-terminated string from byte buffer */
		auto const payload = std::string(static_cast<char *>(message->payload),
		                                 message->payloadlen);
		if (process(Echo_msg(message->topic, payload.c_str()))) return;

		warning("ignoring MQTT topic '", Genode::Cstring(message->topic), "'");
	}

	void on_subscribe(int /* mid */, int /* qos_count */, const int * /* granted_qos */) override
	{
		log("MQTT subscription succeeded");
	}

	/**
	 * Process message as telemetry
	 *
	 * \return    true if message has been processed as Telemetry object
	 */
	bool process(Echo_msg const &message)
	{
		return message.to_cloud([&] (char const *topic, char const *payload) {
			int const mqtt_result = publish(NULL, topic, std::strlen(payload), payload, 2);
			if (mqtt_result != MOSQ_ERR_SUCCESS)
				error("failed to publish MQTT message (result code: ", mqtt_result, ")");
		});
	}
};
