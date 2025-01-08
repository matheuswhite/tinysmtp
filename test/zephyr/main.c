#include <stdint.h>
#include <tinysmtp/message.h>
#include <tinysmtp/transport.h>
#include <tinysmtp/user_credentials.h>
#include <tinysmtp/zephyr/sockets.h>
#include <zephyr/kernel.h>

int main(int argc, char *argv[]) {
    int err;

    struct transport transport = {
        .credentials =
            {
                .user_b64 = USER_B64,
                .password_b64 = PASSWORD_B64,
            },
        .hostname = "ZephyrMachine",
        .server = "smtp.gmail.com",
        .server_tcp_port = 587,
        .server_tls_port = 465,
        .tcp = ts_zephyr_tcp_socket(),
        .tls = ts_zephyr_tls_socket(),
    };
    struct message msg = {
        .from = {""},
        .to = {""},
        .subject = "Test from zephyr",
        .body = "Test from zephyr body",
    };

    err = ts_transport_send(&transport, &msg);

    printk("ts_transport_send result: %d\n", err);

    exit(0);
}