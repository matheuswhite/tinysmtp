#include "sockets.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct tcp_socket_data {
    int fd;
    struct sockaddr_in server_addr;
};

static int resolve_server(char *server, uint8_t *ip) {
    struct hostent *lh = gethostbyname(server);
    memcpy(ip, lh->h_addr_list[0], 4);

    return 0;
}

static int tcp_open(struct socket *sock, char *server, uint16_t port) {
    int err;
    struct tcp_socket_data *data;
    struct in_addr server_ip;
    struct timeval timeout;

    if (sock->user_data != NULL) {
        return -EAGAIN;
    }

    err = resolve_server(server, (uint8_t *) &server_ip.s_addr);
    if (err < 0) {
        return err;
    }

    data = calloc(1, sizeof(struct tcp_socket_data));
    if (data == NULL) {
        return -ENOMEM;
    }

    sock->user_data = data;

    data->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (data->fd < 0) {
        return -EFAULT;
    }

    data->server_addr.sin_family = AF_INET;
    data->server_addr.sin_port = htons(port);
    data->server_addr.sin_addr.s_addr = server_ip.s_addr;

    err = connect(data->fd, (struct sockaddr *) &data->server_addr, sizeof(struct sockaddr_in));
    if (err < 0) {
        return -errno;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(data->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

    return 0;
}

static int tcp_close(struct socket *sock) {
    int err;
    struct tcp_socket_data *data;

    if (sock->user_data == NULL) {
        return -EAGAIN;
    }

    data = sock->user_data;

    err = close(data->fd);
    if (err < 0) {
        return err;
    }

    free(sock->user_data);
    sock->user_data = NULL;

    return 0;
}

static int tcp_write(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tcp_socket_data *data = user_data;

    err = send(data->fd, buffer, buffer_len, 0);
    if (err < 0) {
        return -errno;
    }

    return 0;
}

static int tcp_read(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tcp_socket_data *data = user_data;

    err = recv(data->fd, buffer, buffer_len, 0);
    if (err < 0) {
        return -errno;
    }

    return 0;
}

struct socket *ts_linux_tcp_socket(void) {
    struct socket *socket = calloc(1, sizeof(struct socket));
    if (socket == NULL) {
        return NULL;
    }

    socket->open = tcp_open;
    socket->close = tcp_close;
    socket->write = tcp_write;
    socket->read = tcp_read;

    return socket;
}

struct tls_socket_data {
    int fd;
    struct sockaddr_in server_addr;
    SSL_CTX *ctx;
    SSL *ssl;
};

static int tls_open(struct socket *sock, char *server, uint16_t port) {
    int err;
    struct tls_socket_data *data;
    struct in_addr server_ip;
    struct timeval timeout;

    if (sock->user_data != NULL) {
        return -EAGAIN;
    }

    err = resolve_server(server, (uint8_t *) &server_ip.s_addr);
    if (err < 0) {
        return err;
    }

    data = calloc(1, sizeof(struct tcp_socket_data));
    if (data == NULL) {
        return -ENOMEM;
    }

    sock->user_data = data;

    data->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (data->fd < 0) {
        return -EFAULT;
    }

    data->server_addr.sin_family = AF_INET;
    data->server_addr.sin_port = htons(port);
    data->server_addr.sin_addr.s_addr = server_ip.s_addr;

    err = connect(data->fd, (struct sockaddr *) &data->server_addr, sizeof(struct sockaddr_in));
    if (err < 0) {
        return -errno;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(data->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

    data->ctx = SSL_CTX_new(TLS_method());
    data->ssl = SSL_new(data->ctx);
    err = SSL_set_fd(data->ssl, data->fd);
    if (err < 0) {
        return err;
    }

    err = SSL_connect(data->ssl);
    if (err < 0) {
        return err;
    }

    return 0;
}

static int tls_close(struct socket *sock) {
    int err;
    struct tls_socket_data *data;

    if (sock->user_data == NULL) {
        return -EAGAIN;
    }

    data = sock->user_data;

    err = SSL_shutdown(data->ssl);
    if (err < 0) {
        return err;
    }

    err = close(data->fd);
    if (err < 0) {
        return err;
    }

    SSL_free(data->ssl);
    SSL_CTX_free(data->ctx);
    free(sock->user_data);
    sock->user_data = NULL;

    return 0;
}

static int tls_write(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tls_socket_data *data = user_data;

    err = SSL_write(data->ssl, buffer, buffer_len);
    if (err < 0) {
        return err;
    }

    return 0;
}

static int tls_read(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tls_socket_data *data = user_data;

    err = SSL_read(data->ssl, buffer, buffer_len);
    if (err < 0) {
        return err;
    }

    return 0;
}

struct socket *ts_linux_tls_socket(void) {
    struct socket *socket = calloc(1, sizeof(struct socket));
    if (socket == NULL) {
        return NULL;
    }

    socket->open = tls_open;
    socket->close = tls_close;
    socket->write = tls_write;
    socket->read = tls_read;

    return socket;
}
