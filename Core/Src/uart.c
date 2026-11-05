#include "uart.h"
#include "stm32f1xx_hal_uart.h"
#include "usart.h"
#include "display.h"
#include <string.h>
#include <stdio.h>

extern DMA_HandleTypeDef hdma_usart1_rx; 

uint8_t receiveData[100];
volatile uint8_t rx_flag = 0; // 接收完成标志位
volatile uint16_t rx_size = 0; // 接收到的数据长度
char reply[128]; //声明128字节的数组用于存储回复信息

void UART_Init(void){
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, receiveData, sizeof(receiveData)-1);
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
    if(huart == &huart1){
        rx_size = Size;
        rx_flag = 1;

        HAL_UART_AbortReceive(&huart1); // 停止当前的DMA接收

        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, receiveData, sizeof(receiveData)-1);
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
}

void UART_Task(void){
    if(rx_flag){
        rx_flag = 0;
        
        receiveData[rx_size] = '\0'; 

        for(int i = 0; i < rx_size; i++){
            if(receiveData[i] == '\r' || receiveData[i] == '\n'){
                receiveData[i] = '\0';
                break;
            }
        }

        if(strlen((char*)receiveData) > 0){
            sprintf(reply, "%s is ok!", receiveData); // 将接收到的数据拼接成回复信息

            HAL_UART_Transmit(&huart1, (uint8_t*)reply, strlen(reply), 100); //发送回复信息

            Display_ShowMessage(reply); //通过OLED显示回复信息
        }

        memset(receiveData, 0, sizeof(receiveData));
    }
}