#ifndef SHARED_SOCKET_H
#define SHARED_SOCKET_H
#include <sys/types.h>
// server side
int create_server_socket(int port);

// client side
int create_client_socket(const char *host, int port);

// message send/recv utilities (line based)
ssize_t send_line(int sock, const char *line);
ssize_t recv_line(int sock, char *buf, size_t maxlen);

#endif
