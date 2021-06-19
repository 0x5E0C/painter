#include "task.h"

task taskmanager;
pos temp;

void Taskmanager_Init()//初始化
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

void Append_To_Cache(int x,int y)//将数据存入缓存区，当缓存区满的时候将数据转移到任务区
{
	if(taskmanager.cache_index!=(task_size-1))
	{
		if((taskmanager.cache_isEmpty || (sqrt((x-current_pos.x)*(x-current_pos.x)+(y-current_pos.y)*(y-current_pos.y))>limit_distance))
			&& (sqrt(temp.x-x)*(temp.x-x)+(temp.y-y)*(temp.y-y)>limit_distance))
		{//计算传入的点与上一个点的距离是否过小
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

void Task_Finished()// 检验为最后一位时进行停止
{
	taskmanager.ptr++;
	if(taskmanager.ptr==task_size)
	{
		taskmanager.ptr=0;
		taskmanager.task_isEmpty=true;
		if(taskmanager.cache_isEmpty)
		{
			Stop();
		}
	}
	else if(taskmanager.task_isEmpty && !taskmanager.cache_isEmpty && taskmanager.ptr==taskmanager.cache_index)
	{
		Stop();
	}
	else if(taskmanager.rec_isFinished && taskmanager.ptr==(taskmanager.total-1))
	{
		Stop();
	}
}

pos Get_Task()//当数组为最后一组的时候将直接执行而不进行转存
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

