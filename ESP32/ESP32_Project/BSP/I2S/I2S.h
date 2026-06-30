#ifndef __I2S_H__
#define __I2S_H__

#include "driver/i2s_std.h"
#include "driver/gpio.h"    //导入GPIO设备库
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SAMPLE_RATE             (16000)
#define BITS_PER_SAMPLE         (16)
//用来定义输出的声音，范围是从 0~100，这里设置为了 70
#define EXAMPLE_VOICE_VOLUME    (70)

/* I2S port and GPIOs */
#define I2S_NUM                 (0)
#define I2S_MCK_IO              (I2S_GPIO_UNUSED)
#define I2S_BCK_IO              (GPIO_NUM_1)
#define I2S_WS_IO               (GPIO_NUM_2)
#define I2S_DO_IO               (GPIO_NUM_40)
#define I2S_DI_IO               (I2S_GPIO_UNUSED)

void i2s_audio_tx_init(void);
void i2s_write_audio_data(const void *src_buf, size_t bytes_to_write, size_t *bytes_written, uint32_t ticks_to_wait);
void i2s_audio_tx_deinit(void);
void i2s_play_music(void);

#endif
