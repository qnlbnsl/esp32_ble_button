#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

/**
 * @brief Initialize Wi-Fi in station mode with hardcoded credentials.
 */
void wifi_init(void);

/**
 * @brief Wait for Wi-Fi to connect within a timeout.
 *
 * @return true if connected successfully, false otherwise.
 */
bool wifi_wait_for_connection(void);

#endif // WIFI_H
