
/*
 * \brief  Parser for cloud-to-device recipe deployment messages
 * \author Roman Iten
 * \date   2023-10-25
 */
/*
 * Copyright (C) 2023 Gapfruit AG
 */
#pragma once

/* 3rd-party includes */
#include <json-c/json.h>

/* local includes */
#include "message.h"

namespace vfs_lxip_test {
	class Echo_msg;
}

/**
 * Representation of a recipe deployment message
 */
struct vfs_lxip_test::Echo_msg
{
	class Topic
	{
		private:
			Token _root;
			Token _properties { _root.next_after("/") };
		public:

			Topic(Token token) : _root(token) { }

			bool valid() const
			{
				// FIXME:
				// workaround that allows to declare this
				// method const even tough `Token::matches`
				// isn't, see genodelabs#5039
				Topic *topic = const_cast<Topic *>(this);
				return (topic->_root.matches("to_device")
				    && (topic->_properties.type() == Token::IDENT));
			}
			Token properties() const { return _properties; }
	};

	Topic       const  topic;
	Properties  const  properties;
	std::string const  payload;

	std::string _create_payload() const
	{
		enum {
			TX_SIZE  = 128 * 1024,
		};

		std::string out { };

		out.reserve(TX_SIZE);
		memset(out.data(), 'a', TX_SIZE-1);

		return out;
	}

	Echo_msg(char const *topic, char const *payload)
	:
		topic      { topic },
		properties { this->topic.properties(), "&" },
		payload    { _create_payload() }
	{ }

	template <typename FN>
	bool to_cloud(FN const &) const
	{
		if (!topic.valid())
			return false;

		struct json_object *main_array  { json_object_new_array() };
		struct json_object *main_obj    { json_object_new_object() };
		struct json_object *payload_obj { json_object_new_object() };

		json_object_array_add(main_array, main_obj);
		json_object_object_add(payload_obj, "payload", json_object_new_string(payload.c_str()));

		json_object_object_add(main_obj, "Payload", payload_obj);

		return true;
	}
};
