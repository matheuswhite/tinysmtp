#ifndef TS_TRANSPORT_H
#define TS_TRANSPORT_H

#include <stdint.h>
#include <stdlib.h>
#include <tinysmtp/credentials.h>
#include <tinysmtp/message.h>

#define CONFIG_SERVER_LENGTH 32
#define CONFIG_HOSTNAME_LENGTH 32

struct socket {
    int (*open)(struct socket *, char *, uint16_t);
    int (*close)(struct socket *);
    int (*write)(void *, uint8_t *, size_t);
    int (*read)(void *, uint8_t *, size_t);
    void *user_data;
};

struct transport {
    char hostname[CONFIG_HOSTNAME_LENGTH];
    char server[CONFIG_SERVER_LENGTH];
    uint16_t server_tcp_port;
    uint16_t server_tls_port;
    struct credentials credentials;
    struct socket *tcp;
    struct socket *tls;
};

int ts_transport_send(struct transport *transport, struct message *msg);

#endif /* TS_TRANSPORT_H */
