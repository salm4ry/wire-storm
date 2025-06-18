#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include <signal.h>
#include <string.h>
#include <errno.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "include/socket.h"
#include "include/ctmp.h"
#include "include/log.h"

#define SRC_PORT 33333
#define RCV_PORT 44444
#define BACKLOG 3 /* max number of pending connections */
#define MAGIC 0xcc /* magic byte */

/*
 * Allow a single source client to connect on port 33333
 *
 * Allow multiple destination clients to connect on port 44444
 *
 * Accept CTMP messages from the source and forward to all destination clients.
 * These should be forwarded in the order they are received
 */

struct server_socket *src_server, *rcv_server;

void cleanup_handler(int signum)
{
	/* tear down server sockets */
	server_close(src_server);
	server_close(rcv_server);
}

void setup_signal_handler()
{
	struct sigaction cleanup_action;

	/* run cleanup_handler() on receipt of signals specified with
	 * sigaction() */
	cleanup_action.sa_handler = cleanup_handler;
	sigemptyset(&cleanup_action.sa_mask);

	/* actions to block while in signal handler */
	sigaddset(&cleanup_action.sa_mask, SIGINT);
	sigaddset(&cleanup_action.sa_mask, SIGTERM);
	cleanup_action.sa_flags = 0;

	/* handle SIGINT and SIGTERM */
	if (sigaction(SIGINT, &cleanup_action, NULL) == -1) {
		pr_err("error setting up SIGINT handler: %s\n", strerror(errno));
		exit(errno);
	}
	if (sigaction(SIGTERM, &cleanup_action, NULL) == -1) {
		pr_err("error setting up SIGTERM handler: %s\n", strerror(errno));
		exit(errno);
	}
}

int main(int argc, char *argv[])
{
	int new_socket, bytes_read = 0;

	/* set up servers */
	src_server = server_create(SRC_PORT);
	if (!src_server) {
		pr_err("error setting up server on port %d\n", SRC_PORT);
		return EXIT_FAILURE;
	}

	rcv_server = server_create(RCV_PORT);
	if (!rcv_server) {
		pr_err("error setting up server on port %d\n", RCV_PORT);
		return EXIT_FAILURE;
	}

	/* setup_signal_handler(); */

	while (1) {
		new_socket = server_accept(src_server->fd, src_server->addr);
		if (new_socket < 0) {
			pr_err("error accepting connection to port %d\n", SRC_PORT);
			exit(-new_socket);
		}

		do {
			bytes_read = parse_message(new_socket);
			if (bytes_read != -1) {
				pr_debug("bytes read: %d\n", bytes_read);
			}
		} while (bytes_read);

		pr_debug("closing connection...\n");
		/* close connected socket */
		close(new_socket);
	}

	while(1) {
		parse_message(new_socket);
		sleep(1);
	}


	return EXIT_SUCCESS;
}
