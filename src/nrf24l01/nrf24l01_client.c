/*
* Copyright (c) 2015, CESAR.
* All rights reserved.
*
* This software may be modified and distributed under the terms
* of the BSD license. See the LICENSE file for details.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "nrf24l01-mac.h"
#include "nrf24l01_client.h"
#include "nrf24l01.h"
#include "phy_driver.h"

typedef struct {
	uint8_t		pipe;
} client_t;

typedef struct {
	uint8_t	msg_type,
				pipe;
	uint16_t	net_addr;
	int16_t		offset,
				retry,
				offset_retry;
} data_t;

static int16_t	m_fd = SOCKET_INVALID;
static client_t		m_client;
static version_t	*m_pversion = NULL;

static data_t *build_data(data_t *pd, uint8_t pipe, uint16_t net_addr,
						uint8_t msg_type)
{
	pd->msg_type = msg_type;
	pd->pipe = pipe;
	pd->net_addr = net_addr;
	pd->offset = 0;
	pd->retry = SEND_RETRY;
	pd->offset_retry = 0;
	return pd;
}

int16_t nrf24l01_client_open(int16_t socket, uint8_t channel,
							version_t *pversion)
{

	if (socket == SOCKET_INVALID)
		return -EBADF;

	if (m_fd != SOCKET_INVALID)
		return -EMFILE;

	//set channel
	if (pversion == NULL || nrf24l01_set_channel(channel) == -1)
		return -EINVAL;

	memset(&m_client, 0, sizeof(m_client));
	m_client.pipe = BROADCAST;
	m_fd = socket;
	m_pversion = pversion;

	/* TO DO */
	/* call join_local here */
	return -1;
}

int16_t nrf24l01_client_close(int16_t socket)
{
	return -1;
}

int16_t nrf24l01_client_read(int16_t socket, uint8_t *buffer, uint16_t len)
{
	return -1;
}

int16_t nrf24l01_client_write(int16_t socket, const uint8_t *buffer,
								uint16_t len)
{
	return -1;
}
