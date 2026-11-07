#include "servo.h"
#include "tim.h"

void Servo_Init(void) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    Servo_Angle(90); // 开机默认转到中间 90 度位置
}

void Servo_Angle(uint8_t angle) {

    uint16_t arr = __HAL_TIM_GET_AUTORELOAD(&htim2);

    if (angle > 180) angle = 180;
    
    uint16_t ccr = (arr + 1) * ((float)angle / 1800.0f + 0.025f);

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, ccr);
}