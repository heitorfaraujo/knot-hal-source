/*
 * Copyright (c) 2015, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#ifdef ARDUINO
#include <Arduino.h>
 /* functions for Endianness */
#define khtons(hostshort)	((uint16_t)((((hostshort) >> 8) & 0xff)
				| (((hostshort) & 0xff) << 8)))

#define khtonl(hostlong)	((uint32_t)((hostlong >> 24)
						| ((hostlong<<8) & 0x00FF0000)
						| ((hostlong>>8) & 0x0000FF00)
						| (hostlong << 24)))


#define kntohs(netshort)	khtons(netshort)
#define kntohl(netlong)		khtonl(netlong)
#else
#define khtons(hostshort)	htons((uint16_t)hostshort)
#define khtonl(hostlong)	htonl((uint32_t)hostlong)
#define kntohs(netshort)	khtons(netshort)
#define kntohl(netlong)		khtonl(netlong)
#endif

// invalid socket fd
#define SOCKET_INVALID		-1

 /**
 * struct phy_driver - driver abstraction for the physical layer
 * @name: driver name
 * @probe: function to initialize the driver
 * @remove: function to close the driver and free its resources
 * @socket: function to create a new socket FD
 * @listen: function to enable incoming connections
 * @accept: function that waits for a new connection and returns a 'pollable' FD
 * @connect: function to connect to a server socket
 *
 * This 'driver' intends to be an abstraction for Radio technologies or
 * proxy for other services using TCP or any socket based communication.
 */

struct phy_driver {
	const char *name;
	int domain;		/* Protocol domain: Radio nRF24L01, nRF905, ... */
	int (*probe) (size_t packet_size);
	void (*remove) (void);

	int (*socket) (int type, int protocol);
	int (*close) (int sockfd);
	int (*listen) (int srv_sockfd);
	int (*accept) (int srv_sockfd);
	int (*connect) (int cli_sockfd, uint8_t to_addr, size_t len);
	int (*recv) (int sockfd, void *buffer, size_t len);
	size_t (*send) (int sockfd, const void *buffer, size_t len);
};
