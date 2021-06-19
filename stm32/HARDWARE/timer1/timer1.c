#include "timer1.h"

long int mills=0;//计时

void Trans_TIM1_Init(u16 arr, u16 psc)//初始化定时器1：发送
{
    RCC->APB2ENR |= 1 << 11;
    TIM1->ARR = arr-1;
    TIM1->PSC = psc-1;
    TIM1->DIER |= 1 << 0;
    MY_NVIC_Init(2, 2, TIM1_UP_IRQn, 2);    
}

void Set_TIM1_Enable()//打开定时器
{
	stop_flag=false;
	TIM1->CR1 |= 0x01;
}

void Set_TIM1_Disable()//关闭定时器
{
	TIM1->CR1 &= ~(1 << 0);
}

void TIM1_UP_IRQHandler(void)//中断定时器并进行姿态调整，发送当前坐标
{
    if(TIM1->SR & 0X0001)
    {
		mills+=20;
		if((mills%200)==0)
		{
			if(!stop_flag)			//不是停止状态
			{
				UpdateAttitude();	//每20ms更新一次姿态
			}
			SendCurrentPos();		//发送当前位置数据
		}
    }
    TIM1->SR &= ~(1 << 0);
}
