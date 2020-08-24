/*
 * \brief  Register mappings i2c driver for imx8q_evk
 * \author Pirmin Duss <pirmin.duss@gapfruit.com>
 * \data   2020-08-24
 */

 /*
  * Copyright (C) 2009-2017 Genode Labs GmbH
  * Copyright (C) 2020 gapfruit AG
  *
  * This file is part of the Genode OS framework, which is distributed
  * under the terms of the GNU Affero General Public License version 3.
  */


#ifndef _I2C_SPEC_IMX8Q_MMIO_H_
#define _I2C_SPEC_IMX8Q_MMIO_H_

/* Genode includes */
#include <util/mmio.h>

namespace I2c
{
	using namespace Genode;

	class Mmio;
}


class I2c::Mmio : public Genode::Mmio
{
	public:

		enum Bus_mode { SLAVE_MODE, MASTER_MODE };

		/********************
		 ** MMIO structure **
		 ********************/

		/* slave mode address register */
		struct Address : Register<0x00, 16>
		{
			struct Addr                : Bitfield<1, 7> { };  /* read write */
		};

		/* frequency divider for bus speed scaling */
		struct Freq_divider : Register<0x04, 16> { };

		/* control register */
		struct Control : Register<0x08, 16>
		{
			struct Repeat_start        : Bitfield<2, 1> { };  /* read only */
			struct Tx_ack_enable       : Bitfield<3, 1> { };  /* read write */
			struct Tx_rx_select        : Bitfield<4, 1> { };  /* read write */
			struct Master_slave_select : Bitfield<5, 1> { };  /* read write */
			struct Irq_enable          : Bitfield<6, 1> { };  /* read write */
			struct Enable              : Bitfield<7, 1> { };  /* read write */
		};

		/* status register */
		struct Status : Register<0x0c, 16>
		{
			struct Rcv_ack             : Bitfield<0, 1> { };  /* read only */
			struct Irq                 : Bitfield<1, 1> { };  /* read write */
			struct Slave_rw            : Bitfield<2, 1> { };  /* read only */
			struct Arbitration_lost    : Bitfield<4, 1> { };  /* read write */
			struct Busy                : Bitfield<5, 1> { };  /* read only */
			struct Addressed_as_slave  : Bitfield<6, 1> { };  /* read only */
			struct Data_transfer       : Bitfield<7, 1> { };  /* read only */
		};

		/* data transfer register */
		struct Data : Register<0x10, 16> { };

		Mmio(addr_t const base)
		:
			Genode::Mmio { base }
		{ }
};

#endif  /* _I2C_SPEC_IMX8Q_MMIO_H_ */