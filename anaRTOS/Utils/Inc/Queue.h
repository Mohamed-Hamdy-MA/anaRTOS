/*
 * Queue.h
 *
 *  Created on: Jul 11, 2024
 *      Author: Mohamed Hamdy
 */

#ifndef QUEUE_H_
#define QUEUE_H_

//#include <stdio.h>
//#include <stdint.h>
#include "RTOS_Scheduler.h"

#define ElementType RTOS_TaskConfig_t*

typedef struct {
	 uint32_t length;
	 ElementType* base;
	 ElementType* head;
	 ElementType* tail;
	 uint32_t count;
}Queue_t;

typedef enum {
	Queue_No_Error,
	Queue_Full,
	Queue_Empty,
	Queue_Null,
}QueueStatus_t;

QueueStatus_t Queue_init(Queue_t* FIFOStruct, ElementType* DataBuffer, uint32_t DataBufferLength);
QueueStatus_t Queue_add(Queue_t* FIFOStruct, ElementType* Item);
QueueStatus_t Queue_get(Queue_t* FIFOStruct, ElementType* ItemDestination);
QueueStatus_t Queue_print(Queue_t* FIFOStruct);


#endif /* QUEUE_H_ */
