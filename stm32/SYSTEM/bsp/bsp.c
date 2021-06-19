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
    Wireless_USART1_Init(9600);	//zigbee���ڳ�ʼ��9600������
    UWB_USART2_Init(115200);	//uwb���ڳ�ʼ��115200������
    Gyro_USART3_Init(115200);	//�����Ǵ��ڳ�ʼ��115200������
	Trans_TIM1_Init(20000,72);	//��̬������ʱ��20ms�ж�һ��
    Motor_TIM2_Init(10000,72);	//���pwm�����ʱ��������10ms
    Timing_TIM3_Init(20000,72);	//pid���㶨ʱ��20ms�ж�һ��
	while(!u1rx_flag && !u2rx_flag && !u3rx_flag);//�ȴ�ģ�鿪ʼ��������
}

