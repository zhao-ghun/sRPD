#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void UART_Init(void);
void UART_Task(void);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H__ */