#ifndef __RADAR_H__
#define __RADAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void Radar_Init(void);
uint8_t Radar_Scan(void);
uint8_t Radar_Track(void);

#ifdef __cplusplus
}
#endif

#endif /* __RADAR_H__ */