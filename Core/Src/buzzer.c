#include "buzzer.h"
#include "tim.h"

/* 触发距离：小于 200 mm（20 cm）才鸣叫 */
#define BUZZER_TRIGGER_MM   200u

/* 近场屏蔽：小于该值（贴脸/无效值）不鸣叫。
 * 如果希望 0~200mm 全部鸣叫，把这个值改成 0 即可。*/
#define BUZZER_NEAR_MM      20u

static uint8_t s_buzzer_on;

void Buzzer_Init(void)
{
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);   /* TIM3_CH2，2 kHz PWM */
    Buzzer_Off();
}

void Buzzer_On(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 250);  /* ARR=499 -> 50% 占空比 */
    s_buzzer_on = 1;
}

void Buzzer_Off(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    s_buzzer_on = 0;
}

/* 注意：参数必须是 uint16_t。
 * 用 uint8_t 时 275mm 会被截断成 275-256=19mm，300mm 会变成 44mm，
 * 于是“超过量程”的距离反而落进 20~200mm 区间，蜂鸣器在 277~456mm、
 * 533~712mm……这些 256mm 宽的窗口里乱响。*/
void Buzzer_Task(uint16_t distance_mm)
{
    if ((distance_mm > BUZZER_NEAR_MM) && (distance_mm < BUZZER_TRIGGER_MM))
    {
        if (!s_buzzer_on)
        {
            Buzzer_On();
        }
    }
    else
    {
        if (s_buzzer_on)
        {
            Buzzer_Off();
        }
    }
}