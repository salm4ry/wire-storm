#include <netinet/in.h>

#define BACKLOG 3

int server_create(int port);
int server_accept(int server_fd, struct sockaddr_in address);
