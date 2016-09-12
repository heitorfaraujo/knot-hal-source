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

int nrf24l01_server_open(int socket, int channel, version_t *pversion,
	const void *paddr, size_t laddr)
{

	return -1;
}

int nrf24l01_server_close(int socket)
{

	return -1;
}



