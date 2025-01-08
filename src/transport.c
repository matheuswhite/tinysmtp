#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tinysmtp/transport.h>

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

static void fmt_timestamp(const time_t *timestamp, char *buffer, size_t buffer_size) {
    const char *weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *months[] = {"January", "February", "March",     "April",   "May",      "June",
                            "July",    "August",   "September", "October", "November", "December"};
    struct tm *datetime = localtime(timestamp);

    snprintf(buffer, buffer_size, "%s, %02d %s %04d %02d:%02d:%02d", weekdays[datetime->tm_wday], datetime->tm_mday,
             months[datetime->tm_mon], datetime->tm_year + 1900, datetime->tm_hour + CONFIG_TIMEZONE, datetime->tm_min,
             datetime->tm_sec);
}

int ts_transport_send(struct transport *transport, struct message *msg) {
    int err = 0;
    char datetime[32] = {};
    time_t timestamp = 0;

    time(&timestamp);
    fmt_timestamp(&timestamp, datetime, sizeof(datetime));
    printf("Datetime: %s\n", datetime);

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
                            CMD("Date: ", datetime, "\r\n", "From: ", msg->from.address, "\r\n", "Subject: ", msg->subject, "\r\n",
                                "To: ", msg->to.address, "\r\n", msg->body, "\r\n", ".\r\n"),
                            250),
                   err, close_tls);

close_tls:
    CHECK_ERR(transport->tls->close(transport->tls));

    return err;
}
