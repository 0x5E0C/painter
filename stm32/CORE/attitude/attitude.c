#include "attitude.h"

Attitude car_attitude;

void UpdateAttitude()
{
	float x_bias,y_bias,angle,bias;
	int target_x,target_y;
	target_x=Get_Task().x;
	target_y=Get_Task().y;
	x_bias=target_x-current_pos.x;
	y_bias=target_y-current_pos.y;
	angle=atan2(y_bias,x_bias)*180.0/3.1416;
	if(taskmanager.ptr==(taskmanager.total-2))
	{
		bias=10;
	}
	else
	{
		bias=40;
	}
	if(sqrt(x_bias*x_bias+y_bias*y_bias)<bias)
	{
		Task_Finished();
	}
	else
	{
		Move(BASE_SPEED,angle);
	}
}

