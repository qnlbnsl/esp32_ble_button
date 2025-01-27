#ifndef BLE_H
#define BLE_H

#include <stdint.h>

// Initialize BLE and start advertising
void ble_init(void);

// Create and advertise the BLE payload
void ble_advertise_once(void);
void ble_stop_advertisement(void);

#endif // BLE_H
