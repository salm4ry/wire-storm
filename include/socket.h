/// @file

#include <netinet/in.h>

#define SRC_PORT 33333  ///< source server port
#define DST_PORT 44444  ///< destination server port
#define BACKLOG 3  ///< max pending connection queue length for listen()

/**
 * @struct server_socket
 * @brief Describes a socket server
 */
struct server_socket {
	int fd;  ///< file descriptor
	struct sockaddr_in addr;  ///< address
};

struct sockaddr_in server_address(int port);
struct server_socket *server_create(int port);
int server_accept(int server_fd, struct sockaddr_in address);
void server_close(struct server_socket *server);
