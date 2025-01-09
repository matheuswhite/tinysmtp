/**
 * @file credentials.h
 * @author Matheus T. dos Santos (matheus.santos@edge.ufal.br)
 * @brief Define the structure to store email credentials.
 * @version 0.1
 * @date 09/01/2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TS_CREDENTIALS_H
#define TS_CREDENTIALS_H

/**
 * @brief Maximum length of email user.
 *
 */
#define CONFIG_USER_LENGTH 64

/**
 * @brief Maximum length of email password.
 *
 */
#define CONFIG_PASSWORD_LENGTH 64

/**
 * @brief Stores the email credentials, in base64 format.
 *
 */
struct credentials {
    char user_b64[CONFIG_USER_LENGTH];         /**< Email user (in base64); */
    char password_b64[CONFIG_PASSWORD_LENGTH]; /**< Email app password (in base64); */
};

#endif /* TS_CREDENTIALS_H */
