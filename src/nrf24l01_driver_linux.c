/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>

#include "hal.h"
#include "phy_driver.h"
#include "nrf24l01-mac.h"
#include "nrf24l01.h"

// protocol version
#define NRF24_VERSION_MAJOR	1
#define NRF24_VERSION_MINOR	0

enum {
	STATE_INVALID,
	STATE_UNKNOWN,
	STATE_SERVER,
	STATE_CLIENT
} e_state;

static int m_state = STATE_INVALID, m_fd = SOCKET_INVALID;

static version_t m_version =  {	major: NRF24_VERSION_MAJOR,
						minor : NRF24_VERSION_MINOR,
						packet_size : 0
};

static int nrf24l01_probe(size_t packet_size)
{

	if (m_state > STATE_INVALID)
		return 0;

	if (packet_size == 0 || packet_size > UINT16_MAX)
		return -EINVAL;

	m_version.packet_size = packet_size;

	if (nrf24l01_init() == 0) {
		m_state = STATE_UNKNOWN;
		return 0;
	}

	return -EIO;

}

static void nrf24l01_remove(void)
{

}

static int nrf24l01_socket(int type, int protocol)
{
	/* TODO: implement TCP forward and native (SPI)
	 * TCP FW intends to be a development mechanism to allow
	 * remote development. It should not be enabled by default.
	 * RPi should expose SPI communication through a TCP socket
	 * allowing an external machine to manage pipes and send data.
	 */

	if (m_state == STATE_INVALID)
		return -EACCES;


	if (m_fd != SOCKET_INVALID)
		return -EMFILE;


	switch (protocol) {

	case PROTO_NONE:
		m_fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
		if (m_fd < 0) {
			m_fd = SOCKET_INVALID;
			return -errno;
		}
		break;

	case PROTO_TCP_FWD:
		/* TO DO */
		break;
	}

	return m_fd;
}

static struct phy_driver nrf24l01 = {
	.name = "nRF24L01",
	.domain = PF_NRF24L01,
	.probe = nrf24l01_probe,
	.remove = nrf24l01_remove,
	.socket = nrf24l01_socket
};
