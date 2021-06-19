#include "timer3.h"

void Timing_TIM3_Init(u16 arr, u16 psc)//初始化定时器3
{
    RCC->APB1ENR |= 1 << 1;
    TIM3->ARR = arr-1;
    TIM3->PSC = psc-1;
    TIM3->DIER |= 1 << 0;
    MY_NVIC_Init(1, 1, TIM3_IRQn, 2);
    TIM3->CR1 |= 0x01;
}

void TIM3_IRQHandler(void)//进行PID计算后行驶
{
	int bias;
    if(TIM3->SR & 0X0001)
    {
		if(!stop_flag)
		{
			PID_Gyro.ActualValue=GetAngleBias(car_attitude.angle);		//获取偏差角度
			bias=Incremental_PID(&PID_Gyro);							//计算pid反馈值
			SetSpeed(car_attitude.speed+bias,car_attitude.speed-bias);	//设置电机速度
		}
    }
    TIM3->SR &= ~(1 << 0);
}
