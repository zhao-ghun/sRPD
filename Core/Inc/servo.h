#ifndef __SERVO_H__
#define __SERVO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void Servo_Init(void);
void Servo_Angle(uint8_t angle);

#ifdef __cplusplus
}
#endif

#endif /* __VL53L0X_H__ */