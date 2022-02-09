/*
 * \brief  am1805 over i2c RTC driver
 * \author Jean-Adrien Domage <jean-adrien.domage@gapfruit.com>
 * \date   2022-02-09
 */

/*
 * Copyright (C) 2013-2022 Genode Labs GmbH
 * Copyright (C) 2021-2022 gapfruit AG
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <i2c_session/connection.h>
#include <i2c_session/i2c_session.h>
#include <rtc.h>

namespace Am_1805 {

	using namespace Genode;

	constexpr char const * I2C_SESSION_NAME = "AM1805";

	constexpr uint8_t MS_REGISTER_OFFSET     = 0x0;
	constexpr uint8_t SECOND_REGISTER_OFFSET = 0x1;
	constexpr uint8_t MIN_REGISTER_OFFSET    = 0x2;
	constexpr uint8_t HOUR_REGISTER_OFFSET   = 0x3;
	constexpr uint8_t DAY_REGISTER_OFFSET    = 0x4;
	constexpr uint8_t MONTH_REGISTER_OFFSET  = 0x5;
	constexpr uint8_t YEAR_REGISTER_OFFSET   = 0x6;
	constexpr uint8_t CONTROL1_REGISTER_OFFSET = 0x10;

	/*
	 * Helpers to access the I2c bus
	 */

	static I2c::Connection& get_connection(Env & env) {
		static I2c::Connection connection {env, I2C_SESSION_NAME};
		return connection;
	}

	static inline void write_register(Env &env, uint8_t offset, uint8_t byte) {
		auto & device = get_connection(env);

		I2c::Session::Transaction trxn {
			I2c::Session::Message { I2c::Session::Message::WRITE, offset, byte },
		};
		device.transmit(trxn);
	}

	static inline uint8_t read_register(Env &env, uint8_t offset) {
		auto & device = get_connection(env);

		I2c::Session::Transaction trxn {
			I2c::Session::Message { I2c::Session::Message::WRITE, offset },
			I2c::Session::Message { I2c::Session::Message::READ,  static_cast<uint8_t>(0x0) }
		};
		device.transmit(trxn);
		return trxn.value(1).value(0);
	}

	/*
	 * Helpers to manipulates am18X5 registers
	 */

#pragma pack(1)

	union Millisecond {
		uint8_t reg;
		struct {
			uint8_t sec_hundreds : 4;
			uint8_t sec_tens     : 4;
		};
	};

	union Second {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 3;
			uint8_t gp0  : 1;
		};
	};

	union Minute {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 3;
			uint8_t gp1  : 1;
		};
	};

	union Hour {
		uint8_t reg;
		struct {
			uint8_t ones  : 4;
			uint8_t tens  : 1;
			uint8_t am_pm : 1;
			uint8_t gp2   : 1;
			uint8_t gp3   : 1;
		} mode_12;
		struct {
			uint8_t ones  : 4;
			uint8_t tens  : 2;
			uint8_t gp2   : 1;
			uint8_t gp3   : 1;
		} mode_24;
	};

	union Day {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 2;
			uint8_t gp4  : 1;
			uint8_t gp5  : 1;
		};
	};

	union Month {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 1;
			uint8_t gp6  : 1;
			uint8_t gp7  : 1;
			uint8_t gp8  : 1;
		};
	};

	union Year {
		uint8_t reg;
		struct {
			uint8_t ones : 4;
			uint8_t tens : 4;
		};
	};

	union Control_1 {
		uint8_t reg;
		struct {
			uint8_t wrtc               : 1;
			uint8_t pwr2               : 1;
			uint8_t arst               : 1;
			uint8_t rsp                : 1;
			uint8_t out                : 1;
			uint8_t outb               : 1;
			uint8_t hour_12_or_24_mode : 1;
			uint8_t stop               : 1;
		};
	};

#pragma pack()

}

/*
 * Conversion helpers
 */

static inline Genode::uint8_t hundreds(unsigned value) {
	return static_cast<Genode::uint8_t>((value / 100) % 10);
}

static inline Genode::uint8_t tens(unsigned value) {
	return static_cast<Genode::uint8_t>((value / 10) % 10);
}

static inline Genode::uint8_t ones(unsigned value) {
	return static_cast<Genode::uint8_t>(value % 10);
}

/*
 * Rtc interface
 */

Rtc::Timestamp Rtc::get_time(Env &env) {

	Am_1805::Millisecond ms {
		.reg = Am_1805::read_register(env, Am_1805::MS_REGISTER_OFFSET)
	};
	Am_1805::Second second {
		.reg = Am_1805::read_register(env, Am_1805::SECOND_REGISTER_OFFSET)
	};
	Am_1805::Minute minute {
		.reg = Am_1805::read_register(env, Am_1805::MIN_REGISTER_OFFSET)
	};
	Am_1805::Hour hour {
		.reg = Am_1805::read_register(env, Am_1805::HOUR_REGISTER_OFFSET)
	};
	Am_1805::Day day {
		.reg = Am_1805::read_register(env, Am_1805::DAY_REGISTER_OFFSET)
	};
	Am_1805::Month month {
		.reg = Am_1805::read_register(env, Am_1805::MONTH_REGISTER_OFFSET)
	};
	Am_1805::Year year {
		.reg = Am_1805::read_register(env, Am_1805::YEAR_REGISTER_OFFSET)
	};
	Am_1805::Control_1 control1 {
		.reg = Am_1805::read_register(env, Am_1805::CONTROL1_REGISTER_OFFSET)
	};

	/* check the hour compute mode */
	uint8_t ts_hour;
	if (control1.hour_12_or_24_mode == 0) {
		/* operate in 24 hours mode */
		ts_hour = static_cast<Genode::uint8_t>(10u * hour.mode_24.tens)
		        + hour.mode_24.ones;
	} else {
		/* operate in 12 hours mode */
		ts_hour = static_cast<Genode::uint8_t>(10u * hour.mode_12.tens
		                                       + hour.mode_12.ones
		                                       + hour.mode_12.am_pm ? 12 : 0);
	}

	Rtc::Timestamp ts {
		.microsecond = 1000u * (100u * ms.sec_hundreds + 10u * ms.sec_tens),
		.second      = 10u * second.tens + second.ones,
		.minute      = 10u * minute.tens + minute.ones,
		.hour        = ts_hour,
		.day         = 10u * day.tens + day.ones,
		.month       = 10u * month.tens + month.ones,
		.year        = 2000u + 10u * year.tens + year.ones
	};
	return ts;
}


void Rtc::set_time(Env &env, Timestamp ts) {
	/* set control1 register offset, read it, and set wrtc bit */
	Am_1805::Control_1 control_1{
		.reg = Am_1805::read_register(env, Am_1805::CONTROL1_REGISTER_OFFSET)
	};
	control_1.wrtc = 1;
	Am_1805::write_register(env, Am_1805::CONTROL1_REGISTER_OFFSET, control_1.reg);

	Am_1805::Millisecond ms;
	ms.sec_tens = tens(ts.microsecond / 1000) & 0b1111;
	ms.sec_hundreds = hundreds(ts.microsecond / 1000) & 0b1111;

	Am_1805::Second second;
	second.tens = tens(ts.second) & 0b111;
	second.ones = ones(ts.second) & 0b1111;

	Am_1805::Minute minute;
	minute.tens = tens(ts.minute) & 0b111;
	minute.ones = ones(ts.minute) & 0b1111;

	/* Rtc session ts is always in 24 hours mode */
	Am_1805::Hour hour;
	if (control_1.hour_12_or_24_mode == 0) {
		/* operate in 24 hours mode */
		hour.mode_24.tens = tens(ts.hour) & 0b11;
		hour.mode_24.ones = ones(ts.hour) & 0b1111;
	} else {
		/* operate in 12 hours mode */
		if (ts.hour >= 12) {
			hour.mode_12.tens = tens(ts.hour - 12) & 0b1;
			hour.mode_12.ones = ones(ts.hour - 12) & 0b1111;
		} else {
			hour.mode_12.tens = tens(ts.hour) & 0b1;
			hour.mode_12.ones = ones(ts.hour) & 0b1111;
		}
	}

	Am_1805::Day day;
	day.tens = tens(ts.day) & 0b11;
	day.ones = ones(ts.day) & 0b1111;

	Am_1805::Month month;
	month.tens = tens(ts.month) & 0b1;
	month.ones = ones(ts.month) & 0b1111;

	Am_1805::Year year;
	year.tens = tens(ts.year) & 0b1111;
	year.ones = ones(ts.year) & 0b1111;

	Am_1805::write_register(env, Am_1805::MS_REGISTER_OFFSET, ms.reg);
	Am_1805::write_register(env, Am_1805::SECOND_REGISTER_OFFSET, second.reg);
	Am_1805::write_register(env, Am_1805::MIN_REGISTER_OFFSET, minute.reg);
	Am_1805::write_register(env, Am_1805::HOUR_REGISTER_OFFSET,hour.reg);
	Am_1805::write_register(env, Am_1805::DAY_REGISTER_OFFSET, day.reg);
	Am_1805::write_register(env, Am_1805::MONTH_REGISTER_OFFSET,month.reg);
	Am_1805::write_register(env, Am_1805::YEAR_REGISTER_OFFSET, year.reg);

	/* unset wrtc bit */
	control_1.wrtc = 0;
	Am_1805::write_register(env, Am_1805::CONTROL1_REGISTER_OFFSET, control_1.reg);
}
