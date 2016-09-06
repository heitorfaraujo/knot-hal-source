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
#include "util.h"


static enum {
	UNKNOWN,
	REQUEST,
	PENDING,
	TIMEOUT,
	PTX_FIRE,
	PTX_GAP,
	PTX
} e_state;

typedef struct {
	unsigned long	start,
					delay;
} tline_t;

typedef struct {
	uint8_t		pipe,
				rxmn,
				txmn,
				heartbeat,
				heartbeat_wait;
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

/* send data to be transmitted by the radio */
static int16_t send_data(data_t *pdata, void *praw, uint16_t len)
{
	payload_t payload;
	uint8_t msg_type = pdata->msg_type;

	if (msg_type == NRF24_APP && len > NRF24_PW_MSG_SIZE) {
		//if message needs to be fragmented
		msg_type = NRF24_APP_LAST;
		if (pdata->offset == 0) {
			msg_type = NRF24_APP_FIRST;
			len = NRF24_PW_MSG_SIZE;
		} else {
			len -= pdata->offset;
			if (len > NRF24_PW_MSG_SIZE) {
				msg_type = NRF24_APP_FRAG;
				len = NRF24_PW_MSG_SIZE;
			}
		}
	}
	payload.hdr.msg_xmn = MSGXMN_SET(msg_type,
			pdata->pipe != BROADCAST ? m_client.txmn : 0);

	payload.hdr.net_addr = pdata->net_addr;
	memcpy(payload.msg.raw, (((uint8_t *)praw) + pdata->offset), len);
	pdata->offset_retry = pdata->offset;
	pdata->offset += len;
	//put the radio in TX mode
	nrf24l01_set_ptx(pdata->pipe);
	//transmit data
	nrf24l01_ptx_data(&payload, len + sizeof(hdr_t),
						pdata->pipe != BROADCAST);

	//waiting to be sent - returns 0 if success
	len = nrf24l01_ptx_wait_datasent();
	//put the radio in RX mode
	nrf24l01_set_prx();
	return len;
}

static int16_t ptx_service(data_t *pdata, void *praw, uint16_t len)
{
	int	state = PTX_FIRE,
			result = 0;

	tline_t	timer;

	while (state != PTX) {
		switch (state) {
		case  PTX_FIRE:
			timer.start = tline_ms();
			timer.delay = get_random_value(SEND_RANGE_MS,
						SEND_FACTOR, SEND_RANGE_MS_MIN);
			state = PTX_GAP;
			/* No break; fall through intentionally */
		case PTX_GAP:
			if (!tline_timeout(tline_ms(), timer.start, timer.delay))
				break;

			state = PTX;
			/* No break; fall through intentionally */
		case PTX:
			result = send_data(pdata, praw, len);
			if (result != 0) {
				if (--pdata->retry == 0) {
					printf("Failed to send message to the pipe\n");
					/* TO DO */
					/* Call disconect */
					return -ETIMEDOUT;
				} else {
					pdata->offset = pdata->offset_retry;
					state = PTX_FIRE;
				}
			} else {
				if (pdata->pipe != BROADCAST) {
					if (!m_client.heartbeat)
						m_client.heartbeat_wait = tline_ms();

					++m_client.txmn;
				}
				if (len != pdata->offset) {
					pdata->retry = SEND_RETRY;
					state = PTX_FIRE;
				}
			}
			break;
		}
	}
	return result;
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
