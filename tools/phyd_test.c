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
#include <stdbool.h>
#include <unistd.h>
#include <glib.h>

#include "comm_private.h"
#include "phy_driver.h"
#include "nrf24l01.h"
#include "nrf24l01_ll.h"

static char *opt_mode = "server";

static GMainLoop *main_loop;

int cli_fd = -1;
int quit = 0;

static uint8_t aa[5] = {0x8D, 0xD9, 0xBE, 0x96, 0xDE};


static void sig_term(int sig)
{
	quit = 1;
	nrf24l01_deinit();
	g_main_loop_quit(main_loop);
}


static gboolean timeout_watch_client(gpointer user_data)
{
	int err;

	char buffer[] = {"asdasdasdasdasdasdasdas"};

	err = phy_write(0, buffer, 32);

	if (err < 0)
		return FALSE;

	return TRUE;
}


static void timeout_watch_server()
{
	char buffer[32];

	while (!quit) {
		if (phy_read(0, buffer, 32) > 0) {
			printf("Message received\nBuffer = %s\n", buffer);
		}
	}

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
	int timeout_id=0;

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

	cli_fd = phy_open("NRF0");
	if (cli_fd < 0) {
		printf("error open");
		return EXIT_FAILURE;
	}
	//---------------------------------
	nrf24l01_set_channel(10);
	nrf24l01_set_standby();
	nrf24l01_open_pipe(0, aa);
	nrf24l01_set_prx();
	//---------------------------------
	if (cli_fd < 0) {
		g_main_loop_unref(main_loop);
		return -1;
	}

	g_option_context_free(context);

	main_loop = g_main_loop_new(NULL, FALSE);

	printf("RPi nRF24L01 Radio test tool mode %s\n", opt_mode);

	if (strcmp(opt_mode, "server") == 0){
		timeout_watch_server();

	}else{
		// timeout_id = g_timeout_add_seconds(1, timeout_watch_client, NULL);
		timeout_id = g_timeout_add_seconds(1, timeout_watch_client, NULL);
	}

	printf("%d\n", timeout_id);

	g_main_loop_run(main_loop);

	// g_source_remove(timeout_id);

	g_main_loop_unref(main_loop);

	return 0;
}
