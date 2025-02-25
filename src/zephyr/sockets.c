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

LOG_MODULE_REGISTER(tinysmtp_sockets, LOG_LEVEL_DBG);

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

K_MUTEX_DEFINE(socket_lock);

struct tls_socket_data {
    int fd;
    struct sockaddr_in server_addr;
} socket_data;

static void dns_resolve_callback(enum dns_resolve_status status, struct dns_addrinfo *info, void *user_data) {
    if (info == NULL) {
        return;
    }

    LOG_DBG("dns resolve callback");
    LOG_DBG("\tstatus: %d", status);
    LOG_DBG("\tinfo: %d.%d.%d.%d", info->ai_addr.data[2], info->ai_addr.data[3], info->ai_addr.data[4], info->ai_addr.data[5]);

    dns.status = status;
    memcpy(&dns.addr, &info->ai_addr, sizeof(struct sockaddr));
    dns.has_finished = true;
}

static int resolve_server(char *server, uint8_t *ip) {
    int err;

    dns.ctx = dns_resolve_get_default();

    err = dns_resolve_name(dns.ctx, server, DNS_QUERY_TYPE_A, NULL, dns_resolve_callback, NULL, 2000);
    if (err < 0) {
        LOG_ERR("Resolve name error: %d", err);
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

static int tls_open(struct socket *sock, char *server, uint16_t port) {
    int err;
    struct in_addr server_ip;
    struct timeval timeout;
    sec_tag_t sec_tag_list[] = {CA_CERTIFICATE_TAG};

    err = tls_credential_add(CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate, sizeof(ca_certificate));
    if (err < 0) {
        LOG_ERR("Failed to register public certificate: %d", err);
        return err;
    }

    err = resolve_server(server, (uint8_t *) &server_ip.s_addr);
    if (err < 0) {
        LOG_ERR("Error to resolve DNS: %d", err);
        return err;
    }

    // lock mutex
    err = k_mutex_lock(&socket_lock, K_MSEC(CONFIG_SOCKET_OPEN_TIMEOUT_MS));
    if (err < 0) {
        return err;
    }

    socket_data.fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TLS_1_2);
    if (socket_data.fd < 0) {
        k_mutex_unlock(&socket_lock);
        return -EFAULT;
    }

    err = zsock_setsockopt(socket_data.fd, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_list, sizeof(sec_tag_list));
    if (err < 0) {
        LOG_ERR("Failed to set TLS_SEC_TAG_LIST option: %d", -errno);
        k_mutex_unlock(&socket_lock);
        return -errno;
    }

    err = zsock_setsockopt(socket_data.fd, SOL_TLS, TLS_HOSTNAME, server, strlen(server));
    if (err < 0) {
        LOG_ERR("Failed to set TLS_HOSTNAME option: %d", -errno);
        k_mutex_unlock(&socket_lock);
        return -errno;
    }

    socket_data.server_addr.sin_family = AF_INET;
    socket_data.server_addr.sin_port = htons(port);
    socket_data.server_addr.sin_addr.s_addr = server_ip.s_addr;

    err = zsock_connect(socket_data.fd, (struct sockaddr *) &socket_data.server_addr, sizeof(struct sockaddr_in));
    if (err < 0) {
        k_mutex_unlock(&socket_lock);
        return -errno;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    zsock_setsockopt(socket_data.fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

    return 0;
}

static int tls_close(struct socket *sock) {
    int err;
    struct tls_socket_data *data = sock->user_data;

    err = zsock_close(data->fd);
    if (err < 0) {
        return err;
    }

    memset(data, 0, sizeof(struct tls_socket_data));

    k_mutex_unlock(&socket_lock);

    return 0;
}

static int tls_write(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tls_socket_data *data = user_data;

    err = zsock_send(data->fd, buffer, buffer_len, 0);
    if (err < 0) {
        return -errno;
    }

    return 0;
}

static int tls_read(void *user_data, uint8_t *buffer, size_t buffer_len) {
    int err;
    struct tls_socket_data *data = user_data;

    err = zsock_recv(data->fd, buffer, buffer_len, 0);
    if (err < 0) {
        return -errno;
    }

    return 0;
}

struct socket tls_socket = {
    .open = tls_open,
    .close = tls_close,
    .write = tls_write,
    .read = tls_read,
    .user_data = &socket_data,
};

struct socket *ts_zephyr_tls_socket(void) { return &tls_socket; }
