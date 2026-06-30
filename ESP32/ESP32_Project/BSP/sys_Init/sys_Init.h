#ifndef __SYS_INIT_H__
#define __SYS_INIT_H__

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/gptimer.h"

#include "I2S.h"
#include "SPI.h"
#include "WIFI.h"
#include "GT911.h"
#include "USART.h"
#include "Timer.h"
#include "TF_Card.h"
#include "bluetooth.h"

void sys_Init(void);
void delay_ms(int ms);


#endif
