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

#ifdef ARDUINO
#include <avr_errno.h>
#include <avr_unistd.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

#include "include/comm.h"
#include "phy_driver.h"

int8_t pipes_allocate[] = {0, 0, 0, 0, 0};

int driverIndex = -1;

/* Local functions */
inline int get_free_pipe(void)
{
	uint8_t i;

	for (i = 0; i < sizeof(pipes_allocate); i++) {
		if (pipes_allocate[i] == 0) {
			/* one peer for pipe*/
			pipes_allocate[i] = 1;
			return i+1;
		}
	}

	/* No free pipe */
	return -1;
}

int hal_comm_socket(int domain, int protocol)
{
	int retval;

	switch (protocol) {

	case HAL_COMM_PROTO_MGMT:
		driverIndex = phy_open("NRF0");
		if (driverIndex < 0)
			return driverIndex;

		retval = 0;
		break;

	case HAL_COMM_PROTO_RAW:

		if (driverIndex == -1)
			return -EPERM;	/* Operation not permitted */

		retval = get_free_pipe();
		if (retval < 0)
			return - EUSERS;
		break;

	default:
		retval = -EINVAL;  /* Invalid argument */
		break;
	}

	return retval;
}

void hal_comm_close(int sockfd)
{
	if (sockfd > 1 && sockfd < 5) {
		/* Free pipe */
		pipes_allocate[sockfd-1] = 0;

		/* Close driver */
		phy_close(driverIndex);
	}

}

ssize_t hal_comm_read(int sockfd, void *buffer, size_t count)
{

	return -ENOSYS;
}

ssize_t hal_comm_write(int sockfd, const void *buffer, size_t count)
{

	return -ENOSYS;
}

int hal_comm_listen(int sockfd)
{

	return -ENOSYS;
}

int hal_comm_accept(int sockfd, uint64_t *addr)
{

	return -ENOSYS;
}


int hal_comm_connect(int sockfd, uint64_t *addr)
{

	return -ENOSYS;
}
