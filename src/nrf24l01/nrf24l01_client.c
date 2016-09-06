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


static int16_t	m_fd = SOCKET_INVALID;
static client_t		m_client;
static version_t	*m_pversion = NULL;

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
