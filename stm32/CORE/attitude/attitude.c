#include "attitude.h"

Attitude car_attitude;

void UpdateAttitude()							//更新姿态，绝对偏差角度，调动小车在允许误差内行驶
{
	float x_bias,y_bias,angle,bias;
	int target_x,target_y;
	target_x=Get_Task().x;						//获取下一目标点的x
	target_y=Get_Task().y;						//获取下一目标点的y
	x_bias=target_x-current_pos.x;				//计算x偏差
	y_bias=target_y-current_pos.y;				//计算y偏差
	angle=atan2(y_bias,x_bias)*180.0/3.1416;	//计算偏差角度
	if(taskmanager.ptr==(taskmanager.total-2))
	{
		bias=10;								//最终点允许误差
	}
	else																			
	{
		bias=40;								//非最终点允许误差
	}
	if(sqrt(x_bias*x_bias+y_bias*y_bias)<bias)	//到目标点距离小于允许误差
	{
		Task_Finished();						//结束当前点的任务
	}
	else
	{
		Move(BASE_SPEED,angle);					//继续移动
	}
}

