#ifndef TS_ZEPHYR_SOCKETS_H
#define TS_ZEPHYR_SOCKETS_H

#include <tinysmtp/transport.h>

#define CONFIG_SOCKET_OPEN_TIMEOUT_MS 2000

struct socket *ts_zephyr_tls_socket(void);

#endif /* TS_ZEPHYR_SOCKETS_H */
