#ifndef _EVENT_H
#define _EVENT_H

#define TASK_CMD  	0x03
#define REPLY_CMD 	0x04
#define SENDPOS_CMD	0x05
#define FINISH_CMD  0x07

#define WAIT_STATE	WAIT_CMD
#define STOP_STATE  STOP_CMD

#include "bsp.h"

extern u8 tx_buffer[PACKET_LENGTH];

void GetBroadcastInfo(void);
void Reply(void);
void TxBuffer_Init(void);
bool CheckData(void);
void SendCurrentPos(void);
void Append_To_Task(int x,int y);

#endif
