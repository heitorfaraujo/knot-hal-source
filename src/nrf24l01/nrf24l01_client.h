#ifndef ARDUINO
#include <stdlib.h>
#include <errno.h>
#endif

#ifndef __NRF24L01_CLIENT_H__
#define __NRF24L01_CLIENT_H__

#ifdef __cplusplus
extern "C"{
#endif
//send join request to gateway
int16_t nrf24l01_client_open(int16_t socket, uint8_t channel,
							version_t *pversion);
//send unjoin to gateway
int16_t nrf24l01_client_close(int16_t socket);
int16_t nrf24l01_client_read(int16_t socket, uint8_t *buffer,
								uint16_t len);
int16_t nrf24l01_client_write(int16_t socket, const uint8_t *buffer,
								uint16_t len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __NRF24L01_CLIENT_H__
