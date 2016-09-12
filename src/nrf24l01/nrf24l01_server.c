/*
* Copyright (c) 2016, CESAR.
* All rights reserved.
*
* This software may be modified and distributed under the terms
* of the BSD license. See the LICENSE file for details.
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <glib.h>
#include <stdbool.h>
#include <string.h>

#include "nrf24l01.h"
#include "nrf24l01-mac.h"
#include "nrf24l01_client.h"
#include "list.h"

typedef struct {
	struct dlist_head	node;
	int	len,
			offset,
			pipe,
			retry,
			offset_retry;
	uint16_t	net_addr;
	uint8_t	msg_type,
				raw[];
} data_t;

typedef struct  {
	int			fdsock,
					pipe;
	uint16_t	net_addr,
					packet_size;
	uint32_t	hashid;

	uint8_t		rxmn,
					txmn;
	data_t		*rxmsg;
} client_t;

/* vector saves the pipes where the clientes are connected */
static client_t		*m_pipes_allocate[NRF24_PIPE_ADDR_MAX] = {NULL,
							NULL, NULL, NULL, NULL};

static inline int set_pipe_client(int pipe, client_t *pc)
{
	if (pipe > BROADCAST && pipe < (NRF24_PIPE_ADDR_MAX+1))
		m_pipes_allocate[pipe-1] = pc;

	return 0;
}

static data_t *build_data(int pipe, int net_addr, int msg_type,
	void *praw, uint16_t len, data_t *msg)
{
	uint16_t size = (msg != NULL) ? msg->len : 0;

	msg = (data_t *)g_realloc(msg, sizeof(data_t) + size + len);
	if (msg != NULL) {
		init_list_head(&msg->node);
		msg->len = size + len;
		msg->offset = 0;
		msg->pipe = pipe;
		msg->retry = SEND_RETRY;
		msg->offset_retry = 0;
		msg->net_addr = net_addr;
		msg->msg_type = msg_type;
		memcpy(msg->raw + size, praw, len);
	}
	return msg;
}

/* send data to be transmitted by the radio */
static int send_data(data_t *pdata, client_t *pc)
{
	payload_t payload;
	uint8_t msg_type = pdata->msg_type;
	int len = pdata->len;

	if (msg_type == NRF24_APP && len > NRF24_PW_MSG_SIZE) {
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
	payload.hdr.msg_xmn = MSGXMN_SET(msg_type, pc != NULL ? pc->txmn : 0);
	payload.hdr.net_addr = pdata->net_addr;
	memcpy(payload.msg.raw, pdata->raw + pdata->offset, len);
	pdata->offset_retry = pdata->offset;
	pdata->offset += len;

	nrf24l01_set_ptx(pdata->pipe);
	nrf24l01_ptx_data(&payload, len + sizeof(hdr_t), pc != NULL);

	/* returns 0 if success */
	len = nrf24l01_ptx_wait_datasent();
	nrf24l01_set_prx();
	return len;
}

int nrf24l01_server_open(int socket, int channel, version_t *pversion,
	const void *paddr, size_t laddr)
{

	return -1;
}

int nrf24l01_server_close(int socket)
{

	return -1;
}



