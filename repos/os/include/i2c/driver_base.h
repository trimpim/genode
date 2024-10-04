/*
 *  \brief  VFS plugin TODO:
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
#include <base/env.h>
#include <base/exception.h>
#include <util/array.h>
#include <util/xml_node.h>
#include <vfs/env.h>


namespace I2c {

	using namespace Genode;

	struct Settings;
	struct Message;
	struct Transaction;

	using Byte_array = Array<uint8_t, 8>;

	class Driver_creation_error : Exception { };

	class Driver_base;

	class Bus_error : public Exception { };

	Driver_base* initialize(Env &env, Xml_node const &config);
}


struct I2c::Settings {

	enum {
		STATE_HIGH = 1,
		STATE_LOW  = 0,
	};

	enum class Mode {
	 	mode0 = 0, /* clk line POLARITY: 0 PHASE: 0 */
	 	mode1 = 1, /* clk line POLARITY: 0 PHASE: 1 */
	 	mode2 = 2, /* clk line POLARITY: 1 PHASE: 0 */
	 	mode3 = 3, /* clk line POLARITY: 1 PHASE: 1 */
	}

	Mode { Mode::mode2 };
	/*
	 * I2c clock idle state control. This control if the clock must
	 * stay HIGH or stay LOW while it is idle
	 */
	uint32_t clock_idle_state:          STATE_HIGH;

	/*
	 * I2c data lines state control. This control if the clock must
	 * stay HIGH or stay LOW while it is idle
	 */
	uint32_t data_lines_idle_state:     STATE_HIGH;

	/*
	 * I2c slave select line active state, determinate which state has to be
	 * considered the active state.
	 */
	uint32_t ss_line_active_state:      STATE_HIGH;
};

/**
 * A message to an I2C slave is either a read or write of one or more bytes
 */
struct I2c::Message : I2c::Byte_array
{
	enum Type { READ, WRITE };

	Type type { READ };

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
struct I2c::Transaction : I2c::Array<I2c::Message,2>
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
	virtual void trensfer(uint8_t address, I2c::Transaction & t) = 0;
};


/*
 * factory method to create the model specific driver instance
 *
 * Throws Driver_creation_error when no driver can be created.`
 */
I2c::Driver_base &_create_driver_instance(Genode::Env &, Vfs::Env::User &, I2c::Settings);


#endif  /* _VFS_I2C__DRIVER_H_ */

