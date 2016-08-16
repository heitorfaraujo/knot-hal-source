/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */
//necessary to compile in arduino
#ifdef __cplusplus
extern "C"{
#endif

#ifndef	ARDUINO
int spi_init(const char *dev);
#else
int spi_init(void);
#endif

int spi_transfer(void *tx, uint16_t ltx, void *rx, uint16_t lrx);
void spi_deinit(void);

#ifdef __cplusplus
} // extern "C"
#endif