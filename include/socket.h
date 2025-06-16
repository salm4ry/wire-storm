#include <netinet/in.h>

#define BACKLOG 3

struct server_socket {
	int fd;
	struct sockaddr_in addr;
};

struct sockaddr_in server_address(int port);
struct server_socket *server_create(int port);
int server_accept(int server_fd, struct sockaddr_in address);
void server_close(struct server_socket *server);
