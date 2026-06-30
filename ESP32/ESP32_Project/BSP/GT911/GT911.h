#ifndef __I2C_H__
#define __I2C_H__

#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"

#define TOUCH_IO_I2C_SDA            GPIO_NUM_19
#define TOUCH_IO_I2C_SCL            GPIO_NUM_45
#define TOUCH_I2C_PORT              I2C_NUM_0
#define CONFIG_LCD_I2C_ADDR         0x5D
#define CONFIG_LCD_I2C_CLK_SPEED    400000
#define CONFIG_LCD_HRES             480
#define CONFIG_LCD_VRES             480

// 触摸RST/INT引脚，不用填-1
#define TOUCH_RST_GPIO              GPIO_NUM_NC
#define TOUCH_INT_GPIO              GPIO_NUM_NC

void touch_scan(void);
void GT911_Read(int16_t *x,int16_t *y,uint16_t *state);

// void indev_read(lv_indev_t *indev,lv_indev_data_t *data)
// {
//     int16_t x=0,y=0,state=0;
//     GT911_Read(&x,&y,&state);
//     data->point.x = x;
//     data->point.y = y;
//     data->state = state;
// }

#endif
