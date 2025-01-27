#include <stdio.h>
#include "esp_log.h"
#include "esp_sntp.h"

static const char *TAG = "NTP";

bool ntp_sync_time(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    struct tm timeinfo;
    time_t now = 0;
    int retry = 0;
    const int max_retry = 5;

    while (timeinfo.tm_year < (2023 - 1900) && retry < max_retry) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry + 1, max_retry);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        retry++;
    }

    if (retry >= max_retry) {
        ESP_LOGW(TAG, "Failed to sync time after %d attempts", max_retry);
        return false;
    }

    ESP_LOGI(TAG, "Time synchronized: %s", asctime(&timeinfo));
    return true;
}
