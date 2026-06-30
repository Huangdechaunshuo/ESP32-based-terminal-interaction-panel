#ifndef __WIFI_H__
#define __WIFI_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"

// #include "lwip/err.h"
// #include "lwip/sys.h"
#include "lwip/sockets.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

void WIFI_Init(void);

#endif
