/*
 * Copyright (c) 2015, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#ifndef __PHY_DRIVER_NRF24__
#define __PHY_DRIVER_NRF24__

enum nrf24_cmds {
					CMD_SET_PIPE,
					CMD_SET_CHANNEL,
					CMD_GET_CHANNEL,
					CMD_SET_ADDRESS_PIPE,
					CMD_SET_POWER,
};

struct addr_pipe {
	int pipe;
	uint8_t aa[5];
};

#endif
