#ifndef _TASK_H
#define _TASK_H

#include "bsp.h"

#define task_size 100
#define limit_distance 40

typedef struct _pos
{
	int x;
	int y;
} pos;

typedef struct _task
{
	pos cache[task_size];
	pos task[task_size];
	int cache_index;
	bool cache_isEmpty;
	bool cache_isFull;
	bool task_isEmpty;
	bool rec_isFinished;
	int ptr;
	u32 total;
} task;

extern task taskmanager;
extern pos temp;

void Taskmanager_Init(void);
void Append_To_Cache(int x,int y);
void Task_Finished(void);
pos Get_Task(void);


#endif
