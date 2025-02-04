#include <stdint.h>
#include <tinysmtp/message.h>
#include <tinysmtp/transport.h>
#include <tinysmtp/zephyr/sockets.h>
#include <zephyr/kernel.h>

#include <tinysmtp/user_credentials.h>

#define CA_CERTIFICATE_TAG 1

static const unsigned char ca_certificate[] = {
#include "globalsign_r1.der.inc"
};

int main(int argc, char *argv[]) {
    int err;

    sec_tag_t sec_tag_list[] = {CA_CERTIFICATE_TAG};

    err = tls_credential_add(CA_CERTIFICATE_TAG, TLS_CREDENTIAL_CA_CERTIFICATE, ca_certificate, sizeof(ca_certificate));
    if (err < 0) {
        LOG_ERR("Failed to register public certificate: %d", err);
        return err;
    }

    struct transport transport = {
        .credentials =
            {
                .user_b64 = TS_USER_B64,
                .password_b64 = TS_PASSWORD_B64,
            },
        .hostname = "ZephyrMachine",
        .server = "smtp.gmail.com",
        .server_tls_port = 465,
        .tls = ts_zephyr_tls_socket(),
    };
    struct message msg = {
        .from = {TS_FROM_ADDRESS},
        .to = {TS_TO_ADDRESS},
        .subject = "Test from zephyr",
        .body = "Test from zephyr body",
    };

    ts_zephyr_set_sec_tag(CA_CERTIFICATE_TAG);

    err = ts_transport_send(&transport, &msg);

    printk("ts_transport_send result: %d\n", err);

    exit(0);
}