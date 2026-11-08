#include "key.h"

static volatile uint8_t key_flag = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t t_last = 0;
    uint32_t t_now = HAL_GetTick();

    if (GPIO_Pin != GPIO_PIN_0) return;
    if ((t_now - t_last) < 200) return; // 软件消抖

    t_last = t_now; // 刷新上次按下时间
    key_flag = 1; // 按钮按下
}

uint8_t Key_Take(void)
{
    if (key_flag)
    {
        key_flag = 0;
        return 1;
    }
    return 0;
}