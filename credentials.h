#ifndef TS_CREDENTIALS_H
#define TS_CREDENTIALS_H

#define CONFIG_USER_LENGTH 64
#define CONFIG_PASSWORD_LENGTH 64

struct credentials {
    char user_b64[CONFIG_USER_LENGTH];
    char password_b64[CONFIG_PASSWORD_LENGTH];
};

#endif /* TS_CREDENTIALS_H */
