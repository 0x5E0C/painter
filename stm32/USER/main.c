#include "bsp.h"


int main()
{

	Stm32_Clock_Init(9);
    delay_init(72);
    BSP_Init();			//板载功能初始化
	PID_Total_Init();	//初始化所有pid
	TxBuffer_Init();	//初始化发送数据数组
	Taskmanager_Init();	//任务管理器初始化
	Stop();				//车辆停止
	delay_ms(1000);		//等待模块初始化，等待3秒
	delay_ms(1000);
	delay_ms(1000);
	
    while(1)
	{
		if(u2rx_flag)
		{
			UpdatePosinfo();	//更新自身坐标信息
		}
		if(u1rx_flag)
		{
			GetBroadcastInfo();	//获取采样点坐标信息
		}
	}
}

 




