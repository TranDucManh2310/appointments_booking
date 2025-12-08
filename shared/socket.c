#include "socket.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "constants.h"

// server
int create_server_socket(int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return -1; }
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(s); return -1; }
    if (listen(s, 32) < 0) { perror("listen"); close(s); return -1; }
    return s;
}

// client
int create_client_socket(const char *host, int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return -1; }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) { perror("inet_pton"); close(s); return -1; }
    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); close(s); return -1; }
    return s;
}

// send a line (append '\n')
ssize_t send_line(int sock, const char *line) {
    if (!line) return -1;
    char buf[MAX_LINE];
    int n = snprintf(buf, sizeof(buf), "%s\n", line);
    return send(sock, buf, n, 0);
}

// recv until '\n'
ssize_t recv_line(int sock, char *buf, size_t maxlen) {
    if (!buf) return -1;
    size_t pos = 0;
    while (pos + 1 < maxlen) {
        ssize_t r = recv(sock, buf + pos, 1, 0);
        if (r <= 0) return r;
        if (buf[pos] == '\n') { buf[pos] = '\0'; return pos; }
        pos += r;
    }
    buf[maxlen-1] = '\0';
    return pos;
}
