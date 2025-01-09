/**
 * @file transport.h
 * @author Matheus T. dos Santos (matheus.santos@edge.ufal.br)
 * @brief Define the socket and transport structure and also define the a function to send a email.
 * @version 0.1
 * @date 09/01/2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TS_TRANSPORT_H
#define TS_TRANSPORT_H

#include <stdint.h>
#include <stdlib.h>
#include <tinysmtp/credentials.h>
#include <tinysmtp/message.h>

/**
 * @brief Maximum length of the server name.
 *
 */
#define CONFIG_SERVER_LENGTH 32

/**
 * @brief Maximum length of the hostname.
 *
 */
#define CONFIG_HOSTNAME_LENGTH 32

/**
 * @brief Timezone used for email date.
 *
 */
#define CONFIG_TIMEZONE -3

/**
 * @brief Socket interface.
 *
 */
struct socket {
    /**
     * @brief Open the socket and setup all needed resource.
     *
     * @param sock Reference to this interface;
     * @param server Server name;
     * @param port Server port.
     *
     * @return int a negative integer on error, 0 otherwise.
     */
    int (*open)(struct socket *sock, char *server, uint16_t port);

    /**
     * @brief Close the socket and free all allocated resource.
     *
     * @param sock Reference to this interface;
     *
     * @return int a negative integer on error, 0 otherwise.
     */
    int (*close)(struct socket *sock);

    /**
     * @brief Write data into socket;
     *
     * @param user_data User data stored in this interface;
     * @param buffer Buffer to write;
     * @param buffer_len Amount to write.
     *
     * @return int a negative integer on error, 0 otherwise.
     */
    int (*write)(void *user_data, uint8_t *buffer, size_t buffer_len);

    /**
     * @brief Read data from socket;
     *
     * @param user_data User data stored in this interface;
     * @param buffer Buffer to store read data;
     * @param buffer_len Amount to read.
     *
     * @return int a negative integer on error, 0 otherwise.
     */
    int (*read)(void *user_data, uint8_t *buffer, size_t buffer_len);

    /**
     * @brief Custom data used by socket interface implementation.
     *
     */
    void *user_data;
};

/**
 * @brief Store SMTP transport context.
 *
 */
struct transport {
    char hostname[CONFIG_HOSTNAME_LENGTH]; /**< System hostname; */
    char server[CONFIG_SERVER_LENGTH];     /**< SMTP server; */
    uint16_t server_tls_port;              /**< SMTP TLS port; */
    struct credentials credentials;        /**< Email credentials; */
    struct socket *tls;                    /**< Reference to TLS socket implementation. */
};

/**
 * @brief Send a email.
 *
 * @param transport Reference to transport context;
 * @param msg Message to send by email.
 *
 * @return int a negative integer on error, 0 otherwise.
 */
int ts_transport_send(struct transport *transport, struct message *msg);

#endif /* TS_TRANSPORT_H */
