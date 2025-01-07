#include "message.h"
#include "transport.h"
#include "user_credentials.h"
#include "zephyr/kernel.h"
#include "zephyr/sockets.h"

#include <stdint.h>

void zephyr_sleep(uint32_t duration_ms) { k_msleep(duration_ms); }

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
        .sleep = zephyr_sleep,
    };
    struct message msg = {
        .from = {""},
        .to = {""},
        .subject = "Test from zephyr",
        .body = "Test from zephyr body",
    };

    err = ts_transport_send(&transport, &msg);

    printk("ts_transport_send result: %d\n", err);

    return 0;
}