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
#include <unistd.h>

#ifdef ARDUINO
#include <avr_errno.h>
#include <avr_unistd.h>
#else
#include <errno.h>
#endif

#include "comm_private.h"
#include "nrf24l01.h"

#define NRF24_MTU		32
#define NRF24_PIPE0		0

/* Logical channel ids */
#define PDU_LID_DATA_FRAG	0x00 /* Data: Beginning or fragment */
#define PDU_LID_DATA_END	0x01 /* Data: End of fragment or complete */
#define PDU_LID_CONTROL		0x03 /* Control */

struct ll_data_channel_pdu {
	uint8_t lid:2;	/* 00 (data frag), 01 (data complete), 11: (control) */
	uint8_t pm:2;	/* Protocol: 00 (raw), 01 (X), 10(Y), 11 (Z) */
	uint8_t rfu:4;  /* RFU for Acknowledgment and Flow Control */

	uint8_t payload[0];
} __attribute__ ((packed));

#define DATA_HDR_SIZE		sizeof(struct ll_data_channel_pdu)
#define NRF24_PW_MSG_SIZE	(NRF24_MTU - DATA_HDR_SIZE)

/* 4 bits (0 - 15) are destinated to sequence number */
#define NRF24_MAX_MSG_SIZE (16 * NRF24_PW_MSG_SIZE)

static int nrf24l01_probe(void)
{
	return nrf24l01_init("/dev/spidev0.0");
}

static void nrf24l01_remove(void)
{

}

static ssize_t send_data(int sockfd, const void *buffer, size_t len)
{
	int err;
	/* Puts the radio in TX mode  enabling Acknowledgment */
	nrf24l01_set_ptx(sockfd, true);

	/* Transmits the data */
	nrf24l01_ptx_data((void *)buffer, len);

	/* Waits for ACK */
	err = nrf24l01_ptx_wait_datasent();

	if (err < 0)
		return err;
	/*
	 * The radio do not receive and send at the same time
	 * It's a good practice to put the radio in RX mode
	 * and only switch to TX mode when transmitting data.
	 */

	nrf24l01_set_prx();

	/*
	 * On success, the number of bytes written is returned
	 * Otherwise, -1 is returned.
	 */

	return len;
}

static ssize_t read_data(int sockfd, void *buffer, size_t len)
{
	ssize_t length = -1;

	/* TODO: check if the data received is fragmented or not */

	/* If the pipe available */
	if (nrf24l01_prx_pipe_available() == sockfd)
		/* Copy data to buffer */
		length = nrf24l01_prx_data(buffer, len);

	/*
	 * On success, the number of bytes read is returned
	 * Otherwise, -1 is returned.
	 */

	return length;
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

	return read_data(sockfd, buffer, len);
}

static ssize_t nrf24l01_send(int sockfd, const void *buffer, size_t len)
{
	int err;
	uint8_t seqnumber, datagram[NRF24_MTU];
	struct ll_data_channel_pdu *opdu = (void *)datagram;
	size_t plen, left;

	/* If len is larger than the maximum message size */
	if (len > NRF24_MAX_MSG_SIZE)
		return -1;	/* FIX: erro */

	left = len;
	seqnumber = 0;	/* Packet sequence number */

	while (left) {

		/* Delay to avoid sending all packets too fast */
		usleep(512);
		/*
		 * If left is larger than the NRF24_PW_MSG_SIZE,
		 * payload length = NRF24_PW_MSG_SIZE,
		 * if not, payload length = left
		 */
		plen = _MIN(left, NRF24_PW_MSG_SIZE);

		/*
		 * If left is larger than the NRF24_PW_MSG_SIZE,
		 * it means that the packet is fragmented,
		 * if not, it means that it is the last packet.
		 */
		opdu->lid = (left > NRF24_PW_MSG_SIZE) ?
				PDU_LID_DATA_FRAG : PDU_LID_DATA_END;

		/* Rfu is being used to save the sequence number.*/
		opdu->rfu = seqnumber;

		/* Offset = len - left */
		memcpy(opdu->payload, buffer + (len - left), plen);

		/* Send packet */
		err = send_data(sockfd, datagram, plen + DATA_HDR_SIZE);
		if (err < 0)
			return err;

		left -= plen;
		seqnumber++;

	}

	return len;
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
