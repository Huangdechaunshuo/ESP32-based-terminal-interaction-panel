#include "SPI.h"

// 定义日志TAG
static const char *TAG = "ST7701S";
esp_lcd_panel_handle_t lcd_panel = NULL;


void spi_init(void)
{
    // 一、初始化3线 SPI IO 底层驱动
    ESP_LOGI(TAG, "Install 3-wire SPI panel IO");
    // 1. 定义3线SPI每条线的IO类型与引脚
    spi_line_config_t line_config = {
        .cs_io_type   = IO_TYPE_GPIO,                // IO_TYPE_GPIO 直接ESP32引脚
        .cs_gpio_num  = EXAMPLE_LCD_IO_SPI_CS,       // CS片选引脚号
        .scl_io_type  = IO_TYPE_GPIO,
        .scl_gpio_num = EXAMPLE_LCD_IO_SPI_SCK,      // SPI时钟SCK引脚号
        .sda_io_type  = IO_TYPE_GPIO,
        .sda_gpio_num = EXAMPLE_LCD_IO_SPI_SDO,      // SPI数据SDO（3线SPI只有单向输出）
        .io_expander  = NULL,                        // 不用IO扩展芯片直接填NULL
    };

    // 2. 把SPI配置打包成ST7701专用IO配置结构体
    esp_lcd_panel_io_3wire_spi_config_t io_config = ST7701_PANEL_IO_3WIRE_SPI_CONFIG(line_config, 0);
    // 3. 创建3线SPI IO操作句柄，后续给ST7701驱动调用
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_3wire_spi(&io_config, &io_handle));

    // 二、RGB 并行屏幕时序配置（画面像素传输通道）
    ESP_LOGI(TAG, "Install ST7701 panel driver");
    esp_lcd_rgb_panel_config_t rgb_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT, // 像素时钟源，默认PLL160M不用改
        .psram_trans_align = 64,        // PSRAM内存对齐，LVGL绘图标配64，不用改
        .data_width = 16,               // RGB数据位宽，16=RGB565（最常用）；18=RGB666；24=RGB888
        .bits_per_pixel = 16,           // 单像素位数，和上面data_width保持一致
        // 同步控制引脚（RGB屏幕必备同步信号）
        .de_gpio_num    = EXAMPLE_LCD_IO_RGB_DE,        // DE数据使能
        .pclk_gpio_num  = EXAMPLE_LCD_IO_RGB_PCLK,      // 像素时钟
        .vsync_gpio_num = EXAMPLE_LCD_IO_RGB_VSYNC,     // 场同步（一帧结束）
        .hsync_gpio_num = EXAMPLE_LCD_IO_RGB_HSYNC,     // 行同步（一行结束）
        .disp_gpio_num  = EXAMPLE_LCD_IO_RGB_DISP,      // 屏幕背光/显示使能脚
        // RGB16位数据引脚 DATA0~DATA15，对应硬件排线D0~D15
        .data_gpio_nums = {
            EXAMPLE_LCD_IO_RGB_DATA0,
            EXAMPLE_LCD_IO_RGB_DATA1,
            EXAMPLE_LCD_IO_RGB_DATA2,
            EXAMPLE_LCD_IO_RGB_DATA3,
            EXAMPLE_LCD_IO_RGB_DATA4,
            EXAMPLE_LCD_IO_RGB_DATA5,
            EXAMPLE_LCD_IO_RGB_DATA6,
            EXAMPLE_LCD_IO_RGB_DATA7,
            EXAMPLE_LCD_IO_RGB_DATA8,
            EXAMPLE_LCD_IO_RGB_DATA9,
            EXAMPLE_LCD_IO_RGB_DATA10,
            EXAMPLE_LCD_IO_RGB_DATA11,
            EXAMPLE_LCD_IO_RGB_DATA12,
            EXAMPLE_LCD_IO_RGB_DATA13,
            EXAMPLE_LCD_IO_RGB_DATA14,
            EXAMPLE_LCD_IO_RGB_DATA15,
        },
        // 屏幕时序参数：分辨率+刷新率模板，示例是480*480 60Hz
        .timings = ST7701_480_480_PANEL_60HZ_RGB_TIMING(),
        // .num_fbs = 2,                   // 双缓冲
        .flags.fb_in_psram = 1,         // 强制显存放PSRAM
    };

    // 三、ST7701 芯片厂商专属配置
    st7701_vendor_config_t vendor_config = {
        .rgb_config = &rgb_config,  // 绑定上面的RGB时序配置
        // 自定义初始化指令数组，注释状态=使用驱动内置默认初始化命令
        // .init_cmds = lcd_init_cmds,      // Uncomment these line if use custom initialization commands
        // .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(st7701_lcd_init_cmd_t),
        .flags = {
            .mirror_by_cmd = 1,         // 1=通过寄存器命令实现屏幕镜像翻转，不用软件缓存翻转
            .enable_io_multiplex = 0,
             /**
             * 引脚复用开关：
             * 0：初始化后保留3线SPI引脚，后续可随时修改屏幕参数
             * 1：初始化完成后释放SPI引脚，复用为普通GPIO（节省引脚）
             * 绝大多数场景填0，不要开启
            */
        },
    };

    // 四、顶层屏幕设备配置 + 初始化执行
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_LCD_IO_RST,           // 屏幕复位引脚，无复位脚填-1
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,    // 像素色彩排列：RGB/BGR，花屏换BGR
        .bits_per_pixel = 18,
        .vendor_config  = &vendor_config,               // 绑定厂商配置
    };
    esp_lcd_panel_handle_t panel_handle = NULL;

    // 创建ST7701屏幕驱动句柄
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7701(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));     // 硬件复位屏幕
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));      // 发送SPI初始化指令，点亮屏幕
    // esp_lcd_panel_invert_color(panel_handle, true);         // 颜色反转
    // esp_lcd_panel_swap_xy(panel_handle, true);              // xy坐标翻转
    // esp_lcd_panel_mirror(panel_handle, true, false);        // 镜像
    ESP_LOGI(TAG, "LCD屏幕初始化完成!");

    // gpio_set_direction(GPIO_NUM_38, GPIO_MODE_OUTPUT);
    // gpio_set_level(GPIO_NUM_38, 1);
    // ESP_LOGI(TAG, "背光引脚IO38已拉高，开启屏幕背光");

    // 赋值句柄，可以调用绘图
    lcd_panel = panel_handle;
}

static void test_draw_color_bar(esp_lcd_panel_handle_t panel_handle, uint16_t h_res, uint16_t v_res)
{
    uint8_t byte_per_pixel = (18 + 7) / 8;
    uint16_t row_line = v_res / byte_per_pixel / 8;
    uint8_t *color = (uint8_t *)heap_caps_calloc(1, row_line * h_res * byte_per_pixel, MALLOC_CAP_DMA);

    for (int j = 0; j < byte_per_pixel * 8; j++) {
        for (int i = 0; i < row_line * h_res; i++) {
            for (int k = 0; k < byte_per_pixel; k++) {
                color[i * byte_per_pixel + k] = (BIT(j) >> (k * 8)) & 0xff;
            }
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, j * row_line, h_res, (j + 1) * row_line, color));
    }

    uint16_t color_line = row_line * byte_per_pixel * 8;
    uint16_t res_line = v_res - color_line;
    if (res_line) {
        for (int i = 0; i < res_line * h_res; i++) {
            for (int k = 0; k < byte_per_pixel; k++) {
                color[i * byte_per_pixel + k] = 0xff;
            }
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, color_line, h_res, v_res, color));
    }

    free(color);
}

// 设置液晶屏颜色
void lcd_set_color(uint16_t color)
{
    // 分配内存 这里分配了液晶屏一行数据需要的大小
    uint16_t *buffer = (uint16_t *)heap_caps_malloc(480 * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

    if (NULL == buffer)
    {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
    }
    else
    {
        for (size_t i = 0; i < 480; i++) // 给缓存中放入颜色数据
        {
            buffer[i] = color;
        }

        for (int y = 0; y < 480; y++) // 显示整屏颜色
        {
            esp_lcd_panel_draw_bitmap(lcd_panel, 0, y, 480, y+1, buffer);
        }

        free(buffer); // 释放内存
    }
}

static void lcd_test_task(void *arg)
{
    lcd_set_color(0x0000);
    //  // 红
    // lcd_set_color(0xF800);
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // // 绿
    // lcd_set_color(0x07E0);
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // // 蓝
    // lcd_set_color(0x001F);
    // vTaskDelay(pdMS_TO_TICKS(2000));
    // test_draw_color_bar(lcd_panel, 480, 480);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void lcd_task(void)
{
    spi_init();
    
    // 创建任务，栈8192，优先级5，无参数，内核自动分配核心
    xTaskCreate(lcd_test_task, "lcd_test", 8192*4, NULL, 5, NULL);
    ESP_LOGI(TAG, "LCD测试任务创建成功");
}
