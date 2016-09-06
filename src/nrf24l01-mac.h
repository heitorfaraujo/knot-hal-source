/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#ifndef __NRF24L01_MAC_H__
#define __NRF24L01_MAC_H__

#define BROADCAST			NRF24_PIPE0_ADDR

/**
 * struct version_t - network layer version
 * @major: protocol version, major number
 * @minor: protocol version, minor number
 * @packet_size: application packet size maximum
 *
 * This struct defines the network layer version.
 */
typedef struct __attribute__ ((packed)) {
	uint8_t				major;
	uint8_t				minor;
	uint16_t			packet_size;
} version_t;

#endif //	__NRF24L01_MAC__
