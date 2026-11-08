#include "radar.h"
#include "key.h"
#include "oled.h"
#include "uart.h"
#include "servo.h"
#include "vl53l0x.h"
#include "buzzer.h"
#include "display.h"
#include "key.h"

static uint16_t dist[19]; // 19个角度的距离

void Radar_Init(void){
    for (uint8_t i = 0; i < 19; i++) {
    dist[i] = 0xFFFF; // 设置为无
    }
}
   
uint8_t Radar_Scan(void){
    for(uint8_t i = 0; i < 19; i++) {
        uint8_t  angle = (uint8_t)(i * 10); // 0~180度
        uint16_t d;

        UART_Task(); // 串口任务：每个角度处理一次
        if (Key_Take()) return 1; 

        Servo_Angle(angle); // 舵机转到这个角度
        HAL_Delay(100); // 等它停稳

        d = VL53L0X_MedianDistance(); // 测三次取中值

        if (d != 0xFFFF){
            if (d > 110) d = (uint16_t)(d - 110); // 减固定偏差
            if (d > 400) d = 0xFFFF; // 超过 40cm 当没目标
        }

        dist[i] = d;

        Buzzer_Task(d); // 20cm 内报警
        Display_Radar(dist, angle, 0); // 刷新一帧雷达图
    }
    return 0;
}

uint8_t Radar_Track(void)
{
    static uint8_t target = 0xFF;
    uint16_t best_d     = 0xFFFF;
    uint8_t  best_angle = 0xFF;
    uint16_t d;

    if (target == 0xFF) // 寻找目标
    {
        for (uint8_t i = 0; i < 19; i++)
        {
            uint8_t  angle = (uint8_t)(i * 10);
            uint16_t dd;

            UART_Task();
            if (Key_Take()) return 1;

            Servo_Angle(angle);
            HAL_Delay(100);

            dd = VL53L0X_MedianDistance();
            if (dd != 0xFFFF) {
                if (dd > 110)  dd = (uint16_t)(dd - 110);
                if (dd > 400) dd = 0xFFFF;
            }
            dist[i] = dd;

            Display_Radar(dist, angle, 1);

            if (dist[i] < best_d)
            {
                best_d     = dist[i];
                best_angle = angle;
            }
        }

        if (best_angle == 0xFF) return 0; // 未找到目标

        target = (uint8_t)(best_angle / 10);   // 目标角度
    }

    Servo_Angle((uint8_t)(target * 10));
    HAL_Delay(100);

    d = VL53L0X_MedianDistance();
    if (d != 0xFFFF) {
        if (d > 110)  d = (uint16_t)(d - 110);
        if (d > 400) d = 0xFFFF;
    }

    dist[target] = d;

    UART_Task();
    Display_Radar(dist, (uint8_t)(target * 10), 1);

    if (d == 0xFFFF) target = 0xFF; // 失去目标重新扫

    if (Key_Take()) // 按键切回扫描模式
    {
        target = 0xFF;
        return 1;
    }

    return 0;
}