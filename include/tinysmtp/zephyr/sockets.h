#ifndef TS_ZEPHYR_SOCKETS_H
#define TS_ZEPHYR_SOCKETS_H

#include <tinysmtp/transport.h>

struct socket *ts_zephyr_tcp_socket(void);

struct socket *ts_zephyr_tls_socket(void);

#endif /* TS_ZEPHYR_SOCKETS_H */
