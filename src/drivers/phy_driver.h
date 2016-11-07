/*
 * Copyright (c) 2015, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#ifndef __PHY_DRIVER__
#define __PHY_DRIVER__

int phy_open(const char *pathname);
int phy_close(int sockfd);
int phy_listen(int srv_sockfd);
int phy_accept(int srv_sockfd);
int phy_connect(int cli_sockfd, uint8_t to_addr);
ssize_t phy_read(int sockfd, void *buffer, size_t len);
ssize_t phy_write(int sockfd, const void *buffer, size_t len);
int phy_ioctl(int sockfd, int cmd, void *args);


#endif
