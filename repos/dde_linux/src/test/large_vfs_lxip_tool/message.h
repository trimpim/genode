/*
 * \brief  Message parsing utilities
 * \author Roman Iten
 * \date   2023-10-24
 */
/*
 * Copyright (C) 2023 Gapfruit AG
 */

#pragma once
/* Genode includes */
#include <util/token.h>
/* STL includes */
#include <string>

namespace vfs_lxip_test {

	class Properties;
	class Scanner_policy_identifier_only;

	/**
	 * Define tokenizer that matches key/values of property lists
	 */
	typedef ::Genode::Token<Scanner_policy_identifier_only> Token;
}

/**
 * Scanner policy that accepts letters, digits and even some
 * special characters anywhere in an identifier. As a side
 * effect, it doesn't distinct between IDENT, SINGLECHAR,
 * NUMBER or STRING.
 */
struct vfs_lxip_test::Scanner_policy_identifier_only
{
	static bool identifier_char(char c, unsigned /* i */)
	{
		using Genode::is_letter;
		using Genode::is_digit;
		return is_letter(c) || is_digit(c) || (c == '_')
		                                   || (c == '-')
		                                   || (c == '.')
		                                   || (c == '%');
	}
	static bool end_of_quote(const char * /* s */) { return false; }
};

/**
 * A parser for property lists
 *
 * A property has the form 'name=value', multiple properties are separated by
 * ','.
 */
class vfs_lxip_test::Properties
{
	private:

		Token _key;
		Token _equals { _key   .next().eat_whitespace() };
		Token _value  { _equals.next().eat_whitespace() };
		std::string const _separator;

	public:

		Properties(Token token, char const *separator) : _key(token.eat_whitespace()), _separator(separator) { }
		bool valid() const
		{
			return (_key.type()   == Token::IDENT)
			    && (_equals[0]    == '=')
			    && (_value.type() == Token::IDENT);
		}

		std::string key()   const { return std::string(_key.start(), _key.len()); }
		std::string value() const { return std::string(_value.start(), _value.len()); }

		/**
		 * Execute functor 'fn' for each property
		 */
		template <typename FN>
		void for_each(FN const &fn) const
		{
			Token token = _key;
			while (token.type() != Token::END) {
				Properties property(token, _separator.c_str());
				if (property.valid()) {
					fn(property.key().c_str(), property.value().c_str());
				}
				token = property._value.next_after(_separator.c_str());
			}
		}
};
