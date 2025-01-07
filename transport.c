#include "transport.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD(...)                                                                                                                   \
    (const char *[]) { __VA_ARGS__, NULL }

#define CHECK_ERR(fn)                                                                                                              \
    do {                                                                                                                           \
        int __err = fn;                                                                                                            \
        if (__err < 0) {                                                                                                           \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (0)

#define CHECK_ERR_GOTO(fn, err, label)                                                                                             \
    do {                                                                                                                           \
        err = fn;                                                                                                                  \
        if (err < 0) {                                                                                                             \
            goto label;                                                                                                            \
        }                                                                                                                          \
    } while (0)

static int recv_line(struct socket *sock, char *output, size_t output_size) {
    int err;

    for (int i = 0; i < output_size; i++) {
        err = sock->read(sock->user_data, (uint8_t *) &output[i], 1);
        if (err < 0) {
            return err;
        }

        if (output[i] == '\r' || output[i] == '\n') {
            output[i] = '\0';
            i -= 1;

            if (i > 0) {
                return 0;
            }
        }
    }

    return 0;
}

static void drop_messages_until_error(struct socket *sock) {
    int err = 0;
    char line[128];

    while (err == 0) {
        err = recv_line(sock, line, sizeof(line));
        printf("Dropped >> [%s]\r\n", line);
    }
}

static int send_cmd(struct socket *sock, const char *cmd[], int expected_rsp) {
    int err;
    char rsp[128] = {};
    int rsp_code;

    for (int i = 0; cmd[i] != NULL; i++) {
        err = sock->write(sock->user_data, (uint8_t *) cmd[i], strlen(cmd[i]));
        if (err < 0) {
            return err;
        }
    }

    err = recv_line(sock, rsp, sizeof(rsp));
    if (err < 0) {
        return err;
    }

    rsp_code = strtoul(rsp, NULL, 10);

    if (rsp_code != expected_rsp) {
        return -EPERM;
    }

    return 0;
}

int ts_transport_send(struct transport *transport, struct message *msg) {
    int err = 0;

//     CHECK_ERR(transport->tcp->open(transport->tcp, transport->server, transport->server_tcp_port));

//     drop_messages_until_error(transport->tcp);

//     CHECK_ERR_GOTO(send_cmd(transport->tcp, CMD("EHLO ", transport->hostname, "\r\n"), 250), err, close_tcp);

//     drop_messages_until_error(transport->tcp);

//     CHECK_ERR_GOTO(send_cmd(transport->tcp, CMD("STARTTLS\r\n"), 220), err, close_tcp);

// close_tcp:
//     CHECK_ERR(transport->tcp->close(transport->tcp));

//     if (err < 0) {
//         return err;
//     }

    // transport->sleep(1000);

    CHECK_ERR(transport->tls->open(transport->tls, transport->server, transport->server_tls_port));

    drop_messages_until_error(transport->tls);

    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD("EHLO ", transport->hostname, "\r\n"), 250), err, close_tls);

    drop_messages_until_error(transport->tls);

    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD("AUTH LOGIN\r\n"), 334), err, close_tls);
    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD(transport->credentials.user_b64, "\r\n"), 334), err, close_tls);
    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD(transport->credentials.password_b64, "\r\n"), 235), err, close_tls);

    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD("MAIL FROM:<", msg->from.address, ">\r\n"), 250), err, close_tls);
    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD("RCPT TO:<", msg->to.address, ">\r\n"), 250), err, close_tls);

    CHECK_ERR_GOTO(send_cmd(transport->tls, CMD("DATA\r\n"), 354), err, close_tls);
    CHECK_ERR_GOTO(send_cmd(transport->tls,
                            CMD("Date: ", "Wed, 30 July 2019 06:04:34", "\r\n", "From: ", msg->from.address, "\r\n",
                                "Subject: ", msg->subject, "\r\n", "To: ", msg->to.address, "\r\n", msg->body, "\r\n", ".\r\n"),
                            250),
                   err, close_tls);

close_tls:
    CHECK_ERR(transport->tls->close(transport->tls));

    return err;
}
