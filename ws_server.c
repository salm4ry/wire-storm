/**
 * @file ws_server.c
 * @brief Server code: contains `main()`
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include <errno.h>
#include <pthread.h>
#include <sys/queue.h>

#include "args.h"
#include "log.h"
#include "socket.h"
#include "ctmp.h"
#include "msg_queue.h"
#include "thread.h"
#include "timestamp.h"

#define INITIAL_DELAY 1  ///< starting delay (seconds) when waiting for idle workers

/**
 * @brief Array of client worker thread structures
 * @details Memory is allocated for `num_workers` threads when the server
 * starts. New clients have to wait for a thread to become available if all
 * threads are busy.
 */
struct worker_list dst;

pthread_mutex_t msg_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t msg_cond = PTHREAD_COND_INITIALIZER;
struct msg_queue msg_queue_head;

struct args init_args;

/**
 * @brief Run source server
 * @details Accept a single client connection and parse messages from it,
 * broadcasting to receivers when valid
 */
void run_src_server(void *data)
{
	int src_socket;
	struct server_socket *src_server = NULL;
	struct ctmp_msg *current_msg = NULL;
	struct msg_entry *new_msg_entry = NULL;

	/* CTMP message parsing function */
	struct ctmp_msg *(*ctmp_parse_func)(int) = NULL;

	src_server = server_create(SRC_PORT, init_args.backlog);
	if (!src_server) {
		pr_err("error setting up server on port %d\n", SRC_PORT);
		exit(EXIT_FAILURE);
	}

	/* check which protocol version to use */
	if (init_args.extended) {
		ctmp_parse_func = &parse_ctmp_msg_extended;
	} else {
		ctmp_parse_func = &parse_ctmp_msg;
	}

	while (1) {
		src_socket = server_accept(src_server->fd, src_server->addr);
		if (src_socket < 0) {
			pr_err("error accepting connection to port %d\n", SRC_PORT);
			exit(-src_socket);
		}

		/* keep parsing messages while the connection is open */
		while (is_alive(src_socket)) {
			current_msg = ctmp_parse_func(src_socket);
			if (current_msg) {
				init_msg_entry(&new_msg_entry, current_msg,
						init_args.num_workers);

				pthread_mutex_lock(&msg_lock);
				/* add message to queue */
				TAILQ_INSERT_TAIL(&msg_queue_head, new_msg_entry, entries);

				/* signal that the message queue is non-empty
				 * (broadcast signal to all waiting threads) */
				pthread_cond_broadcast(&msg_cond);
				pthread_mutex_unlock(&msg_lock);
			}
		}

		/* close old src connection */
		pr_debug("closing src connection...\n");
		close(src_socket);
	}
}

/**
 * @brief Run destination worker
 * @param data `struct worker_args` object (includes the client file descriptor
 * to send messages to and its timestamp)
 * @details Send CTMP messages to a given client (using timestamps to ensure that
 * it only sends messages it is entitled to send)
 */
void *run_dst_worker(void *data)
{
	struct msg_entry *current = NULL, *prev = NULL, *next = NULL;
	ssize_t bytes_sent = 0;
	struct worker_args *args = (struct worker_args *) data;

	while (1) {
		current = get_msg_entry(&msg_queue_head, &msg_lock, &msg_cond,
				current, prev, false);

		/* check the connection is open before attempting to send */
		if (!is_alive(args->client_fd)) {
			pr_debug("thread %d: client connection closed\n",
					args->thread_index);
			goto conn_closed;
		}

		if (can_forward(current, args->thread_index, args->timestamp)) {
			/* send message to the assigned file descriptor */
			pr_debug("thread %d: sending a %d-byte message\n",
					args->thread_index, current->msg->len);
			bytes_sent = send_ctmp_msg(args->client_fd, current->msg);
			set_sent(current, args->thread_index, true);
		}

		pthread_mutex_lock(&msg_lock);
		/* get next message */
		next = TAILQ_NEXT(current, entries);
		pthread_mutex_unlock(&msg_lock);

		if (bytes_sent < 0) {
conn_closed:
			/* close old client fd */
			close(args->client_fd);

			pr_debug("thread %d: waiting for new fd...\n",
					args->thread_index);
			/* send failed, wait for new fd */
			pthread_mutex_lock(&args->lock);

			/* update status */
			*(args->self_status) = THREAD_READY;
			pthread_mutex_lock(&args->threads_status->lock);
			set_bit(&args->threads_status->data,
					args->thread_index, false);
			pthread_mutex_unlock(&args->threads_status->lock);

			while (*(args->self_status) != THREAD_BUSY) {
				pthread_cond_wait(&args->cond, &args->lock);
			}
			/* reset sent status for new connection */
			set_sent(current, args->thread_index, false);

			pthread_mutex_unlock(&args->lock);
			pr_debug("thread %d: got new fd %d\n",
					args->thread_index, args->client_fd);
		}

		prev = current;
		current = next;
	}

	return NULL;
}

/**
 * @brief Run destination server
 * @details Accept client connections and assign them to worker threads
 */
void *run_dst_server(void *data)
{
	int res, new_fd, thread_index, delay = INITIAL_DELAY;
	struct timespec client_ts;
	struct server_socket *dst_server = NULL;

	dst_server = server_create(DST_PORT, init_args.backlog);
	if (!dst_server) {
		pr_err("error setting up server on port %d\n", DST_PORT);
		exit(EXIT_FAILURE);
	}

	while (1) {
		new_fd = server_accept(dst_server->fd, dst_server->addr);
		if (new_fd < 0) {
			pr_err("error accepting connection to port %d\n", DST_PORT);
			/* retry */
			continue;
		}

		/* set timestamp to be time of accepting the connection */
		get_clock_time(&client_ts);

		thread_index = find_idle_thread(&dst);
		while (thread_index < 0) {
			pr_err("no thread available, retrying...\n");
			/* exponential backoff */
			sleep(delay);
			delay *= 2;
			thread_index = find_idle_thread(&dst);
		}
		/* reset delay after worker thread found */
		delay = INITIAL_DELAY;

		/* check thread state */
		switch (dst.workers[thread_index].status) {
		case THREAD_AVAILABLE:
			init_thread_info(&dst, thread_index, new_fd, client_ts);

			res = pthread_create(&dst.workers[thread_index].thread,
					NULL, run_dst_worker, &dst.workers[thread_index].args);
			if (res != 0) {
				p_error("pthread_create", errno);
				exit(errno);
			}
			break;
		case THREAD_READY:
			/* reassign file descriptor and signal */
			wake_up_thread(&dst, thread_index, new_fd, client_ts);
			break;
		default:
			break;
		}
	}

	return NULL;
}

/**
 * @brief Run cleanup worker
 * @details Walk the message queue, deleting message data for entries past their
 * TTL
 */
void *run_cleanup_worker(void *data)
{
	struct msg_entry *current = NULL, *prev = NULL, *next = NULL;
	struct timespec now, msg_plus_ttl;

	/* clean up messages in the queue that are past their TTL */
	while (true) {
		current = get_msg_entry(&msg_queue_head, &msg_lock, &msg_cond,
				current, prev, true);

		if (current->msg) {
			get_clock_time(&now);
			msg_plus_ttl = current->timestamp;
			msg_plus_ttl.tv_sec += init_args.ttl;

			/* remove and free the entry if its TTL has passed */
			if (compare_times(&msg_plus_ttl, &now)) {
				pr_debug("cleanup: freeing %d-byte message\n",
						current->msg->len);
				pthread_mutex_lock(&msg_lock);
				free_ctmp_msg(current->msg);
				current->msg = NULL;
				pthread_mutex_unlock(&msg_lock);
			}
		}

		pthread_mutex_lock(&msg_lock);
		/* get next message */
		next = TAILQ_NEXT(current, entries);
		pthread_mutex_unlock(&msg_lock);

		prev = current;
		current = next;
	}


	return NULL;
}

/**
 * @brief Start source and destination servers, destination worker threads, and
 * cleanup worker
 */
int main(int argc, char *argv[])
{
	int res;
	pthread_t dst_server_thread, cleanup_thread;

	/* parse command-line arguments */
	set_default_args(&init_args);
	parse_args(argc, argv, &init_args);
	pr_debug("extended = %d, num_workers = %d, backlog = %d, ttl = %d\n",
			init_args.extended, init_args.num_workers,
			init_args.backlog, init_args.ttl);

	/* initialise client and message queues */
	TAILQ_INIT(&msg_queue_head);

	/* allocate thread array */
	init_workers(&dst, init_args.num_workers);

	/* create destination server thread */
	res = pthread_create(&dst_server_thread, NULL, &run_dst_server, NULL);
	if (res != 0) {
		p_error("pthread_create", errno);
		exit(res);
	}

	/* create message cleanup thread */
	res = pthread_create(&cleanup_thread, NULL, &run_cleanup_worker, NULL);
	if (res != 0) {
		p_error("pthread_create", errno);
		exit(res);
	}

	/* run source server */
	run_src_server(NULL);

	return EXIT_SUCCESS;
}
