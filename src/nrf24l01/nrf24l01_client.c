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
#include <netinet/in.h>

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
	PTX,
	USERS,
	OVERFLOW,
	PRX
} e_state;

typedef struct {
	unsigned long	start,
					delay;
} tline_t;

typedef struct {
	uint8_t			pipe,
					rxmn,
					txmn,
					heartbeat;
	unsigned long	heartbeat_wait;
	uint32_t		hashid;
	uint16_t		net_addr;
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

static void disconnect(void)
{
	nrf24l01_set_standby();
	nrf24l01_close_pipe(0);
	memset(&m_client, 0, sizeof(m_client));
	m_client.pipe = BROADCAST;
	m_fd = SOCKET_INVALID;
	m_pversion = NULL;
}


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

					disconnect();
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

static int check_heartbeat(join_t *pj)
{
	if (tline_timeout(tline_ms(), m_client.heartbeat_wait,
						NRF24_HEARTBEAT_TIMEOUT_MS)) {
		disconnect();
		return -ETIMEDOUT;
	}

	if (!m_client.heartbeat && tline_timeout(tline_ms(),
			m_client.heartbeat_wait, NRF24_HEARTBEAT_SEND_MS)) {
		data_t data;

		pj->version.major = m_pversion->major;
		pj->version.minor = m_pversion->minor;
		pj->version.packet_size = khtons(m_pversion->packet_size);
		pj->hashid = khtonl(m_client.hashid);
		pj->data = khtonl(m_client.pipe);
		pj->result = NRF24_SUCCESS;
		if (ptx_service(build_data(&data, (m_client.pipe),
			khtons(m_client.net_addr), NRF24_HEARTBEAT), pj,
			sizeof(join_t)) == 0) {
			m_client.heartbeat_wait = tline_ms();
			m_client.heartbeat = true;
		} else {
			return -1;
		}
	}

	return 0;
}

static int16_t clireq_read(uint16_t net_addr, join_t *pj, tline_t *pt)
{
	payload_t	data;
	int	pipe,
			len;
	//checking if RX fifo contains data
	for (pipe = nrf24l01_prx_pipe_available(); pipe != NRF24_NO_PIPE;
					pipe = nrf24l01_prx_pipe_available()) {
		//read the data
		len = nrf24l01_prx_data(&data, sizeof(data));
		if (len == (sizeof(data.hdr)+sizeof(data.msg.join)) &&
			pipe == BROADCAST && data.hdr.net_addr == net_addr
			&& data.msg.join.hashid == pj->hashid &&
			MSG_GET(data.hdr.msg_xmn) == NRF24_JOIN_LOCAL) {
			//if the message is the answer for the join request
			if (data.msg.result != NRF24_SUCCESS)
				return (data.msg.result == NRF24_EUSERS ?
						USERS : OVERFLOW);

			m_client.pipe = kntohl(data.msg.join.data);
			m_client.net_addr = kntohs(data.hdr.net_addr);
			m_client.hashid = kntohl(data.msg.join.hashid);
			m_client.heartbeat_wait = tline_ms();
			m_client.heartbeat = false;
			nrf24l01_set_standby();
			nrf24l01_close_pipe(0);
			nrf24l01_open_pipe(0, m_client.pipe);
			//put radio in RX mode
			nrf24l01_set_prx();
			return PRX;
		}
	}

	return tline_timeout(tline_ms(), pt->start, pt->delay) ? TIMEOUT : PENDING;
}

static int16_t prx_service(uint8_t *buffer, uint16_t length)
{
	static int16_t offset = 0;
	payload_t data;
	uint8_t pipe, msg_type;
	int16_t	len;

	//checking if RX fifo contains data
	for (pipe = nrf24l01_prx_pipe_available(); pipe != NRF24_NO_PIPE;
					pipe = nrf24l01_prx_pipe_available()) {

		len = nrf24l01_prx_data(&data, sizeof(data));
		if (len > sizeof(data.hdr)) {
			pipe = m_client.pipe;

			len -= sizeof(data.hdr);
			msg_type = MSG_GET(data.hdr.msg_xmn);
			switch (msg_type) {
			case NRF24_HEARTBEAT:
				if (len != sizeof(join_t) ||
					kntohl(data.msg.join.data) != pipe ||
					data.msg.join.version.major != m_pversion->major ||
					data.msg.join.version.minor != m_pversion->minor ||
					kntohs(data.msg.join.version.packet_size) !=
										m_pversion->packet_size) {
					break;
				}
				if (MSGXMN_SET(msg_type, m_client.rxmn)  == data.hdr.msg_xmn) {
					++m_client.rxmn;
					m_client.heartbeat_wait = tline_ms();
					m_client.heartbeat = false;
				}
				break;

			case NRF24_APP_FIRST:
			case NRF24_APP_FRAG:
				if (len != NRF24_PW_MSG_SIZE)
					break;

				/* No break; fall through intentionally */
			case NRF24_APP_LAST:
			case NRF24_APP:
				if (m_client.pipe == pipe &&
					m_client.net_addr == kntohs(data.hdr.net_addr) &&
					(MSGXMN_SET(msg_type, m_client.rxmn) ==
								data.hdr.msg_xmn)) {

					++m_client.rxmn;
					if (!m_client.heartbeat)
						m_client.heartbeat_wait = tline_ms();

					if (msg_type == NRF24_APP ||
									msg_type == NRF24_APP_FIRST)
						offset = 0;

					if ((offset + len) <= m_pversion->packet_size) {
						if ((offset + len) > length)
							len = length - offset;

						if (len != 0) {
							memcpy(buffer + offset,
									&data.msg.raw, len);
							offset += len;
						}

						if (msg_type == NRF24_APP ||
							msg_type == NRF24_APP_LAST)
							return offset;

					}
				}
				break;
			}
		}
	}

	return check_heartbeat(&data.msg.join);
}

static int16_t join_local(void)
{
	int16_t state = REQUEST;
	tline_t	timer;
	join_t	join;
	data_t data;

	nrf24l01_open_pipe(0, BROADCAST);

	join.version.major = m_pversion->major;
	join.version.minor = m_pversion->minor;
	join.version.packet_size = khtons(m_pversion->packet_size);
	join.hashid = get_random_value(RAND_MAX, 1, 1);
	join.hashid ^= (get_random_value(RAND_MAX, 1, 1) * 65536);
	join.hashid = khtonl(join.hashid);
	join.data = 0;
	join.result = NRF24_SUCCESS;
	//put the values on data
	build_data(&data, BROADCAST, ((join.hashid / 65536) ^
		join.hashid), NRF24_JOIN_LOCAL);
	data.retry = get_random_value(JOINREQ_RETRY, 2, JOINREQ_RETRY);
	while (state == REQUEST || state == PENDING) {
		switch (state) {
		case REQUEST:
			timer.delay = get_random_value(SEND_RANGE_MS,
				SEND_FACTOR, SEND_RANGE_MS_MIN);
			send_data(&data, &join, sizeof(join_t));
			data.offset = 0;
			timer.start = tline_ms();
			state = PENDING;
			break;

		case PENDING:
			state = clireq_read(data.net_addr, &join, &timer);
			if (state == TIMEOUT && --data.retry != 0)
				state = REQUEST;

			break;
		}
	}

	if (state != PRX) {
		nrf24l01_set_standby();
		nrf24l01_close_pipe(0);
	}
	return state;
}

//send NRF24_UNJOIN_LOCAL message
static int16_t unjoin_local(void)
{
	join_t	join;
	data_t data;

	join.version.major = m_pversion->major;
	join.version.minor = m_pversion->minor;
	join.version.packet_size = khtons(m_pversion->packet_size);
	join.hashid = khtonl(m_client.hashid);
	join.data = m_client.pipe;
	join.result = NRF24_SUCCESS;
	return ptx_service(build_data(&data, m_client.pipe,
		khtons(m_client.net_addr), NRF24_UNJOIN_LOCAL), &join,
								sizeof(join_t));
}


int16_t nrf24l01_client_open(int16_t socket, uint8_t channel,
							version_t *pversion)
{
	int16_t ret, err;

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

	ret = join_local();
	if (ret == PRX)
		return 0;

	if (ret == TIMEOUT)
		err = ETIMEDOUT;
	else if (ret == USERS)
		err = EUSERS;
	else
		err = EOVERFLOW;


	return -err;
}

int16_t nrf24l01_client_close(int16_t socket)
{
	if (m_fd == SOCKET_INVALID || m_fd != socket)
		return -EBADF;

	if (m_client.net_addr != 0) {
		if (unjoin_local() != 0)
			return -EAGAIN;

		disconnect();
	}
	m_fd = SOCKET_INVALID;
	m_pversion = NULL;

	return 0;
}

int16_t nrf24l01_client_read(int16_t socket, uint8_t *buffer, uint16_t len)
{
	if (m_fd == SOCKET_INVALID || m_fd != socket)
		return -EBADF;

	return prx_service(buffer, len);
}

int16_t nrf24l01_client_write(int16_t socket, const uint8_t *buffer,
								uint16_t len)
{
	data_t data;

	if (m_fd == SOCKET_INVALID || m_fd != socket)
		return -EBADF;


	return ptx_service(build_data(&data, m_client.pipe,
		khtons(m_client.net_addr), NRF24_APP), (void *)buffer, len);

}
