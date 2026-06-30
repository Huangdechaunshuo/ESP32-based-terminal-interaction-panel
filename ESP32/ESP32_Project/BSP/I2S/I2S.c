#include "I2S.h"

static i2s_chan_handle_t tx_handle;

/* 将音乐导入缓冲区 */
// extern const uint8_t music_pcm_start[] asm("_binary_music_pcm_start");
// extern const uint8_t music_pcm_end[]   asm("_binary_music_pcm_end");
extern const uint16_t pcm_start[] asm("_binary_DaoShu_pcm_start");
extern const uint16_t pcm_end[]   asm("_binary_DaoShu_pcm_end");

void i2s_audio_tx_init(void)
{
    /* 配置i2s发送通道 */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true; // 自动清除 DMA 缓冲区内残留旧数据

    /* 分配新的 TX 通道并获取该通道的句柄 */
    i2s_new_channel(&chan_cfg, &tx_handle, NULL);

    i2s_std_config_t std_cfg = {
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
            .mclk = I2S_MCK_IO,
            .bclk = I2S_BCK_IO,
            .ws   = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din  = I2S_DI_IO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    /* 初始化通道 */
    i2s_channel_init_std_mode(tx_handle, &std_cfg);

    // =====================  启用I2S发送通道，开始输出时钟信号 =====================
    /* 必须先enable才能调用i2s_channel_write写音频数据
     * 执行后IO1/IO2持续输出BCLK、WS时钟，等待DIN输入PCM音频流 */
     /* 在写入数据之前，先启用 TX 通道 */
    i2s_channel_enable(tx_handle);
}

/**
 * @brief 发送PCM音频数据到I2S功放MAX98357
 * @param src_buf 存放16bit立体声PCM音频缓冲区（MP3解码后的原始音频数据）
 * @param bytes_to_write 需要发送的字节总长度
 * @param bytes_written 实际成功发送的字节数输出指针
 * @param ticks_to_wait 阻塞等待超时时间，portMAX_DELAY永久阻塞直到数据发送完成
 * @return esp_err_t 执行状态
 */
void i2s_write_audio_data(const void *src_buf, size_t bytes_to_write, size_t *bytes_written, uint32_t ticks_to_wait)
{
    /* I2S标准写入函数：DMA搬运音频数据到硬件I2S外设，自动通过dout引脚串行输出
     * 内部DMA缓冲自动处理，无需手动操作中断 */
    i2s_channel_write(tx_handle, src_buf, bytes_to_write, bytes_written, ticks_to_wait);
}

/**
 * @brief 释放I2S全部硬件资源（程序退出/停止播放时调用）
 */
void i2s_audio_tx_deinit(void)
{
    // 1. 先禁用通道，停止时钟与音频输出
    i2s_channel_disable(tx_handle);
    /* 如果不再需要句柄，删除该句柄以释放通道资源 */
    i2s_del_channel(tx_handle);
}

static void i2s_music(void *args)
{
    (void)args;

    // 播放音乐
    size_t  bytes_write = 0;
    // uint8_t  *data_ptr = (uint8_t  *)music_pcm_start;
    uint16_t *data_ptr = (uint16_t *)pcm_start;

    /* 在启用传输通道之前先禁用该通道并预先加载数据 */
    i2s_channel_disable(tx_handle);
    i2s_channel_preload_data(tx_handle, data_ptr, pcm_end - data_ptr, &bytes_write);
    data_ptr += bytes_write;    // 将数据指针向前移动

    /* 启用传输通道 */
    i2s_channel_enable(tx_handle);

    while(1)
    {
        /* 把音乐写入I2S通道 */
        i2s_channel_write(tx_handle, data_ptr, pcm_end - data_ptr, &bytes_write, portMAX_DELAY);

        // data_ptr = (uint8_t  *)music_pcm_start;
        data_ptr = (uint16_t *)pcm_start; //播放完当前剩余音频后，指针重置到歌曲开头，实现循环播放
        vTaskDelay(1000 / portTICK_PERIOD_MS);  //播放完一整段后停顿 1 秒，再重新从头播放
    }
}

void i2s_play_music(void)
{
    i2s_audio_tx_init();
    xTaskCreate(i2s_music, "i2s_music", 4096, NULL, 5, NULL);
}