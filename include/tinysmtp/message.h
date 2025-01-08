#ifndef TS_MESSAGE_H
#define TS_MESSAGE_H

#include <tinysmtp/address.h>

#define CONFIG_SUBJECT_LENGTH 32
#define CONFIG_BODY_LENGTH 64

struct message {
    struct address from;
    struct address to;
    char subject[CONFIG_SUBJECT_LENGTH];
    char body[CONFIG_BODY_LENGTH];
};

int ts_message_raw(struct message *message, char *output);

#endif /* TS_MESSAGE_H */
