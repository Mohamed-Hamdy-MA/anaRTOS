/*
 ============================================================================
 Name        : Queue.c
 Author      : Mohamed Hamdy
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Queue.h"

#define IsQueuePointerGoToEnd(FIFOStruct, Pointer) \
    ((Pointer) == ((FIFOStruct)->base + (FIFOStruct)->length) ? 1 : 0)

#define PointerResetToBase(FIFOStruct, ptr) \
    ((ptr) = (FIFOStruct)->base)

#define IsQueueFull(FIFOStruct) \
    ((FIFOStruct)->count == (FIFOStruct)->length ? 1 : 0)

#define IsQueueEmpty(FIFOStruct) \
    ((FIFOStruct)->count == 0 ? 1 : 0)

#define IsQueueNotExist(FIFOStruct) \
    (!(FIFOStruct)->base || !(FIFOStruct)->head || !(FIFOStruct)->tail ? 1 : 0)


QueueStatus_t Queue_init(Queue_t* FIFOStruct, ElementType* DataBuffer, uint32_t DataBufferLength)
{
	FIFOStruct->length = DataBufferLength;
	FIFOStruct->base = DataBuffer;
	FIFOStruct->head = FIFOStruct->base;
	FIFOStruct->tail = FIFOStruct->base;
	FIFOStruct->count = 0;

	if (IsQueueNotExist(FIFOStruct)){
		return Queue_Null;
	}

	return Queue_No_Error;
}

QueueStatus_t Queue_add(Queue_t* FIFOStruct, ElementType* ItemSource)
{
	if (IsQueueNotExist(FIFOStruct)){
		return Queue_Null;
	}

	if (IsQueueFull(FIFOStruct)){
		return Queue_Full;
	}

	if (IsQueueEmpty(FIFOStruct)){
		FIFOStruct->head = FIFOStruct->base;
		FIFOStruct->tail = FIFOStruct->base;
	}

	*(FIFOStruct->head) = *ItemSource;
	FIFOStruct->head++;
	FIFOStruct->count++;
	if (IsQueuePointerGoToEnd(FIFOStruct, FIFOStruct->head))
		PointerResetToBase(FIFOStruct, FIFOStruct->head);
	return Queue_No_Error;
}

QueueStatus_t Queue_get(Queue_t* FIFOStruct, ElementType* ItemDestination)
{
	if (IsQueueNotExist(FIFOStruct)){
		return Queue_Null;
	}
	if (IsQueueEmpty(FIFOStruct)){
		return Queue_Empty;
	}

	*ItemDestination = *(FIFOStruct->tail);
	FIFOStruct->tail++;
	FIFOStruct->count--;
	if (IsQueuePointerGoToEnd(FIFOStruct, FIFOStruct->tail))
		PointerResetToBase(FIFOStruct, FIFOStruct->tail);
	return Queue_No_Error;
}

QueueStatus_t Queue_print(Queue_t* FIFOStruct)
{
	if (IsQueueNotExist(FIFOStruct)){
		return Queue_Null;
	}
	if (IsQueueEmpty(FIFOStruct)){
		return Queue_Empty;}

	ElementType* ptr_temp = FIFOStruct->tail;
	for (uint32_t i = 0; i < FIFOStruct->count; i++)
	{
		if (IsQueuePointerGoToEnd(FIFOStruct, ptr_temp))
			PointerResetToBase(FIFOStruct, ptr_temp);
		ptr_temp++;
	}
	return Queue_No_Error;
}
