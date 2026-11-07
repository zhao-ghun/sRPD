#ifndef __BUZZER_H__
#define __BUZZER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);

/* 根据距离控制蜂鸣器：distance_mm 必须用 uint16_t（0~65535），
 * 用 uint8_t 会把大于 255mm 的距离截断，导致远处反而误响。*/
void Buzzer_Task(uint16_t distance_mm);

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H__ */
