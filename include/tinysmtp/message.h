/**
 * @file message.h
 * @author Matheus T. dos Santos (matheus.santos@edge.ufal.br)
 * @brief Define the structure to store a email message.
 * @version 0.1
 * @date 09/01/2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TS_MESSAGE_H
#define TS_MESSAGE_H

#include <tinysmtp/address.h>

/**
 * @brief Maximum length of email subject.
 *
 */
#define CONFIG_SUBJECT_LENGTH 32

/**
 * @brief Maximum length of email body.
 *
 */
#define CONFIG_BODY_LENGTH 64

/**
 * @brief Stores a email message metadata and contents.
 *
 */
struct message {
    struct address from;                 /**< Sender address; */
    struct address to;                   /**< Recipient address; */
    char subject[CONFIG_SUBJECT_LENGTH]; /**< Subject of the email; */
    char body[CONFIG_BODY_LENGTH];       /**< Content of the email; */
};

#endif /* TS_MESSAGE_H */
