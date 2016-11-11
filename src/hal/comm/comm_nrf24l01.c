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
#include <string.h>

#ifdef ARDUINO
#include <avr_errno.h>
#include <avr_unistd.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

#include "include/comm.h"
#include "phy_driver.h"
#include "nrf24l01_ll.h"

#define KNOT_DATA_SIZE 128

struct nrf24_ctrl {
	uint8_t buffer_rx[30];
	size_t len_rx;
	uint8_t buffer_tx[30];
	size_t len_tx;
};

struct nrf24_data {
	int8_t pipe;
	uint8_t buffer_rx[KNOT_DATA_SIZE];
	size_t len_rx;
	uint8_t buffer_tx[KNOT_DATA_SIZE];
	size_t len_tx;
};

struct nrf24_ctrl mgmt_ctrl = {.len = 0};

#ifndef ARDUINO
struct nrf24_data my_things[5] = {
	{.pipe = -1, .len = 0 },
	{.pipe = -1, .len = 0 },
	{.pipe = -1, .len = 0 },
	{.pipe = -1, .len = 0 },
	{.pipe = -1, .len = 0 }
};
#else
struct nrf24_data my_things[1] = {
	{.pipe = -1, .len = 0 },
};
#endif

/* ARRAY SIZE */
#define CONNECTION_COUNTER	((int) (sizeof(my_things) \
				 / sizeof(my_things[0])))


static uint8_t aa_pipes[6][5] = {
	{0x8D, 0xD9, 0xBE, 0x96, 0xDE},
	{0x35, 0x96, 0xB6, 0xC1, 0x6B},
	{0x77, 0x96, 0xB6, 0xC1, 0x6B},
	{0xD3, 0x96, 0xB6, 0xC1, 0x6B},
	{0xE7, 0x96, 0xB6, 0xC1, 0x6B},
	{0xF0, 0x96, 0xB6, 0xC1, 0x6B}
};

int driverIndex = -1;

/* Local functions */
static int get_free_pipe(void)
{
	uint8_t i;

	for (i = 0; i < CONNECTION_COUNTER; i++) {
		if (my_things[i].pipe == -1) {
			/* one peer for pipe*/
			my_things[i].pipe = i+1;
			return my_things[i].pipe;
		}
	}

	/* No free pipe */
	return -1;
}

static ssize_t read_data(int sockfd, void *buffer, size_t count)
{
	ssize_t ilen;
	size_t plen;
	uint8_t lid, datagram[NRF24_MTU];
	const struct nrf24_ll_data_pdu *ipdu = (void *)datagram;
	static size_t offset = 0;
	static uint8_t seqnumber = 0;

	do {

		/*
		 * Reads the data,
		 * on success, the number of bytes read is returned
		 */
		ilen = phy_read(sockfd, datagram, NRF24_MTU);

		if (ilen < 0)
			return -EAGAIN;	/* Try again */

		if (seqnumber != ipdu->nseq)
			return -EILSEQ; /* Illegal byte sequence */

		if (seqnumber == 0)
			offset = 0;

		/* Payloag length = input length - header size */
		plen = ilen - DATA_HDR_SIZE;
		lid = ipdu->lid;

		if (lid == NRF24_PDU_LID_DATA_FRAG && plen < NRF24_PW_MSG_SIZE)
			return -EBADMSG; /* Not a data message */

		/* Reads no more than len bytes */
		if (offset + plen > count)
			plen = count - offset;

		memcpy(buffer + offset, ipdu->payload, plen);
		offset += plen;
		seqnumber++;

	} while (lid != NRF24_PDU_LID_DATA_END && offset < count);

	/* If the complete msg is received, resets the sequence number */
	seqnumber = 0;

	/* Returns de number of bytes received */
	return offset;
}

static int index_sock = 0;

static void running(void)
{
	if (index_sock == 0) {
		phy_ioctl();
		/* READ */
		mgmt_ctrl ...
		/* WRITE */
	} else {
		phy_ioctl();
		/* READ */
		my_things ...
		/* WRITE */

	}
	++index_sock;
}

/* Global functions */

int hal_comm_init(const char *pathname)
{
	if (driverIndex != -1)
		return -EPERM;

	driverIndex = phy_open(pathname);
	if (driverIndex < 0)
		return driverIndex;

	return 0;
}

int hal_comm_deinit()
{
	if (driverIndex == -1)
		return -EPERM;

	return phy_close(driverIndex);
}

int hal_comm_socket(int domain, int protocol)
{
	int retval;
	struct addr_pipe ap;

	if (driverIndex == -1)
		return -EPERM;	/* Operation not permitted */

	switch (protocol) {

	case HAL_COMM_PROTO_MGMT:

		ap.ack = false;
		retval = 0;
		break;

	case HAL_COMM_PROTO_RAW:

		retval = get_free_pipe();
		if (retval < 0)
			return - EUSERS;

		ap.ack = true;
		break;

	default:
		return -EINVAL;  /* Invalid argument */
	}

	ap.pipe = retval;
	memcpy(ap.aa, aa_pipes[retval], sizeof(aa_pipes[retval]));

	phy_ioctl(driverIndex, CMD_SET_PIPE, &ap);

	return retval;
}

void hal_comm_close(int sockfd)
{
	if (driverIndex == -1)
		return -EPERM;

	if (sockfd >=1 && sockfd <= 5) {
		/* Free pipe */
		my_things[sockfd-1].pipe = -1;
		phy_ioctl(driverIndex, CMD_RESET_PIPE, sockfd);
	}
}

ssize_t hal_comm_read(int sockfd, void *buffer, size_t count)
{
	size_t length = 0;

	/* Run background procedures */
	running();

	if (sockfd < 0 || sockfd > 5 || count == 0)
		return -EINVAL;

	if (sockfd == 0) {
		if (mgmt_ctrl.len_rx != 0){
			length = mgmt_ctrl.len_rx > count ? count : mgmt_ctrl.len_rx;
			memcpy(buffer, mgmt_ctrl.buffer_rx, length);
			mgmt_ctrl.len_rx = 0;
		}
	} else if (my_things[sockfd-1].len_rx != 0){
		length = my_things[sockfd-1].len_rx > count ?
				 count : my_things[sockfd-1].len_rx;
		memcpy(buffer, my_things[sockfd-1].buffer_rx, length);
		my_things[sockfd-1].len_rx = 0;
	}

	return length;
}

ssize_t hal_comm_write(int sockfd, const void *buffer, size_t count)
{
	/* Run background procedures */
	running();

	if (sockfd < 1 || sockfd > 5 || count == 0 || count > KNOT_DATA_SIZE)
		return -EINVAL;

	if (my_things[sockfd-1].len_tx != 0)
		return -EBUSY;

	memcpy(my_things[sockfd-1].buffer_tx, buffer, count);
	my_things[sockfd-1].len_tx = count;

	return count;
}

int hal_comm_listen(int sockfd)
{
	/* Run background procedures */
	running();


	return -ENOSYS;
}

int hal_comm_accept(int sockfd, uint64_t *addr)
{
	/* Run background procedures */
	running();


	return -ENOSYS;
}


int hal_comm_connect(int sockfd, uint64_t *addr)
{
	return -ENOSYS;
}
