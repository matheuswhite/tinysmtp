#include <errno.h>
#include <string.h>
#include <sys/errno.h>
#include <tinysmtp/zephyr/sockets.h>
#include <zephyr/kernel.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>

#define CA_CERTIFICATE_TAG 1

static const unsigned char ca_certificate[] = {
#include "globalsign_r1.der.inc"
};

static struct dns_fields {
    struct dns_resolve_context *ctx;
    enum dns_resolve_status status;
    struct sockaddr addr;
    bool has_finished;
} dns = {
    .ctx = NULL,
    .has_finished = false,
    .status = DNS_EAI_NODATA,
};

struct tcp_socket_data {
    int fd;
    struct sockaddr_in server_addr;
};

static void dns_resolve_callback(enum dns_resolve_status status, struct dns_addrinfo *info, void *user_data) {
    if (info == NULL) {
        return;
    }

    printk("dns resolve callback\n");
    printk("\tstatus: %d\n", status);
    printk("\tinfo: %d.%d.%d.%d\n", info->ai_addr.data[2], info->ai_addr.data[3], info->ai_addr.data[4], info->ai_addr.data[5]);

    dns.status = status;
    memcpy(&dns.addr, &info->ai_addr, sizeof(struct sockaddr));
    dns.has_finished = true;
}

static int resolve_server(char *server, uint8_t *ip) {
    int err;

    dns.ctx = dns_resolve_get_default();

    err = dns_resolve_name(dns.ctx, server, DNS_QUERY_TYPE_A, NULL, dns_resolve_callback, NULL, 2000);
    if (err < 0) {
        printk("Resolve name error: %d\n", err);
        return err;
    }

    while (!dns.has_finished) {
        k_msleep(10);
    }

    if (dns.status != DNS_EAI_ALLDONE && dns.status != DNS_EAI_INPROGRESS) {
        return -EFAULT;
    }

    memcpy(ip, dns.addr.data + 2, 4);

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
        printk("Error to resolve DNS: %d\n", err);
        return err;
    }

    data = calloc(1, sizeof(struct tcp_socket_data));
    if (data == NULL) {
        return -ENOMEM;
    }

    sock->user_data = data;

    data->fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (data->fd < 0) {
        printk("Cannot open socket: %d\n", data->fd);
        return -EFAULT;
    }

    data->server_addr.sin_family = AF_INET;
    data->server_addr.sin_port = htons(port);
    data->server_addr.sin_addr.s_addr = server_ip.s_addr;

    err = zsock_connect(data->fd, (struct sockaddr *) &data->server_addr, sizeof(struct sockaddr_in));
    if (err < 0) {
        printk("Cannot connect to socket: %d\n", -errno);
        return -errno;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    zsock_setsockopt(data->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

    return 0;
}

static int socket_close(struct socket *sock) {
    int err;
    struct tcp_socket_data *data;

    if (sock->user_data == NULL) {
        return -EAGAIN;
    }

    data = sock->user_data;

    err = zsock_close(data->fd);
    if (err < 0) {
        return err;
    }

    free(sock->user_data);
    sock->user_data = NULL;

    return 0;
}

static int socket_write(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tcp_socket_data *data = user_data;

    err = zsock_send(data->fd, buffer, buffer_len, 0);
    if (err < 0) {
        return -errno;
    }

    return 0;
}

static int socket_read(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tcp_socket_data *data = user_data;

    err = zsock_recv(data->fd, buffer, buffer_len, 0);
    if (err < 0) {
        return -errno;
    }

    return 0;
}

struct socket *ts_zephyr_tcp_socket(void) {
    struct socket *socket = calloc(1, sizeof(struct socket));
    if (socket == NULL) {
        return NULL;
    }

    socket->open = tcp_open;
    socket->close = socket_close;
    socket->write = socket_write;
    socket->read = socket_read;

    return socket;
}

static int tls_open(struct socket *sock, char *server, uint16_t port) {
    int err;
    struct tcp_socket_data *data;
    struct in_addr server_ip;
    struct timeval timeout;
    sec_tag_t sec_tag_list[] = {CA_CERTIFICATE_TAG};

    if (sock->user_data != NULL) {
        return -EAGAIN;
    }

    err = tls_credential_add(CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate, sizeof(ca_certificate));
    if (err < 0) {
        printk("Failed to register public certificate: %d\n", err);
        return err;
    }

    err = resolve_server(server, (uint8_t *) &server_ip.s_addr);
    if (err < 0) {
        printk("Error to resolve DNS: %d\n", err);
        return err;
    }

    data = calloc(1, sizeof(struct tcp_socket_data));
    if (data == NULL) {
        return -ENOMEM;
    }

    sock->user_data = data;

    data->fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TLS_1_2);
    if (data->fd < 0) {
        return -EFAULT;
    }

    err = zsock_setsockopt(data->fd, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_list, sizeof(sec_tag_list));
    if (err < 0) {
        printk("Failed to set TLS_SEC_TAG_LIST option: %d\n", -errno);
        return -errno;
    }

    err = zsock_setsockopt(data->fd, SOL_TLS, TLS_HOSTNAME, server, strlen(server));
    if (err < 0) {
        printk("Failed to set TLS_HOSTNAME option: %d\n", -errno);
        return -errno;
    }

    data->server_addr.sin_family = AF_INET;
    data->server_addr.sin_port = htons(port);
    data->server_addr.sin_addr.s_addr = server_ip.s_addr;

    err = zsock_connect(data->fd, (struct sockaddr *) &data->server_addr, sizeof(struct sockaddr_in));
    if (err < 0) {
        return -errno;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    zsock_setsockopt(data->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

    return 0;
}

struct socket *ts_zephyr_tls_socket(void) {
    struct socket *socket = calloc(1, sizeof(struct socket));
    if (socket == NULL) {
        return NULL;
    }

    socket->open = tls_open;
    socket->close = socket_close;
    socket->write = socket_write;
    socket->read = socket_read;

    return socket;
}
