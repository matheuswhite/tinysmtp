/**
 * @file address.h
 * @author Matheus T. dos Santos (matheus.santos@edge.ufal.br)
 * @brief Define the email address structure.
 * @version 0.1
 * @date 09/01/2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TS_ADDRESS_H
#define TS_ADDRESS_H

/**
 * @brief Maximum length of the email address.
 *
 */
#define CONFIG_ADDRESS_LENGTH 64

/**
 * @brief Stores the email address.
 *
 */
struct address {
    char address[CONFIG_ADDRESS_LENGTH]; /**< Buffer to store the email address. */
};

#endif /* TS_ADDRESS_H */
