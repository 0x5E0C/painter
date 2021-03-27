#include "task.h"

task taskmanager;
pos temp;

void Taskmanager_Init()
{
	taskmanager.cache_index=0;
	taskmanager.cache_isEmpty=true;
	taskmanager.cache_isFull=false;
	taskmanager.task_isEmpty=true;
	taskmanager.ptr=0;
	taskmanager.rec_isFinished=false;
	taskmanager.total=0;
	temp.x=0;
	temp.y=0;
}

void Append_To_Cache(int x,int y)
{
	if(taskmanager.cache_index!=(task_size-1))
	{
		if((taskmanager.cache_isEmpty || (sqrt((x-current_pos.x)*(x-current_pos.x)+(y-current_pos.y)*(y-current_pos.y))>limit_distance))
			&& (sqrt(temp.x-x)*(temp.x-x)+(temp.y-y)*(temp.y-y)>limit_distance))
		{
			taskmanager.cache_isEmpty=false;
			taskmanager.cache[taskmanager.cache_index].x=x;
			taskmanager.cache[taskmanager.cache_index].y=y;
			temp.x=x;
			temp.y=y;
			taskmanager.cache_index++;
			taskmanager.total++;
		}
		Reply();
	}
	else
	{
		if(!taskmanager.cache_isFull)
		{
			taskmanager.cache[taskmanager.cache_index].x=x;
			taskmanager.cache[taskmanager.cache_index].y=y;
			taskmanager.cache_isEmpty=false;
			taskmanager.cache_isFull=true;
			taskmanager.total++;
			Reply();
		}
		if(taskmanager.task_isEmpty)
		{
			memcpy(taskmanager.task,taskmanager.cache,task_size*sizeof(pos));
			taskmanager.cache_index=0;
			taskmanager.task_isEmpty=false;
			taskmanager.cache_isEmpty=true;
			taskmanager.cache_isFull=false;
		}
	}
}

void Task_Finished()
{
	taskmanager.ptr++;
	if(taskmanager.ptr==task_size)
	{
		taskmanager.ptr=0;
		taskmanager.task_isEmpty=true;
		if(taskmanager.cache_isEmpty)
		{
			//Set_TIM1_Disable();
			Stop();
		}
	}
	else if(taskmanager.task_isEmpty && !taskmanager.cache_isEmpty && taskmanager.ptr==taskmanager.cache_index)
	{
		//Set_TIM1_Disable();
		Stop();
	}
	else if(taskmanager.rec_isFinished && taskmanager.ptr==(taskmanager.total-1))
	{
		//Set_TIM1_Disable();
		Stop();
	}
}

pos Get_Task()
{
	if(taskmanager.rec_isFinished)
	{
		if(taskmanager.task_isEmpty && !taskmanager.cache_isEmpty)
		{
			return taskmanager.cache[taskmanager.ptr];
		}
	}
	return  taskmanager.task[taskmanager.ptr];
}

