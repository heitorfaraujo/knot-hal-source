/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

// network messages
enum {
	NRF24_APP,			/* Send message */
	NRF24_APP_FIRST,	/* First packet */
	NRF24_APP_FRAG,		/* Fragmented packet */
	NRF24_APP_LAST		/* Last packet */
};

/**
 * struct hdr_t - net layer message header
 * @net_addr: net address
 * @msg_type: message type
 * @offset: message fragment offset
 *
 * This struct defines the network layer message header
 */
typedef struct __attribute__ ((packed)) {
	uint16_t	net_addr;
	uint8_t		msg_xmn;
} hdr_t;
