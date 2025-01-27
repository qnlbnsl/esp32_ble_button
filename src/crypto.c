// # crypto.c
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk.h"
#include "crypto.h"

static const char *TAG = "CRYPTO";

// Use ECDSA with P-256 curve for smaller signatures
const char *private_key_pem = "-----BEGIN EC PRIVATE KEY-----\n"
                             "<your-EC-private-key>\n"
                             "-----END EC PRIVATE KEY-----";

secure_payload_t create_signed_payload(void) {
    secure_payload_t payload = {0};
    int ret;
    mbedtls_ecdsa_context ctx;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    unsigned char hash[32];
    size_t sig_len;

    // Initialize contexts
    mbedtls_ecdsa_init(&ctx);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // Get current timestamp
    payload.timestamp = (uint32_t)time(NULL);

    do {
        // Seed the RNG
        ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                   (const unsigned char *)"esp32_crypto", 11);
        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to seed RNG: -0x%04x", -ret);
            break;
        }

        // Load the private key
        mbedtls_pk_context pk;
        mbedtls_pk_init(&pk);

        ret = mbedtls_pk_parse_key(&pk, (const unsigned char *)private_key_pem,
                                  strlen(private_key_pem) + 1, NULL, 0,
                                  mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to parse private key: -0x%04x", -ret);
            mbedtls_pk_free(&pk);
            break;
        }

        // Setup ECDSA context from private key
        ret = mbedtls_ecdsa_from_keypair(&ctx, mbedtls_pk_ec(pk));
        mbedtls_pk_free(&pk);

        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to setup ECDSA context: -0x%04x", -ret);
            break;
        }

        // Hash the timestamp
        mbedtls_sha256((unsigned char *)&payload.timestamp, sizeof(payload.timestamp), hash, 0);

        // Sign the hash
        ret = mbedtls_ecdsa_write_signature(
            &ctx,                          // ECDSA context
            MBEDTLS_MD_SHA256,            // Hash algorithm
            hash,                         // Input hash
            sizeof(hash),                 // Hash length
            payload.signature,            // Output signature buffer
            sizeof(payload.signature),    // Signature buffer size
            &sig_len,                    // Actual signature length
            mbedtls_ctr_drbg_random,     // RNG function
            &ctr_drbg                    // RNG context
        );

        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to create signature: -0x%04x", -ret);
            break;
        }

        ESP_LOGI(TAG, "Timestamp signed successfully: %lu", (unsigned long)payload.timestamp);

    } while(0);

    // Cleanup
    mbedtls_ecdsa_free(&ctx);
    mbedtls_entropy_free(&entropy);
    mbedtls_ctr_drbg_free(&ctr_drbg);

    return payload;
}
