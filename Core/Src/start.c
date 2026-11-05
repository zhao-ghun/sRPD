#include "start.h"
#include "tim.h"
#include <math.h>

#define PI 3.1415926f

void Start_Run(void){
    
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
   
    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(&htim1);

    uint32_t t0 = HAL_GetTick();//ms

    float t = 2.0f;//s
    int n = 3;//times
    float s = t * n * 1000.0f;//ms

    float omega = 2 * PI / t;//rad/s
    while ((HAL_GetTick() - t0) < s)
    {
        float ct = HAL_GetTick() - t0;
        float duty = 0.5f * (1 - cosf(omega * ct / 1000.0f));
        uint16_t ccr = (uint16_t)(duty * (arr + 1));

        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);

        HAL_Delay(1);
    }
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}