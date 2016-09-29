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
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef ARDUINO
#include <avr_errno.h>
#include <avr_unistd.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

#include "comm_private.h"
#include "nrf24l01.h"
#include "nrf24l01_net.h"


static enum {
	PTX_FIRE,
	PTX_GAP,
	PTX
} e_state;

#define NRF24_MTU		32
#define NRF24_PIPE0		0

static int16_t send_data(int sockfd, payload_t payload, size_t len)
{
	int err;

	/* Puts the radio in TX mode - sockfd address */
	nrf24l01_set_ptx(sockfd);
	/* Transmits the data disabling Acknowledgment */
	err = nrf24l01_ptx_data(&payload, len + sizeof(hdr_t), false);
	printf("err = %d\n", err );
	if (err == NRF24_TX_FIFO_FULL)
		return -EAGAIN;
	/*
	 * The radio do not receive and send at the same time
	 * It's a good practice to put the radio in RX mode
	 * and only switch to TX mode when transmitting data.
	 */

	nrf24l01_set_prx();

	return len;
}

static payload_t frag_msg(size_t length, uint8_t *msg_type,
	const void *buffer, uint16_t offset, uint8_t txmn, size_t PW_MSG_SIZE)
{

	payload_t payload;
	size_t len  = length;

	*msg_type = APP_LAST;
	/*
	 * If the message doesn't need to be fragmented
	 * NRF24_APP_LAST is returned and the len is not modified
	 */
	if (len > PW_MSG_SIZE){
		/* If first packet */
		if (offset == 0) {
			*msg_type = APP_FIRST;
			len = PW_MSG_SIZE;
		} else {
			len -= offset;
			/* If the message still need to be fragmented */
			if (len > PW_MSG_SIZE){
				*msg_type = APP_FRAG;
				len = PW_MSG_SIZE;
			}
			/* Else, the msg_type = NRF24_APP_LAST*/
		}
	}
	/* Type send message */
	payload.hdr.msg_xmn = MSGXMN_SET(*msg_type, txmn);
	/* move buffer to payload data */
	memcpy(payload.raw, buffer + offset, len);


	return payload;
}

static int16_t ptx_service(int sockfd, const void *buffer, size_t length)
{
	int err, state = PTX_FIRE;
	uint8_t txmn, msg_type = APP;
	uint16_t offset;
	size_t len;
	unsigned long delay;
	payload_t payload;

	len = length;
	err = length;
	txmn = 0;
	offset = 0;

	while (msg_type != APP_LAST) {

		switch (state) {

		case PTX_FIRE:
			/* TODO: randomic delay*/
			delay = 100;
			state = PTX_GAP;
			/* No break; fall through intentionally */

		case PTX_GAP:
			usleep((delay)*1000);
			/* No break; fall through intentionally */

		case PTX:
			/*
			 * The frag_msg function set the
			 * msg_type [NRF24_APP_FIRST | NRF24_APP_FRAG | NRF24_APP_LAST]
			 * and return the length of message that will be sent
			 */

			payload = frag_msg(len, &msg_type, buffer, offset, txmn, NRF24_PW_MSG_SIZE);
			printf("to frag: length=%ld offset=%d msg_type=%d len=%ld sizeof=%ld\n", len, offset, msg_type, length, sizeof(payload));
			/* ERRADO-> sizeof(payload.raw - sempre ta escrevendo 29 + 3*/
			err = send_data(sockfd, payload, sizeof(payload.raw));
			printf("buff=%s offset=%d length=%ld txmn=%d\n", (char *)payload.raw, offset, length, txmn );

			if (err > 0) {
				offset += sizeof(payload.raw);
				txmn++;
			} else {
				offset = 0;
				txmn = 0;
			}

			/* If is not the last package */
			if(msg_type != APP_LAST)
				state = PTX_FIRE;

			break;

		}

	}

	printf("saiu do laco\n");

	return (err > 0 ? offset : err);
}

static int16_t read_data(int sockfd, payload_t *recdata,
		uint8_t rxmn)
{
	ssize_t length = -1;

	/* If the pipe available */
	if (nrf24l01_prx_pipe_available() == sockfd){
		/* Copy data to buffer */
		printf("sizeof = %ld\n",  sizeof(payload_t));
		length = nrf24l01_prx_data(recdata, sizeof(payload_t));
		printf("length is %ld\n", length);
		if (length < sizeof(hdr_t))
			return -ENOMSG;		/* No message of desired type */

		length -= sizeof(hdr_t);	/* Raw data length */

	}

	return length;
}


static uint16_t defrag_msg(void *buffer, size_t length, size_t len,
	payload_t data, uint8_t *msg_type, uint16_t offset, size_t PW_MSG_SIZE)
{

	uint8_t rxmn;

	*msg_type = MSG_GET(data.hdr.msg_xmn);
	printf("Message type is %d\n", *msg_type);

	switch (*msg_type) {

	case APP_FRAG:
	case APP_FIRST:
		if (len != PW_MSG_SIZE)
			break;
		/* No break; fall through intentionally */
	case APP_LAST:
		if (MSGXMN_SET(*msg_type, rxmn) == data.hdr.msg_xmn) {

			rxmn++;

			if (*msg_type == APP_FIRST)
				offset = 0;

			/*
			 * If msg size received > length passed as argument
			 * move only the length passed as argument
			 */
			if ((offset + len) > length)
				len = length - offset;

			if (len > 0){
				memcpy(buffer + offset, &data.raw, len);
				offset += len;
			}
		}
	}


	return offset;
}

static uint16_t prx_service(int sockfd, void *buffer, size_t length,
		size_t PW_MSG_SIZE)
{
	uint8_t rxmn, msg_type = APP;
	uint16_t offset;
	payload_t datarec;
	ssize_t len;

	rxmn = 0;
	offset = 0;

	while (msg_type != APP_LAST){

		len = read_data(sockfd, &datarec, rxmn);
		if (len < 0)
			return -1;

		printf("Datarec = %s and length %ld\n", datarec.raw, len);
		offset = defrag_msg(buffer, length, len, datarec, &msg_type, offset, NRF24_PW_MSG_SIZE);

		printf("offset=%d length=%ld, len=%ld, buffer=%s msg_type=%d\n", offset, length, len, (char *)buffer , msg_type);
		usleep(100*1000);
	}

	printf("saiu do laço\n");
	printf("Complete Buffer = %s\n", (char *)buffer);

}

static int nrf24l01_probe(void)
{
	return nrf24l01_init("/dev/spidev0.0");
}

static void nrf24l01_remove(void)
{

}

static int nrf24l01_open(const char *pathname)
{
	/*
	 * Considering 16-bits to address adapter and logical
	 * channels. The most significant 4-bits will be used to
	 * address the local adapter, the remaining (12-bits) will
	 * used to address logical channels(clients/pipes).
	 * eg: For nRF24 '0' will be mapped to pipe0.
	 * TODO: Implement addressing
	 */

	return 0;
}

static ssize_t nrf24l01_recv(int sockfd, void *buffer, size_t len)
{

	/* TODO: check if the data received is fragmented or not */
	ssize_t length;

	length = prx_service(sockfd, buffer, len, NRF24_PW_MSG_SIZE);
	/*
	 * On success, the number of bytes read is returned
	 * Otherwise, -1 is returned.
	 */
	return length;
}

static ssize_t nrf24l01_send(int sockfd, const void *buffer, size_t len)
{
	int err;

	err = ptx_service(sockfd, buffer, len);
	/*
	 * On success, the number of bytes written is returned
	 * Otherwise, -1 is returned.
	 */
	return err;
}

static int nrf24l01_listen(int sockfd)
{
	if (sockfd < 0 || sockfd > NRF24_PIPE_ADDR_MAX)
		return -EINVAL;

	/* Set channel */
	if (nrf24l01_set_channel(NRF24_CHANNEL_DEFAULT) == -1)
		return -EINVAL;

	/* Open pipe zero in the sockfd address */
	nrf24l01_open_pipe(NRF24_PIPE0, sockfd);

	/*
	 * Standby mode is used to minimize average
	 * current consumption while maintaining short
	 * start up times. In this mode only
	 * part of the crystal oscillator is active.
	 * FIFO state: No ongoing packet transmission.
	 */
	nrf24l01_set_standby();

	/* Put the radio in RX mode to start receiving packets.*/
	nrf24l01_set_prx();

	return 0;
}

struct phy_driver nrf24l01 = {
	.name = "nRF24L01",
	.probe = nrf24l01_probe,
	.remove = nrf24l01_remove,
	.open = nrf24l01_open,
	.recv = nrf24l01_recv,
	.send = nrf24l01_send,
	.listen = nrf24l01_listen
};
