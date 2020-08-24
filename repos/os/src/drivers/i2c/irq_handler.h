/*
 * \brief  Input-interrupt handler
 * \author Josef Soentgen
 * \date   2015-04-08
 */

/*
 * Copyright (C) 2015-2017 Genode Labs GmbH
 * Copyright (C) 2020 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _DRIVERS__I2C__SPEC__IMX8Q__IRQ_HANDLER_H
#define _DRIVERS__I2C__SPEC__IMX8Q__IRQ_HANDLER_H

/* Genode includes */
#include <irq_session/client.h>

namespace I2c
{
	using namespace Genode;

	class Irq_handler;
}


class I2c::Irq_handler
{
	private:

		Env                           &_env;
		Irq_session_client             _irq;
		Io_signal_handler<Irq_handler> _handler;

		unsigned _sem_cnt = 1;

		void _handle() { _sem_cnt = 0; }

	public:

		Irq_handler(Env &env, Irq_session_capability irq)
		:
			_env(env), _irq(irq),
			_handler(env.ep(), *this, &Irq_handler::_handle)
		{
			_irq.sigh(_handler);
			_irq.ack_irq();
		}

		void wait()
		{
			_sem_cnt++;
			while (_sem_cnt > 0)
				_env.ep().wait_and_dispatch_one_io_signal();
		}

		void ack() { _irq.ack_irq(); }
};

#endif /* _DRIVERS__I2C__SPEC__IMX8Q__IRQ_HANDLER_H */