#include "gpio.h"

void GPIO_Init()//初始化GPIO口（PBout）
{
	RCC->APB2ENR|=1<<3;
	
	GPIOB->CRL&=0XFFFFFF00;
	GPIOB->CRL|=0X00000033;
}
