/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

/*
 * Build instructions while a Makefile is not available:
 * gcc $(pkg-config --libs --cflags glib-2.0) -Iinclude -Itools \
 * tools/rpiecho.c -o tools/rpiechod
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdbool.h>

#include "spi.h"
#include "nrf24l01.h"

#define MAXRETRIES 20

enum states {
	SEND,
	RECEIVE
};

static char *opt_mode = "server";

uint8_t bufferRx[32];

static GMainLoop *main_loop;

static void sig_term(int sig)
{
	g_main_loop_quit(main_loop);
	nrf24l01_deinit();

}

static void printBuffer(uint8_t *buffer, ssize_t len)
{
	int index;

	for (index = 0; index < len; index++) {
		if (index % 8 == 0)
			printf("\n");

		printf("0x%02x ", buffer[index]);
	}
	printf("\n");

}

static void timeout_destroy(gpointer user_data)
{
	g_main_loop_quit(main_loop);

}

static gboolean timeout_watch_client(gpointer user_data)
{

	/*
	 * This function gets called frequently to verify
	 * if there is SPI/nRF24L01 data to be read or
	 * written. Later GPIO sys interface can be used
	 * to avoid busy loop.
	 */
	uint8_t buffer[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x00, 0x00, 0x00, 0x00
	};

	static uint32_t counter = 0;
	static enum states state = SEND;
	static int err = 0;
	int retries;

	switch (state) {

	case SEND: /* Send the message */
		counter++;

		buffer[28] = counter >> 24;
		buffer[29] = counter >> 16;
		buffer[30] = counter >> 8;
		buffer[31] = counter;

		printf("\nClient Mode -> Send Message\n");

		printBuffer(buffer, 32);
		memcpy(bufferRx, buffer, sizeof(buffer));

		retries = 0;

		do {
			/* Open pipe 0 enabling ACK */
			nrf24l01_set_ptx(0, true);
			/* Sends the data */
			nrf24l01_ptx_data(&buffer, sizeof(buffer));
			/* Waits for ACK - returns 0 on succes */
			err = nrf24l01_ptx_wait_datasent();

			if (err != 0)
				memcpy(buffer, bufferRx, sizeof(bufferRx));

			if (retries > MAXRETRIES) {
				printf("Could not send the message\n");
				return FALSE;
			}

			retries++;
		/* Loop until receive ACK */
		} while (err != 0);

		nrf24l01_set_prx();
		state = RECEIVE;
		printf("Waiting for response...\n");
		break;

	case RECEIVE: /* Receive the message sent*/

		/* If data available in pipe 0 */
		if (nrf24l01_prx_pipe_available() == 0) {

			/* Reads the data */
			nrf24l01_prx_data(&buffer, sizeof(buffer));

			/* Compares the message */
			if (memcmp(buffer, bufferRx, sizeof(bufferRx)) == 0)
				printf("Correct Message Received\n");
			else {
				printf("Different Message Received\n");
				return -1;
			}

			/* Go to SEND state */
			state = SEND;
		}
		break;
	}

	return TRUE;
}

static gboolean timeout_watch_server(gpointer user_data)
{

	static enum states state = RECEIVE;
	static int err = 0;
	static uint8_t buffer[32];
	int retries;
	uint16_t rxlen;

	switch (state) {

	case SEND:	/* Send the message received*/

		printf("\nServer Mode -> Send Message");
		/* Print the message received */
		printBuffer(buffer, 32);

		memcpy(bufferRx, buffer, sizeof(buffer));

		retries = 0;

		do {
			nrf24l01_set_ptx(0, true);
			nrf24l01_ptx_data(&buffer, sizeof(buffer));
			err = nrf24l01_ptx_wait_datasent();

			if (err != 0)
				memcpy(buffer, bufferRx, sizeof(bufferRx));

			if (retries > MAXRETRIES) {
				printf("Could not send the message\n");
				return FALSE;
			}

			retries++;

		/* Loop until receive the ACK */
		} while (err != 0);

		state = RECEIVE;
		nrf24l01_set_prx();

		break;

	case RECEIVE: /* Receive the messagem from client */

		if (nrf24l01_prx_pipe_available() == 0) {

			printf("\nServer Mode -> Message Received ");

			rxlen = nrf24l01_prx_data(&buffer, sizeof(buffer));
			printBuffer(buffer, rxlen);

			state = SEND;
		}
		break;

	}

	return TRUE;
}

static int radio_init(void)
{
	printf("Radio init\n");
	/* Init the nrf24l01+ */
	nrf24l01_init("/dev/spidev0.0");
	/* Set the operation channel - Channel default = 10 */
	nrf24l01_set_channel(NRF24_CHANNEL_DEFAULT);
	nrf24l01_set_standby();
	/* Open pipe zero */
	nrf24l01_open_pipe(0, 0);
	/* Put the radio in RX mode to start receive packets */
	nrf24l01_set_prx();

	return 0;
}

static GOptionEntry options[] = {
	{ "mode", 'm', 0, G_OPTION_ARG_STRING, &opt_mode,
					"mode", "Operation mode: server or client" },
	{ NULL },
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *gerr = NULL;
	int err;
	signal(SIGTERM, sig_term);
	signal(SIGINT, sig_term);
	signal(SIGPIPE, SIG_IGN);

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (!g_option_context_parse(context, &argc, &argv, &gerr)) {
		printf("Invalid arguments: %s\n", gerr->message);
		g_error_free(gerr);
		g_option_context_free(context);
		return EXIT_FAILURE;
	}

	g_option_context_free(context);

	main_loop = g_main_loop_new(NULL, FALSE);

	printf("RPi nRF24L01 Radio test tool %s mode\n", opt_mode);
	err = radio_init();
	if (err < 0) {
		g_main_loop_unref(main_loop);
		return EXIT_FAILURE;
	}

	if (strcmp(opt_mode, "client") == 0)
		g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, 1,
			timeout_watch_client, NULL, timeout_destroy);
	else
		g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, 1,
			timeout_watch_server, NULL, timeout_destroy);

	g_main_loop_run(main_loop);

	g_main_loop_unref(main_loop);

	return EXIT_SUCCESS;
}
