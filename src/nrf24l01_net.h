/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

enum {
	APP, 			/* Send packet */
	APP_FIRST,		/* First packet */
	APP_FRAG,		/* Fragmented packet */
	APP_LAST		/* Last packet */
};


/* Mask to transmission number */
#define XMNMASK						0b11110000
/* Mask to network messsage */
#define MSGMASK						0b00001111
/* Set message type and sequence number */
#define MSGXMN_SET(m,x)	((((x) << 4) & XMNMASK) | ((m) & MSGMASK))
/* Get message type*/
#define MSG_GET(v)					((v) & MSGMASK)

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

#define NRF24_PW_MSG_SIZE				(NRF24_PAYLOAD_SIZE - sizeof(hdr_t))

/**
 * union payload_t - defines a network layer payload
 * @hdr: net layer message header
 * @result: process result
 * @join: net layer join local message
 * @raw: raw data of network layer
 *
 * This union defines the network layer payload.
 */
typedef struct __attribute__ ((packed))  {
	hdr_t			hdr;
	uint8_t	raw[NRF24_PW_MSG_SIZE];

} payload_t;
