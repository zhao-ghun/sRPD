#ifndef __VL53L0X_H__
#define __VL53L0X_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "i2c.h"
#include <stdint.h>

uint8_t  VL53L0X_Init(void);
uint16_t VL53L0X_ReadDistance_mm(void);
uint16_t VL53L0X_MedianDistance(void);

#ifdef __cplusplus
}
#endif

#endif /* __VL53L0X_H__ */