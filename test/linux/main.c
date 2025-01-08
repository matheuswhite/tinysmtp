#include <stdio.h>
#include <tinysmtp/linux/sockets.h>
#include <tinysmtp/message.h>
#include <tinysmtp/transport.h>
#include <tinysmtp/user_credentials.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int err;

    struct transport transport = {
        .credentials =
            {
                .user_b64 = TS_USER_B64,
                .password_b64 = TS_PASSWORD_B64,
            },
        .hostname = "LinuxMachine",
        .server = "smtp.gmail.com",
        .server_tls_port = 465,
        .tls = ts_linux_tls_socket(),
    };
    struct message msg = {
        .from = {TS_FROM_ADDRESS},
        .to = {TS_TO_ADDRESS},
        .subject = "Test from linux",
        .body = "Test from linux body",
    };

    err = ts_transport_send(&transport, &msg);

    printf("ts_transport_send result: %d\n", err);

    return err;
}
