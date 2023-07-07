/*
 * \brief  Lx_emul time backend
 * \author Stefan Kalkowski
 * \date   2021-05-05
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */


#include <linux/time.h>

static unsigned long long genode_initial_ts_sec = 0;

void lx_emul_time_initial(unsigned long long seconds)
{
	genode_initial_ts_sec = seconds;
}

void read_persistent_clock64(struct timespec64 *ts)
{
	ts->tv_sec  = genode_initial_ts_sec;
	ts->tv_nsec = 0;
}
