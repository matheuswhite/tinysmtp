/**
 * @file sockets.h
 * @author Matheus T. dos Santos (matheus.santos@edge.ufal.br)
 * @brief Define Linux's TLS socket implementation.
 * @version 0.1
 * @date 09/01/2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TS_LINUX_SOCKETS_H
#define TS_LINUX_SOCKETS_H

#include <tinysmtp/transport.h>

/**
 * @brief Get the Linux's TLS socket implementation.
 *
 * @return struct socket* Reference to Linux's TLS socket implementation.
 */
struct socket *ts_linux_tls_socket(void);

#endif /* TS_LINUX_SOCKETS_H */
