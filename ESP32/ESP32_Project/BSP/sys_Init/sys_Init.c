#include "sys_Init.h"

void sys_Init(void)
{
    Timer_Init();
    // UART_Iint();        // 串口0初始化
    // WIFI_Init();
    // bluetooth_init();
    // TF_Card_Init();

    // i2s_play_music();
    printf("PSRAM空闲内存1: %zu bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("内部SRAM空闲1: %lu bytes\n", esp_get_free_internal_heap_size());
    lcd_task();
    touch_scan();
    printf("PSRAM空闲内存2: %zu bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    printf("内部SRAM空闲2: %lu bytes\n", esp_get_free_internal_heap_size());
}

void delay_ms(int ms)  // 毫秒级延时函数
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}