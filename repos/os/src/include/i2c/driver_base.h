/*
 *  \brief  VFS plugin for i2c
 *  \author Pirmin Duss
 *  \date   2024-03-29
 */

/*
 * Copyright (C) 2024 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _VFS_I2C__DRIVER_H_
#define _VFS_I2C__DRIVER_H_

/* Genode includes */
#include <base/allocator.h>
#include <base/env.h>
#include <base/exception.h>
#include <util/array.h>
#include <util/xml_node.h>
#include <vfs/env.h>


namespace I2c {

	using namespace Genode;

	struct Bus_address;
	struct Settings;
	struct Message;
	struct Transaction;

	using Byte_array = Array<uint8_t, 8>;
	using Name       = String<64>;

	class Driver_creation_error : Exception { };

	class Driver_base;

	class Bus_error : public Exception { };

	Driver_base* initialize(Env &env, Xml_node const &config);
}


struct I2c::Bus_address {
	uint8_t address;
};


struct I2c::Settings {

	uint16_t         bus_speed_khz;
	bool             verbose;
    I2c::Name const  name;
};


/**
 * A message to an I2C slave is either a read or write of one or more bytes
 */
struct I2c::Message : I2c::Byte_array
{
	enum Type { READ, WRITE };

	static uint8_t const  MAX_LEN { 16 };

	Type type  { READ };

	Message() { }

	template<typename ... ARGS>
	Message(Type type, ARGS ... args)
	:
		Byte_array { args... }, type { type }
	{ }
};


/**
 * A transaction to an I2C slave consists of one, or several messages
 */
struct I2c::Transaction : I2c::Array<I2c::Message, 2>
{
	using Base = Array<Message,2>;
	using Base::Base;
};


/*
 * Base class for platform specific driver to implement
 *
 * Note about the endianness: the driver is transparent.
 *
 * The driver read/write bytes to memory in the order they are
 * read/write to the bus.
 * It is the responsibility of the component interacting with
 * a slave device on the bus to figure out how to interpret the data.
 */
class I2c::Driver_base : Interface
{
	public:

	/**
	 * Transaction on the I2C bus
	 *
	 * \param address  device address
	 * \param t        transaction to perform
	 *
	 * \throw I2c::Session::Bus_error An error occure while performing an operation on the bus
	 */
	virtual void transfer(Bus_address &a, Transaction & t) = 0;
};


/*
 * factory method to create the model specific driver instance
 *
 * Throws Driver_creation_error when no driver can be created.`
 */
I2c::Driver_base &_create_driver_instance(Genode::Env &,
                                          Genode::Allocator &,
                                          Vfs::Env::User &,
                                          I2c::Settings const &);


#endif  /* _VFS_I2C__DRIVER_H_ */
