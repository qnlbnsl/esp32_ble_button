#ifndef ESP32_CRYPTO_H
#define ESP32_CRYPTO_H

#include <stdint.h>

// Structure to hold the signed timestamp
typedef struct {
    uint32_t timestamp;
    uint8_t signature[64];  // ECDSA P-256 signature is 64 bytes
} __attribute__((packed)) secure_payload_t;

// Function to create a signed payload
secure_payload_t create_signed_payload(void);

#endif // ESP32_CRYPTO_H
