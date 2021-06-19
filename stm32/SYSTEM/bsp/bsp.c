#include "bsp.h"

int fputc(int ch, FILE* f)
{
    while((USART1->SR & 0X40) == 0);
    USART1->DR = (u8) ch;
    return ch;
}

void BSP_Init()
{
	GPIO_Init();
    Wireless_USART1_Init(9600);	//zigbee串口初始化9600波特率
    UWB_USART2_Init(115200);	//uwb串口初始化115200波特率
    Gyro_USART3_Init(115200);	//陀螺仪串口初始化115200波特率
	Trans_TIM1_Init(20000,72);	//姿态调整定时器20ms中断一次
    Motor_TIM2_Init(10000,72);	//电机pwm输出定时器，周期10ms
    Timing_TIM3_Init(20000,72);	//pid计算定时器20ms中断一次
	while(!u1rx_flag && !u2rx_flag && !u3rx_flag);//等待模块开始发送数据
}

