/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
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

enum states {
	SEND,
	RECEIVE
};

static char *opt_mode = "server";

static GMainLoop *main_loop;

static void sig_term(int sig)
{
	g_main_loop_quit(main_loop);
	nrf24l01_deinit();

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
	int index;
	static enum states state = SEND;

	switch (state) {

	case SEND: /* Send the message */

		counter++;

		buffer[28] = counter >> 24;
		buffer[29] = counter >> 16;
		buffer[30] = counter >> 8;
		buffer[31] = counter;

		for (index = 0; index < 32; index++) {
			if (index % 8 == 0)
			printf("\n");

			printf("0x%02x ", buffer[index]);
		}

		/* Send the Buffer here */

		state = RECEIVE;
		break;

	case RECEIVE:

		/* Receives the message sent here */
		state = SEND;
		break;
	}

	return TRUE;
}

static gboolean timeout_watch_server(gpointer user_data)
{

	static enum states state = RECEIVE;

	/*
	 * The server receive the message from the
	 * client and send back the message received.
	 */

	switch (state) {

	case SEND:

		/* Send the received message here */
		state = RECEIVE;
		break;

	case RECEIVE:

		/* Receive the messagem from client here */
		state = SEND;
		break;
	}

	return TRUE;
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
	int timeout_id;
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

	if (strcmp(opt_mode, "client") == 0)
		timeout_id = g_timeout_add_seconds(1,
						timeout_watch_client, NULL);
	else
		timeout_id = g_timeout_add_seconds(1,
						timeout_watch_server, NULL);

	g_main_loop_run(main_loop);

	g_source_remove(timeout_id);

	g_main_loop_unref(main_loop);

	return 0;
}
