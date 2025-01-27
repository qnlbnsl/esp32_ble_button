// #include <stdio.h>
// #include <string.h>
// #include "esp_log.h"
// #include "nimble/nimble_port.h"
// #include "nimble/nimble_port_freertos.h"
// #include "host/ble_hs.h"
// #include "host/ble_gap.h"
// #include "crypto.h"
// #include "esp_nimble_hci.h"  // Add this line at the top of your file

// static const char *TAG = "BLE";
// static uint8_t ble_addr_type;

// // BLE event callback
// static int ble_gap_event(struct ble_gap_event *event, void *arg) {
//     if (event->type == BLE_GAP_EVENT_ADV_COMPLETE) {
//         ESP_LOGI(TAG, "Advertisement completed.");
//     }
//     return 0;
// }

// // Advertise the BLE payload
// void ble_advertise_once(void) {
//     struct ble_gap_adv_params adv_params = {0};
//     struct ble_hs_adv_fields adv_fields = {0};

//     // Get signed timestamp
//     secure_payload_t payload = create_signed_payload();

//     // Set up advertisement fields
//     adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
//     adv_fields.tx_pwr_lvl_is_present = 1;
//     adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

//     // Use manufacturer-specific data field for our secure payload
//     uint16_t company_id = 0xFFFF;  // Development company ID

//     // Prepare manufacturer data
//     uint8_t mfg_data[sizeof(secure_payload_t) + 2];
//     memcpy(mfg_data, &company_id, 2);
//     memcpy(mfg_data + 2, &payload, sizeof(secure_payload_t));

//     adv_fields.mfg_data = mfg_data;
//     adv_fields.mfg_data_len = sizeof(mfg_data);

//     int rc = ble_gap_adv_set_fields(&adv_fields);
//     if (rc != 0) {
//         ESP_LOGE(TAG, "Error setting advertisement data: %d", rc);
//         return;
//     }

//     adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
//     adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

//     rc = ble_gap_adv_start(ble_addr_type, NULL, 5000, &adv_params, ble_gap_event, NULL);
//     if (rc != 0) {
//         ESP_LOGE(TAG, "Error starting advertising: %d", rc);
//         return;
//     }

//     ESP_LOGI(TAG, "Started advertising signed timestamp: %lu", (unsigned long)payload.timestamp);
// }

// // BLE sync callback
// static void ble_sync_cb(void) {
//     ESP_LOGI(TAG, "BLE host synced.");
//     ble_hs_id_infer_auto(0, &ble_addr_type);
// }

// // NimBLE port run task
// static void nimble_run_task(void *arg) {
//     nimble_port_run();
// }

// // Initialize BLE
// void ble_init(void) {
//     ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
//     nimble_port_init();

//     // Set sync callback
//     ble_hs_cfg.sync_cb = ble_sync_cb;

//     // Initialize NimBLE port for FreeRTOS
//     nimble_port_freertos_init(nimble_run_task);
// }

// void ble_stop_advertisement(void) {
//     int rc = ble_gap_adv_stop();
//     if (rc == 0) {
//         ESP_LOGI(TAG, "Advertisement stopped successfully");
//     } else {
//         ESP_LOGE(TAG, "Failed to stop advertisement: %d", rc);
//     }
// }





#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "crypto.h"

static const char *TAG = "BLE";
static uint8_t ble_addr_type;

// BLE event callback
static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    if (event->type == BLE_GAP_EVENT_ADV_COMPLETE) {
        ESP_LOGI(TAG, "Advertisement completed.");
    }
    return 0;
}

// Advertise the BLE payload
void ble_advertise_once(void) {
    struct ble_gap_adv_params adv_params = {0};
    struct ble_hs_adv_fields adv_fields = {0};

    // Get signed timestamp
    secure_payload_t payload = create_signed_payload();

    // Set up advertisement fields
    adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv_fields.tx_pwr_lvl_is_present = 1;
    adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    // Use manufacturer-specific data field for our secure payload
    uint16_t company_id = 0xFFFF;  // Development company ID

    // Prepare manufacturer data
    uint8_t mfg_data[sizeof(secure_payload_t) + 2];
    memcpy(mfg_data, &company_id, 2);
    memcpy(mfg_data + 2, &payload, sizeof(secure_payload_t));

    adv_fields.mfg_data = mfg_data;
    adv_fields.mfg_data_len = sizeof(mfg_data);

    int rc = ble_gap_adv_set_fields(&adv_fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error setting advertisement data: %d", rc);
        return;
    }

    adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(ble_addr_type, NULL, 5000, &adv_params, ble_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error starting advertising: %d", rc);
        return;
    }

    ESP_LOGI(TAG, "Started advertising signed timestamp: %lu", payload.timestamp);
}

// BLE sync callback
static void ble_sync_cb(void) {
    ESP_LOGI(TAG, "BLE host synced");

    // Get the default address
    ble_hs_id_infer_auto(0, &ble_addr_type);

    // Get BLE public address
    uint8_t addr_val[6] = {0};
    int rc = ble_hs_id_copy_addr(ble_addr_type, addr_val, NULL);

    if (rc == 0) {
        ESP_LOGI(TAG, "Device Address: %02x:%02x:%02x:%02x:%02x:%02x",
                addr_val[5], addr_val[4], addr_val[3],
                addr_val[2], addr_val[1], addr_val[0]);
    } else {
        ESP_LOGE(TAG, "Failed to get device address: %d", rc);
    }
}

// NimBLE port run task
static void nimble_run_task(void *arg) {
    nimble_port_run();
}

// Initialize BLE
void ble_init(void) {
    esp_err_t ret;

    // Initialize NimBLE host stack
    ret = esp_nimble_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NimBLE: %d", ret);
        return;
    }

    // Initialize NimBLE port
    nimble_port_init();

    // Set sync callback
    ble_hs_cfg.sync_cb = ble_sync_cb;

    // Initialize NimBLE port for FreeRTOS
    nimble_port_freertos_init(nimble_run_task);
}

void ble_stop_advertisement(void) {
    int rc = ble_gap_adv_stop();
    if (rc == 0) {
        ESP_LOGI(TAG, "Advertisement stopped successfully");
    } else {
        ESP_LOGE(TAG, "Failed to stop advertisement: %d", rc);
    }
}
