/*
 * Copyright (c) 2015, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef ARDUINO
#include "avr_errno.h"
#include "avr_unistd.h"
#else
#include <errno.h>
#include <unistd.h>
#endif

#include "comm_private.h"
#include "phy_driver_nrf24.h"
#include "nrf24l01.h"
#include "nrf24l01_ll.h"

/* Access Address for each pipe */
static uint8_t aa_pipes[6][5] = {
  {0x8D, 0xD9, 0xBE, 0x96, 0xDE},
  {0x6B, 0x96, 0xB6, 0xC1, 0x35},
  {0x6B, 0x96, 0xB6, 0xC1, 0x77},
  {0x6B, 0x96, 0xB6, 0xC1, 0xD3},
  {0x6B, 0x96, 0xB6, 0xC1, 0xE7},
  {0x6B, 0x96, 0xB6, 0xC1, 0xF0}
};

static ssize_t nrf24l01_write(int sockfd, const void *buffer, size_t len)
{
	int err;
	/* Puts the radio in TX mode  enabling Acknowledgment */
	nrf24l01_set_ptx(aa_pipes[sockfd], true);

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

static ssize_t nrf24l01_read(int sockfd, void *buffer, size_t len)
{
	ssize_t length = -1;

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
	int err;
	/*
	 * Considering 16-bits to address adapter and logical
	 * channels. The most significant 4-bits will be used to
	 * address the local adapter, the remaining (12-bits) will
	 * used to address logical channels(clients/pipes).
	 * eg: For nRF24 '0' will be mapped to pipe0.
	 * TODO: Implement addressing
	 */

	err = nrf24l01_init(pathname);
	if (err < 0)
		return err;

	return 0;
}

static void nrf24l01_close(int sockfd)
{
	// passar fd spi
	nrf24l01_deinit();
}

static int nrf24l01_ioctl(int sockfd, int cmd, void *arg)
{
	int err = 0;
	struct addr_pipe *addrpipe  = (struct addr_pipe*) arg;

	switch (cmd) {

	case CMD_SET_PIPE:
		//passar fd
		nrf24l01_open_pipe(addrpipe->pipe, addrpipe->aa);
		break;

	case CMD_SET_CHANNEL:
		//passar fd
		err = nrf24l01_set_channel(*((int *) arg));
		break;

	default:
		err = -1;
	}

	return err;
}

struct phy_driver nrf24l01 = {
	.name = "NRF0",
	.pathname = "/dev/spidev0.0",
	.open = nrf24l01_open,
	.recv = nrf24l01_read,
	.send = nrf24l01_write,
	.ioctl = nrf24l01_ioctl,
	.close = nrf24l01_close,
	.ref_open = 0,
	.fd = -1,
	.aa = NRF24_AA_MGMT_CHANNEL
};
