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
#include <unistd.h>

#include "phy_driver.h"
#include "nrf24l01-mac.h"
#include "nrf24l01.h"
#include "nrf24l01_client.h"

// protocol version
#define NRF24_VERSION_MAJOR	1
#define NRF24_VERSION_MINOR	0

#define PACKET_SIZE 132

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

static int nrf24l01_probe()
{

	if (m_state > STATE_INVALID)
		return 0;

	if (PACKET_SIZE == 0 || PACKET_SIZE > UINT16_MAX)
		return -EINVAL;

	m_version.packet_size = PACKET_SIZE;

	if (nrf24l01_init() == 0) {
		m_state = STATE_UNKNOWN;
		return 0;
	}

	return -EIO;

}

static int nrf24l01_connect(int cli_sockfd, uint8_t to_addr, size_t len)
{

	int result;
	/* if m state is not UNKNOW, something went wrong in probe */
	if (m_state != STATE_UNKNOWN)
		return -EACCES;

	if (m_fd == SOCKET_INVALID || cli_sockfd != m_fd)
		return -EBADF;

	if (len != sizeof(int) || to_addr < NRF24_CH_MIN ||
						to_addr > NRF24_CH_MAX_1MBPS)
		return -EINVAL;

	result = nrf24l01_client_open(cli_sockfd, to_addr, &m_version);
	m_state = STATE_CLIENT;

	return result;
}

void nrf24l01_close(int socket)
{

	if (m_state == STATE_CLIENT)
		nrf24l01_client_close(socket);

	/* TO DO */
	/* if (m_state == STATE_SERVER) */
	/* Call server close */

	close(socket);

	if (socket == m_fd) {
		m_fd = SOCKET_INVALID;
		m_state =  STATE_UNKNOWN;
	}

}

static void nrf24l01_remove(void)
{

}

static int nrf24l01_open(const char *pathname)
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


	m_fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
	if (m_fd < 0) {
		m_fd = SOCKET_INVALID;
		return -errno;
	}


	return m_fd;
}

struct phy_driver nrf24l01 = {
	.name = "nRF24L01",
	.probe = nrf24l01_probe,
	.remove = nrf24l01_remove,
	.open = nrf24l01_open,
	.close = nrf24l01_close
};
