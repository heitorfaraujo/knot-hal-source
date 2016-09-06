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

// network retransmiting parameters
#define NRF24_DELAY_MS		(((NRF24_ARD + 1) * \
						NRF24_ARD_FACTOR_US) / 1000)

#define NRF24_TIMEOUT_MS			(NRF24_DELAY_MS * NRF24_ARC)
#define NRF24_HEARTBEAT_SEND_MS		2000
#define NRF24_HEARTBEAT_TIMEOUT_MS	(NRF24_HEARTBEAT_SEND_MS * 3)
#define NRF24_RETRIES				200

// constant to SEND time values
//	SEND_RANGE_MS_MIN <= SEND <= (SEND_RANGE_MS * SEND_FACTOR)
#define SEND_FACTOR					1
#define SEND_RANGE_MS_MIN	NRF24_DELAY_MS
#define SEND_RANGE_MS				NRF24_TIMEOUT_MS

// constant retry values
#define JOINREQ_RETRY	NRF24_RETRIES
#define SEND_RETRY		((NRF24_HEARTBEAT_TIMEOUT_MS - \
		(NRF24_HEARTBEAT_SEND_MS + SEND_RANGE_MS)) / SEND_RANGE_MS)

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
