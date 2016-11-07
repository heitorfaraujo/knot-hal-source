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
#include <stdio.h>

#ifdef ARDUINO
#include "avr_errno.h"
#include "avr_unistd.h"
#else
#include <errno.h>
#include <unistd.h>
#endif

#include "comm_private.h"
#include "phy_driver.h"

struct phy_driver *driver_ops[] = {
	&nrf24l01
};

#define PHY_DRIVERS_COUNTER	((int) (sizeof(driver_ops) / sizeof(driver_ops[0])))

inline int phy_open(const char *pathname)
{
	uint8_t i;
	int err, sockfd = -1;

	for (i = 0; i < PHY_DRIVERS_COUNTER; ++i) {
		if (strcmp(pathname, driver_ops[i]->name) == 0)
			sockfd = i;
	}

	if (sockfd == -1)
		return sockfd;

	if (driver_ops[sockfd]->ref_open == 0) {
		err = driver_ops[sockfd]->open(driver_ops[sockfd]->pathname);
		if (err < 0)
			return err;

		driver_ops[sockfd]->fd = err;
	}

	++driver_ops[sockfd]->ref_open;

	return sockfd;
}

inline int phy_close(int sockfd)
{
	if (sockfd < 0 || sockfd > PHY_DRIVERS_COUNTER)
		return -1;

	if (driver_ops[sockfd]->ref_open != 0) {
		--driver_ops[sockfd]->ref_open;
		if (driver_ops[sockfd]->ref_open == 0) {
			driver_ops[sockfd]->close(driver_ops[sockfd]->fd);
			driver_ops[sockfd]->fd = -1;
		}
	}

	return 0;
}

inline int phy_listen(int srv_sockfd)
{

	return -ENOSYS;
}

inline int phy_accept(int srv_sockfd)
{
	return -ENOSYS;
}

inline int phy_connect(int cli_sockfd, uint8_t to_addr)
{
	return -ENOSYS;
}

inline ssize_t phy_read(int sockfd, void *buffer, size_t len)
{
	return driver_ops[sockfd]->recv(driver_ops[sockfd]->fd, buffer, len);
}

inline ssize_t phy_write(int sockfd, const void *buffer, size_t len)
{

	return driver_ops[sockfd]->send(driver_ops[sockfd]->fd, buffer, len);

}

inline int phy_ioctl(int sockfd, int cmd, void *args)
{

	return driver_ops[sockfd]->ioctl(driver_ops[sockfd]->fd, cmd, args);
}
