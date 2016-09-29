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

#include <glib.h>
#include "comm_private.h"
#include "nrf24l01.h"

static char *opt_mode = "server";

static struct phy_driver *driver = &nrf24l01;

static GMainLoop *main_loop;

int cli_fd = -1;
int quit = 1;
static void sig_term(int sig)
{
	quit = 0;
	g_main_loop_quit(main_loop);

}


static gboolean timeout_watch_client()
{

	// static uint32_t counter = 0;
	int err, retries;

	// int8_t buffer[] = {
	// 	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	// 	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	// 	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	// 	0x18, 0x19, 0x1A, 0x1B, 0x00, 0x00, 0x00, 0x00
	// };

	// counter++;

	// buffer[28] = counter >> 24;
	// buffer[29] = counter >> 16;
	// buffer[30] = counter >> 8;
	// buffer[31] = counter;

	// for (index = 0; index < 32; index++) {
	// 	if (index % 8 == 0)
	// 		printf("\n");

	// 	printf("0x%02x ", buffer[index]);
	// }

	char buffer[] = {"abcdefghijklmnopqrstuvwxyz1234567890knotdabcdefghijklmnopqrstuvwxyz1234567890knotdabcdefghijklmnopqrstuvwxyz1234567890"};
	err = 0;
	retries = 0;
	printf("buffer sended: %s\nsizeof=%ld\n", buffer, sizeof(buffer));
	// do {
	err = driver->send(cli_fd, buffer, sizeof(buffer));
	retries++;

	if (err == 0)
		printf("\nMessage sended\n");

	// } while(err != 0 && retries < 20);
	if (err < 0)
		printf("Error sending message\n");

	return TRUE;
}


static gboolean timeout_watch_server()
{
	int8_t buffer[120];
	// int index;

	memset(buffer,0,sizeof(buffer));

	if(driver->recv(cli_fd, buffer, sizeof(buffer)) > 0){
		printf("Message received\n");

		// for (index = 0; index < 32; index++) {
		// 	if (index % 8 == 0)
		// 		printf("\n");

		// 	printf("0x%02x ", buffer[index]);
		// }
		printf("%s\n", buffer);
		printf("\n");
	}else
		printf("No data available\n");

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

	cli_fd = driver->probe();

	/*TO DO: GET PATH CORRECT */
	driver->open("");

	g_option_context_free(context);

	main_loop = g_main_loop_new(NULL, FALSE);

	printf("RPi nRF24L01 Radio test tool\n");

	if (strcmp(opt_mode, "server") == 0){
        nrf24l01_open_pipe(0, cli_fd);
        nrf24l01_set_standby();
        nrf24l01_set_prx();
        // cli_fd = driver->connect(cli_fd, 10);

        // if (driver->listen(cli_fd) < 0)
        //     return -1;

        // do {
        //     ret = driver->accept(cli_fd);
        // } while (ret < 0 && quit ==1);

        // cli_fd = ret;
        timeout_id = g_timeout_add_seconds(1, timeout_watch_server, NULL);

    }else{
    	/* Open pipe zero in the sockfd address */
        nrf24l01_open_pipe(0, cli_fd);
        nrf24l01_set_standby();
        nrf24l01_set_ptx(cli_fd);
        // cli_fd = driver->connect(cli_fd, 10);
        timeout_id = g_timeout_add_seconds(1, timeout_watch_client, NULL);
    }


	g_main_loop_run(main_loop);

	g_source_remove(timeout_id);

	g_main_loop_unref(main_loop);

	return 0;
}
