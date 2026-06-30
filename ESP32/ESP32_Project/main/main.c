#include "sys_Init.h"

void app_main(void)
{
    sys_Init();

    while(1)
    {
        delay_ms(1000); // 主线程必须加延时，让出CPU给IDLE
    }
}