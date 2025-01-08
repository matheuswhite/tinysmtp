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
                .user_b64 = USER_B64,
                .password_b64 = PASSWORD_B64,
            },
        .hostname = "LinuxMachine",
        .server = "smtp.gmail.com",
        .server_tcp_port = 587,
        .server_tls_port = 465,
        .tcp = ts_linux_tcp_socket(),
        .tls = ts_linux_tls_socket(),
    };
    struct message msg = {
        .from = "",
        .to = "",
        .subject = "Test from linux",
        .body = "Test from linux body",
    };

    err = ts_transport_send(&transport, &msg);

    printf("ts_transport_send result: %d\n", err);

    return err;
}
