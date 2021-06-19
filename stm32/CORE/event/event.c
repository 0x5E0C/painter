#include "event.h"

u8 reply_buffer[PACKET_LENGTH];
u8 tx_buffer[PACKET_LENGTH];

void GetBroadcastInfo()//获取无线模块数据并解析为坐标信息
{
	int x,y;
	u1rx_flag=0;
	if(USART1_RX_BUF[0]==0x5E && USART1_RX_BUF[1]==0x0C && CheckData())	//检验包头和校验和
	{
		if(USART1_RX_BUF[3]==TASK_CMD)									//若为任务指令
		{
			x=(USART1_RX_BUF[4]<<8)|USART1_RX_BUF[5];					//解析x
			y=(USART1_RX_BUF[6]<<8)|USART1_RX_BUF[7];					//解析y
			Append_To_Cache(x,y);										//添加至任务缓冲区
			if((TIM1->CR1&0x00000001)==0 && !taskmanager.task_isEmpty)	//若姿态调整定时器关闭关闭但任务区非空
			{
				Set_TIM1_Enable();										//打开定时器
				UpdateAttitude();										//更新姿态
			}
		}
		else if(USART1_RX_BUF[3]==FINISH_CMD)							//若为完成指令
		{
			x=(USART1_RX_BUF[4]<<8)|USART1_RX_BUF[5];					//解析x
			y=(USART1_RX_BUF[6]<<8)|USART1_RX_BUF[7];					//解析y
			taskmanager.rec_isFinished=true;							//数据接收完成标志位置true
			temp.x=x-limit_distance;									//计算该点与上一点的x偏差
			temp.y=y-limit_distance;									//计算该点与上一点的y偏差
			Append_To_Cache(x,y);										//添加至任务缓冲区
//			Reply();
			if(taskmanager.total<=task_size)							//缓冲区未满但以接收完成
			{
				memcpy(taskmanager.task,taskmanager.cache,(taskmanager.total-1)*sizeof(pos));
				taskmanager.cache_isEmpty=true;							//将数据从缓冲区移至任务区
				taskmanager.task_isEmpty=false;
				Set_TIM1_Enable();										//定时更新姿态
				UpdateAttitude();
			}
		}
	}
}

void Reply()//回复
{
	u8 i;
	for(i=0;i<PACKET_LENGTH;i++)
	{
		USART1_Send(reply_buffer[i]);
	}
}

void TxBuffer_Init()//初始化用于回复的数组
{
	u16 sum=0;
	u8 i;
	reply_buffer[0]=0x5E;
	reply_buffer[1]=0x0C;
	reply_buffer[2]=0x01;
	reply_buffer[3]=REPLY_CMD;
	reply_buffer[4]=0x00;
	reply_buffer[5]=0x00;
	reply_buffer[6]=0x00;
	reply_buffer[7]=0x00;
	for(i=0;i<=7;i++)
	{
		sum+=reply_buffer[i];
	}
	reply_buffer[8]=sum/256;
	reply_buffer[9]=sum%256;
}

bool CheckData()//校验解析前的数据包
{
	u16 check,sum=0;
	u8 i;
	for(i=0;i<PACKET_LENGTH-2;i++)
	{
		sum+=USART1_RX_BUF[i];
	}
	check=(USART1_RX_BUF[PACKET_LENGTH-2]<<8)|USART1_RX_BUF[PACKET_LENGTH-1];
	return (check==sum);
}

void SendCurrentPos()//发送自身坐标信息给上位机
{
	int sum=0;
	u8 i;
	tx_buffer[0]=0x5E;
	tx_buffer[1]=0x0C;
	tx_buffer[2]=0x01;
	tx_buffer[3]=SENDPOS_CMD;
	tx_buffer[4]=(current_pos.x&0xFF00)>>8;
	tx_buffer[5]=current_pos.x&0x00FF;
	tx_buffer[6]=(current_pos.y&0xFF00)>>8;
	tx_buffer[7]=current_pos.y&0x00FF;
	for(i=0;i<=7;i++)
	{
		sum+=tx_buffer[i];
		USART1_Send(tx_buffer[i]);
	}
	USART1_Send((sum&0xFF00)>>8);
	USART1_Send(sum&0x00FF);
}

