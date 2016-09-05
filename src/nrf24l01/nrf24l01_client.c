/*
* Copyright (c) 2015, CESAR.
* All rights reserved.
*
* This software may be modified and distributed under the terms
* of the BSD license. See the LICENSE file for details.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "nrf24l01-mac.h"
#include "nrf24l01_client.h"

int16_t nrf24l01_client_open(int16_t socket, uint8_t channel,
							version_t *pversion)
{
	return -1;
}

int16_t nrf24l01_client_close(int16_t socket)
{
	return -1;
}

int16_t nrf24l01_client_read(int16_t socket, uint8_t *buffer, uint16_t len)
{
	return -1;
}

int16_t nrf24l01_client_write(int16_t socket, const uint8_t *buffer,
								uint16_t len)
{
	return -1;
}
