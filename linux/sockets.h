#ifndef TS_LINUX_SOCKETS_H
#define TS_LINUX_SOCKETS_H

#include "../transport.h"

struct socket *ts_linux_tcp_socket(void);

struct socket *ts_linux_tls_socket(void);

#endif /* TS_LINUX_SOCKETS_H */
