#include "GT911.h"

esp_lcd_touch_handle_t  tp_touch = NULL; // 全局触摸句柄
i2c_master_bus_handle_t i2c_bus_handle = NULL; //总线句柄

static void touch_i2c_master_bus_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port   = TOUCH_I2C_PORT,
        .sda_io_num = TOUCH_IO_I2C_SDA,
        .scl_io_num = TOUCH_IO_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    // 创建I2C主机总线
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus_handle));
    ESP_LOGI("TOUCH", "I2C master bus create success");
}

void gt911_touch_init(void)
{
    touch_i2c_master_bus_init();
    
    // 1. I2C引脚配置
    // io_config 就是 I2C 总线操作句柄，后续触摸芯片通信全靠它
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus_handle, &io_config, &io_handle));

    // 2. GT911芯片私有参数配置
    esp_lcd_touch_io_gt911_config_t tp_gt911_config = {
        .dev_addr = io_config.dev_addr, // 从I2C配置里同步GT911的7位设备地址（0x14/0x5D二选一）
    };

    // 3. 触摸参数配置
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = CONFIG_LCD_HRES,   // 屏幕横向最大像素 你的屏480 → 填480
        .y_max = CONFIG_LCD_VRES,   // 屏幕纵向最大像素 你的屏480 → 填480
        .rst_gpio_num  = -1,        // 触摸复位引脚，不用就填-1；有复位就写对应GPIO号
        .int_gpio_num  = -1,        // 触摸中断INT引脚，不用中断就填-1；用中断填GPIO号
        .levels = {
            .reset     =  0,         // 复位引脚有效电平：0=低电平复位，1=高电平复位
            .interrupt =  0,         // 中断引脚有效电平：0=低电平触发，1=高电平中断
        },
        .flags = {
            .swap_xy   =  0,         // 交换X/Y坐标，屏幕上下/左右颠倒时改成1
            .mirror_x  =  0,         // X轴镜像（左右翻转）
            .mirror_y  =  0,         // Y轴镜像（上下翻转）
        },
        .driver_data   =  &tp_gt911_config,    // 绑定上面GT911私有配置
    };

    // // 4. 创建触摸驱动
    esp_lcd_touch_handle_t tp;      // 触摸全局句柄，后续读触摸数据都用它
    // 传入I2C总线句柄、触摸参数，初始化GT911硬件
    // esp_lcd_touch_new_i2c_gt911(io_handle, &tp_cfg, &tp); //执行这一句后，GT911 I2C 通信初始化完成，芯片进入就绪状态
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &tp_cfg, &tp));
    ESP_LOGI("TOUCH", "GT911触摸初始化完成");
    tp_touch = tp;
}

void GT911_Read(int16_t *x,int16_t *y,uint16_t *state)
{
    esp_lcd_touch_point_data_t touch_point_data[1];
    uint16_t touch_cnt = 0;  // 实际触摸到的手指数量

    esp_lcd_touch_read_data(tp_touch);
    esp_lcd_touch_get_data(tp_touch, touch_point_data, &touch_cnt, 1);

    *x = touch_point_data[0].x;
    *y = touch_point_data[0].y;
    *state = touch_cnt;
}

static void touch_scan_task(void *arg)
{
    // 最多支持1点触摸，数组长度改5可支持5点多点触控
    esp_lcd_touch_point_data_t touch_point_data[1];
    uint8_t touch_cnt = 0;  // 实际触摸到的手指数量

    while(1)
    {
        // 第一步：读取芯片最新触摸数据
        // 从GT911寄存器读取最新触摸坐标、触点数量，存入驱动缓存
        esp_lcd_touch_read_data(tp_touch);//作用：更新内部缓存，不直接获取坐标，只是拉取芯片数据
        
        // 第二步：提取触点
        // 从缓存取出触点数据，1=最多读取1个触点
        ESP_ERROR_CHECK(esp_lcd_touch_get_data(tp_touch, touch_point_data, &touch_cnt, 1));

        // touch_cnt > 0 代表有手指触摸屏幕；
        // touch_point_data[0].x / touch_point_data[0].y 就是触摸点像素坐标

        // 先判断是否有触摸，有触点才访问数组，杜绝空访问越界
        if(touch_cnt > 0)
        {
            uint16_t raw_x  = touch_point_data[0].x;
            uint16_t raw_y  = touch_point_data[0].y;

            // 有触摸，打印坐标
            ESP_LOGI("TOUCH", "原始坐标 X:%d Y:%d", raw_x, raw_y);
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // 20ms轮询一次，响应流畅不占CPU
    }
}

void touch_scan(void)
{
    gt911_touch_init();
    // xTaskCreate(touch_scan_task, "touch_scan_task", 4096, NULL, 5, NULL);
}