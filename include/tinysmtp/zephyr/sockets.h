/**
 * @file sockets.h
 * @author Matheus T. dos Santos (matheus.santos@edge.ufal.br)
 * @brief Define the Zephyr's TLS socket implementation.
 * @version 0.1
 * @date 09/01/2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TS_ZEPHYR_SOCKETS_H
#define TS_ZEPHYR_SOCKETS_H

#include <tinysmtp/transport.h>

/**
 * @brief Maximum timeout, in milliseconds, to open socket.
 *
 */
#define CONFIG_SOCKET_OPEN_TIMEOUT_MS 2000

/**
 * @brief Get the Zephyr's TLS socket implementation.
 *
 * @return struct socket* Reference to Zephyr's TLS socket implementation.
 */
struct socket *ts_zephyr_tls_socket(void);

#endif /* TS_ZEPHYR_SOCKETS_H */
