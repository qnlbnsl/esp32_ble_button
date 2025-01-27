// # main.c
#include <stdio.h>
#include <time.h>
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "ble.h"
#include "ntp.h"
#include "wifi_setup.h"

#define BUTTON_GPIO GPIO_NUM_0
#define TIME_SYNC_BIT BIT0
static const char *TAG = "MAIN";
static EventGroupHandle_t sync_event_group;

void enter_deep_sleep(void) {
    ESP_LOGI(TAG, "Entering deep sleep...");
    esp_sleep_enable_ext0_wakeup(BUTTON_GPIO, 0);
    esp_deep_sleep_start();
}

// Task to handle WiFi connection and time synchronization
void time_sync_task(void *pvParameters) {
    if (wifi_wait_for_connection()) {
        ESP_LOGI(TAG, "Wi-Fi connected, checking time...");

        time_t before_sync = time(NULL);
        if (ntp_sync_time()) {
            time_t after_sync = time(NULL);
            // time_t time_diff = abs(after_sync - before_sync);
            double time_diff = difftime(after_sync, before_sync);
            if (time_diff > 15) {
                ESP_LOGW(TAG, "Time difference too large (%f seconds). Signaling for advertisement update.", time_diff);
                xEventGroupSetBits(sync_event_group, TIME_SYNC_BIT);
            }
        }
    }
    vTaskDelete(NULL);
}

void app_logic(void) {
    ESP_LOGI(TAG, "ESP32 woke up!");

    // Create sync event group
    sync_event_group = xEventGroupCreate();

    // Start time sync task
    xTaskCreate(time_sync_task, "time_sync", 4096, NULL, 5, NULL);

    // Start initial BLE advertisement
    ESP_LOGI(TAG, "Starting BLE advertisement...");
    ble_advertise_once();

    // Wait for either time sync completion or advertisement timeout
    EventBits_t bits = xEventGroupWaitBits(
        sync_event_group,
        TIME_SYNC_BIT,
        pdTRUE,
        pdFALSE,
        pdMS_TO_TICKS(5000)
    );

    if (bits & TIME_SYNC_BIT) {
        ESP_LOGI(TAG, "Time was updated. Starting new advertisement...");
        ble_stop_advertisement();
        ble_advertise_once();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    vEventGroupDelete(sync_event_group);
    enter_deep_sleep();
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing...");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    wifi_init();
    ble_init();

    app_logic();
}
