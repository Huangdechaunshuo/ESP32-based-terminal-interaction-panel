#include "USART.h"

int OPT101_Light           = 0;        // 光强
int DHT11_Temperature      = 0;        // 温度
int DHT11_Humidity         = 0;        // 湿度
float MPU6050_Angle        = 0.0f;     // 角度
float MPU6050_Acceleration = 0.0f;     // 加速度

/**
 * @函数说明        uart_init_config
 * @传入参数        uart_port = 串口号
 *                      可选参数：UART_NUM_0、UART_NUM_1、UART_NUM_2
 *                 baud_rate = 串口波特率
 *                 tx_pin = 发送引脚
 *                 rx_pin = 接收引脚
 * @函数返回        无
 */
void uart_init_config(uart_port_t uart_port, uint32_t baud_rate, int tx_pin, int rx_pin)
{
    //定义 串口配置结构体，必须赋初值，否则无法实现
    uart_config_t uart_config={0};

    uart_config.baud_rate = baud_rate;                  //配置波特率
    uart_config.data_bits = UART_DATA_8_BITS;           //配置数据位为8位
    uart_config.parity    = UART_PARITY_DISABLE;        //配置校验位为不需要校验
    uart_config.stop_bits = UART_STOP_BITS_1;           //配置停止位为 一位
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;   //禁用硬件流控制

    //将以上参数加载到串口的寄存器
    uart_param_config(uart_port, &uart_config);

    //绑定引脚  TX=tx_pin RX=rx_pin RTS=不使用 CTS=不使用
    uart_set_pin(uart_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //安装 串口 驱动程序
    uart_driver_install(uart_port, 200, 200, 0, NULL, 0);
}

/**
 * @函数说明        串口0接收任务
 * @传入参数        无
 * @函数返回        无
 * @函数说明        请通过xTaskCreate创建该任务
 */
void uart0_rx_task(void *pvParameters)
{
    (void)pvParameters;

    char rx_data[200]={0};
    char tx_data[200]={0};
    while(1)
    {
        //接收串口数据收到的数据长度
        int rx_bytes = uart_read_bytes(UART_NUM_0, rx_data, 200, 10 / portTICK_PERIOD_MS);
        if( rx_bytes > 0 )//数据长度大于0，说明接收到数据
        {
            rx_data[rx_bytes] = 0;//将串口数据的最后一个设置为0，形成字符串
            
            // 通过sscanf函数将接收到的字符串数据解析为各个传感器的数据
            float temp[5] = {0};
            sscanf(rx_data,"[%f,%f,%f,%f,%f]",&temp[0],&temp[1],&temp[2],&temp[3],&temp[4]);
            
            if( (temp[0]<0.0f      || temp[0]>100.0f) ||
                (temp[1]<0.0f      || temp[1]>50.0f ) ||
                (temp[2]<20.0f     || temp[2]>90.0f ) ||
                (temp[3]<-180.0f   || temp[3]>180.0f) ||
                (temp[4]<-180.0f   || temp[4]>180.0f) );
            else if( temp[0]==0.0f && temp[1]==0.0f   &&
                     temp[2]==0.0f && temp[3]==0.0f   &&
                     temp[4]==0.0f);
            else
            {
                OPT101_Light         = (int)temp[0];    // 光强
                DHT11_Temperature    = (int)temp[1];    // 温度
                DHT11_Humidity       = (int)temp[2];    // 湿度
                MPU6050_Angle        = temp[3];         // 角度
                MPU6050_Acceleration = temp[4];         // 加速度

                //通过串口0输出接收到的数据
                sprintf(tx_data,"[%d,%d,%d,%.1f,%.1f]\r\n",
                        OPT101_Light,DHT11_Temperature,DHT11_Humidity,
                        MPU6050_Angle,MPU6050_Acceleration);
                uart_write_bytes(UART_NUM_0, (const char*)tx_data, strlen((const char*)tx_data));
            }

            //刷新UART环形缓冲区，丢弃UART RX缓冲区中的所有数据，准备下次接收
            uart_flush(UART_NUM_0);
        }
    }
}

void UART_Iint(void)
{
    //初始化串口0 TX=GPIO_43 RX=GPIO_44
    uart_init_config(UART_NUM_0, 9600, 43, 44);
    //创建串口0接收任务
    xTaskCreate(uart0_rx_task, "uart0_rx_task", 1024*3, NULL, configMAX_PRIORITIES-1, NULL);
}