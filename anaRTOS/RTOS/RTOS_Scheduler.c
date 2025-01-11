/*
 * RTOS_Scheduler.c
 *
 *  Created on: Dec 2, 2024
 *      Author: Mohamed Hamdy
 */


//==========================================================================================
//=========================================Includes=========================================
//==========================================================================================
#include "RTOS_Scheduler.h"
#include "Queue.h"


//==========================================================================================
//==========================================Macros==========================================
//==========================================================================================
#define anaRTOS_TasksCapacity	100U

#define anaRTOS_SVC_ActivateTaskeServiceNumber	0U
#define anaRTOS_SVC_TerminateTaskeServiceNumber	1U
#define anaRTOS_SVC_HoldTaskeServiceNumber		2U


//==========================================================================================
//=====================================Type Definitions=====================================
//==========================================================================================
typedef enum
{
	OS_Suspend,
	OS_Running
} RTOS_OSState_t;

typedef struct
{
	RTOS_TaskConfig_t* OS_SchedulerTable[anaRTOS_TasksCapacity];
	uint32_t OS_startMSP;
	uint32_t OS_endMSP;
	uint32_t OS_NewTaskPSPLocator;
	uint32_t OS_NoOfActiveTasks;
	RTOS_TaskConfig_t* OS_CurrentExecutedTask;
	RTOS_TaskConfig_t* OS_NextExecutedTask;
	RTOS_OSState_t OS_State;
} RTOS_OSControl_t;


//==========================================================================================
//=====================================Global Instants======================================
//==========================================================================================
RTOS_OSControl_t anaRTOS;

RTOS_TaskConfig_t* anaRTOS_ReadyArr[anaRTOS_TasksCapacity];
Queue_t anaRTOS_ReadyFIFO;

RTOS_TaskConfig_t anaRTOS_IdleTask;


//==========================================================================================
//===================================Functions Declaration==================================
//==========================================================================================
void SVC_Handler_C(uint32_t* SVC_args);
void MSP_Init(void);
void TCB_Init(RTOS_TaskConfig_t* TaskConfig);
void anaRTOS_IdleTaskFunc(void);
void anaRTOS_UpdateReadyQueue(void);
void anaRTOS_UpdateNextExecutedTask(void);
void BubbleSortArr(RTOS_TaskConfig_t** SchedulerTableArr, uint32_t ArrSize);
void anaRTOS_AddHighPriorityTaskToReadyQueue(void);
void anaRTOS_UpdateTaskHoldingTime(void);


//==========================================================================================
//===========================================APIs===========================================
//==========================================================================================
void anaRTOS_Init(void)
{
	// 1. Update OS state.
	anaRTOS.OS_State = OS_Suspend;

	// 2. Create the OS main stack.
	MSP_Init();

	// 3. Create Ready Queue.
	Queue_init(&anaRTOS_ReadyFIFO, anaRTOS_ReadyArr, anaRTOS_TasksCapacity);

	// 4. create Idle Task.
	strcpy(anaRTOS_IdleTask.TaskUserConfig.Task_Name, "IDLE Task");
	anaRTOS_IdleTask.TaskUserConfig.Task_InputPriority = 255;	// Lowest priority
	anaRTOS_IdleTask.TaskUserConfig.Task_ProgramEntry = &anaRTOS_IdleTaskFunc;
	anaRTOS_IdleTask.TaskUserConfig.Task_StackSize = 100;	// 100 Bytes

	anaRTOS_CreateTask(&anaRTOS_IdleTask);

}
void anaRTOS_CreateTask(RTOS_TaskConfig_t* TaskConfig)
{
	// 1. Create Task stack using PSP.
	TaskConfig->TaskOSConfig.Task_startPSP = anaRTOS.OS_NewTaskPSPLocator;
	TaskConfig->TaskOSConfig.Task_endPSP = (TaskConfig->TaskOSConfig.Task_startPSP - TaskConfig->TaskUserConfig.Task_StackSize);
	TaskConfig->TaskOSConfig.Task_currentPSP = (uint32_t*)TaskConfig->TaskOSConfig.Task_startPSP;

	// 2. todo: Check if Task stack exceed heap region or not.

	// 3. Align 8 Bytes and update OS_NewTaskPSPLocator.
	anaRTOS.OS_NewTaskPSPLocator = (TaskConfig->TaskOSConfig.Task_endPSP - 8);

	// 4. Create Task control Block (TCB) on task stack --> Task Frame (Init/Save all processor register for this task).
	TCB_Init(TaskConfig);

	// 5. Initialization of task priority.
	TaskConfig->TaskOSConfig.Task_Priority = TaskConfig->TaskUserConfig.Task_InputPriority;

	// 6. Update Task state.
	TaskConfig->TaskOSConfig.Task_State = Task_Suspend;

	// 7. Send the task to OS scheduler table.
	anaRTOS.OS_SchedulerTable[anaRTOS.OS_NoOfActiveTasks] = TaskConfig;
	anaRTOS.OS_NoOfActiveTasks++;
}
void anaRTOS_ActivateTask(RTOS_TaskConfig_t* TaskConfig)
{
	// 1. Update Task state to waiting.
	TaskConfig->TaskOSConfig.Task_State = Task_Waiting;

	// 2. SVCall with Activate task service number.
	SVCall(anaRTOS_SVC_ActivateTaskeServiceNumber);
}
void anaRTOS_TerminateTask(RTOS_TaskConfig_t* TaskConfig)
{
	// 1. Update Task state to suspend.
	TaskConfig->TaskOSConfig.Task_State = Task_Suspend;

	// 2. SVCall with Terminate task service number.
	SVCall(anaRTOS_SVC_TerminateTaskeServiceNumber);
}
void anaRTOS_HoldTask(RTOS_TaskConfig_t* TaskConfig, uint32_t NoOfTicks)
{
	// 1. Update Task timing hold.
	TaskConfig->TaskOSConfig.Task_TimingHold.TimingHold_Status = TimingHold_Enabled;
	TaskConfig->TaskOSConfig.Task_TimingHold.TimingHold_RemainingTicks = NoOfTicks;

	// 2. Update Task state to Suspend.
	TaskConfig->TaskOSConfig.Task_State = Task_Suspend;

	// 3. SVCall with Terminate task service number.
	SVCall(anaRTOS_SVC_TerminateTaskeServiceNumber);
}
void anaRTOS_AcquireSemaphore(RTOS_SemaphoreConfig_t* SemaphoreConfig)
{
	// 0. Update Task semaphore configuration of the caller that request to acquire the semaphore (The semaphore, Status, Task priority Inheritance = Input priority).
	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Semaphore = (struct RTOS_SemaphoreConfig_t*)SemaphoreConfig;

	if (SemaphoreConfig->SemaphoreUserConfig.Semaphore_Value == 0) 	// Semaphore is not Available.
	{
		// Assumption: the caller task always have higher priority than next user and higher or equal than current user.

		// 1. Make the caller task to be the next task that should acquire the semaphore.
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_NextUser = anaRTOS.OS_CurrentExecutedTask;
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_NoOfWaitingToReleaseTasks++;

		anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Status = SemaphoreRequest_WaitingRelease;


		// 2. Increase the priority of the last owner of the semaphore to be more than the caller task.
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_CurrentUser->TaskOSConfig.Task_Priority = (anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_Priority - 1);

		// 3. Update the state of the caller task to Suspend (Terminate the caller task until the release of the semaphore).
		anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_State = Task_Suspend;

		// 4. Raise SVCall to update the ready queue and OS next executed next (Terminate the caller task and Activate current user task).
		SVCall(anaRTOS_SVC_TerminateTaskeServiceNumber);
	}
	else // Semaphore is Available.
	{
		// 1. Update (Minus) the semaphore shared value (represent the current number of tasks that can acquire semaphore).
		SemaphoreConfig->SemaphoreUserConfig.Semaphore_Value--;

		// 2. Make the caller task to be one of the owners (the last owner) of the semaphore.
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_CurrentUser = anaRTOS.OS_CurrentExecutedTask;

		anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Status = SemaphoreRequest_Acquired;
	}
}
void anaRTOS_ReleaseSemaphore(RTOS_SemaphoreConfig_t* SemaphoreConfig)
{
	// 1. Update (Plus) the semaphore shared value (represent the current number of tasks that can acquire semaphore).
	SemaphoreConfig->SemaphoreUserConfig.Semaphore_Value++;

	// 2. Reset Task semaphore configuration of the user that request to release to the default (No semaphore, No request, Task Input priority).
	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Semaphore = NULL;
	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Status = SemaphoreRequest_NoRequest;
	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_Priority = anaRTOS.OS_CurrentExecutedTask->TaskUserConfig.Task_InputPriority;

	if (SemaphoreConfig->SemaphoreOSConfig.Semaphore_NextUser != NULL)
	{
		// 3. Update the state of the next user (Waiting) (Always higher or same original priority than the task that request to release) to acquire the semaphore.
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_NextUser->TaskOSConfig.Task_State = Task_Waiting;

		// 4. Switch the next user to be one of the owners (the last owner) of the semaphore.
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_CurrentUser = SemaphoreConfig->SemaphoreOSConfig.Semaphore_NextUser;
		SemaphoreConfig->SemaphoreOSConfig.Semaphore_NoOfWaitingToReleaseTasks--;

		// 5. Choose and update the new next user to acquire the semaphore.
		if (SemaphoreConfig->SemaphoreOSConfig.Semaphore_NoOfWaitingToReleaseTasks == 0)
		{
			SemaphoreConfig->SemaphoreOSConfig.Semaphore_NextUser = NULL;
		}
		else
		{
			// Choose the highest suspended task in the scheduler table that request the semaphore and its status is to acquire the semaphore.
			for (uint32_t i = 0; i < anaRTOS.OS_NoOfActiveTasks -1; i++)
			{
				if(anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_State == Task_Suspend)
				{
					if (anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Semaphore == (struct RTOS_SemaphoreConfig_t*)SemaphoreConfig)
					{
						if (anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_SemaphoreRequest_t.SemaphoreRequest_Status == SemaphoreRequest_WaitingRelease)
						{
							SemaphoreConfig->SemaphoreOSConfig.Semaphore_NextUser = anaRTOS.OS_SchedulerTable[i];
							break;
						}

					}
				}
			}
		}

		// 6. Raise SVCall to update the ready queue and OS next executed next (Activate current user "the original next user" task).
		SVCall(anaRTOS_SVC_ActivateTaskeServiceNumber);
	}
}

void anaRTOS_StartOS(void)
{
	// 1. Update OS state to Running.
	anaRTOS.OS_State = OS_Running;

	// 2. Execute the idle task.
	anaRTOS.OS_CurrentExecutedTask = &anaRTOS_IdleTask;
	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_State = Task_Running;
	__set_PSP( (uint32_t)anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_startPSP );

	/* The idle task will run until the the first SysTick handler which will start
	 * execution of applications tasks. the idle task states will be waiting, since
	 * it should run if there is no tasks in ready queue and the current task is suspended.
	 *
	 * The current task can not be add to the ready queue by anaRTOS_AddHighPriorityTaskToReadyQueue,
	 * since its states during execute anaRTOS_UpdateReadyQueue process can only be (suspend or running).
	 * The current task can be changed to ready or waiting during anaRTOS_UpdateNextExecutedTask process
	 * in case of round-robin or if it preempted by higher priority task respectively.
	 */

	// 3. Start OS ticker.
	RTOS_StartOSTicker(8, 1000); //CPUClock_MHz = 8, Period_us = 1000 us = 1 ms.

	// 4. Switch to PSP.
	Switch_To_PSP;

	// 5. Switch to unprivileged access.
	Switch_To_UnPrivileged;

	// 6. Call idle task function.
	anaRTOS_IdleTask.TaskUserConfig.Task_ProgramEntry();
}

//==========================================================================================
//===================================Functions Definitions==================================
//==========================================================================================
void SVC_Handler_C(uint32_t* SVC_args)
{
	/*
	 * SVC_args is a pointer to the stack frame in which the interrupted context saved
	 * after exception acceptance and before execution the exception handler.
	uint32_t Stacked_R0 	= SVC_args[0];
	uint32_t Stacked_R1 	= SVC_args[1];
	uint32_t Stacked_R2 	= SVC_args[2];
	uint32_t Stacked_R3 	= SVC_args[3];
	uint32_t Stacked_R12 	= SVC_args[4];
	uint32_t Stacked_LR 	= SVC_args[5];
	uint32_t Stacked_PC 	= SVC_args[6];
	uint32_t Stacked_xPSR 	= SVC_args[7];
	*/

	uint32_t SVCNumber 	= ( (uint8_t*) SVC_args[6] )[-2];

	switch (SVCNumber)
	{
	case anaRTOS_SVC_ActivateTaskeServiceNumber:					//Case 0
	case anaRTOS_SVC_TerminateTaskeServiceNumber:					//Case 1

		// 1. Update the scheduler table and ready queue.
		anaRTOS_UpdateReadyQueue();

		if (anaRTOS.OS_State == OS_Running)
		{
			// 2. Update the next executed task.
			anaRTOS_UpdateNextExecutedTask();

			// 3. Trigger PendSV to context switch (SAVE current task context and RESTORE the next task context).
			if (anaRTOS.OS_NextExecutedTask != anaRTOS.OS_CurrentExecutedTask)
			{
				Trigger_PendSV;
			}
		}
		break;

	case anaRTOS_SVC_HoldTaskeServiceNumber:					//Case 2

		// 1. Update the scheduler table and ready queue.
		anaRTOS_UpdateReadyQueue();
		break;

	default:
		break;
	}
}
void MSP_Init(void)
{
	anaRTOS.OS_startMSP = (uint32_t)(&_estack);
	anaRTOS.OS_endMSP = (anaRTOS.OS_startMSP - anaRTOS_MSPStackSize);
	anaRTOS.OS_NewTaskPSPLocator = (anaRTOS.OS_endMSP - 8);
}
void TCB_Init(RTOS_TaskConfig_t* TaskConfig)
{
	/*
	 * xPSR
	 * PC
	 * LR
	 * R12
	 * R3
	 * R2
	 * R1
	 * R0
	 * -----
	 * R11
	 * R10
	 * R9
	 * R8
	 * R7
	 * R6
	 * R5
	 * R4
	 */

	// Always ensure that Task_currentPSP at the top of task stack (Descending RAM on Cortex Mx)
	TaskConfig->TaskOSConfig.Task_currentPSP = (uint32_t*)TaskConfig->TaskOSConfig.Task_startPSP;


	*(TaskConfig->TaskOSConfig.Task_currentPSP) = 0x01000000;	// xPSR: Reset value --> Thumb bit (bit 24) is always 1 to avoid hard fault.

	TaskConfig->TaskOSConfig.Task_currentPSP--;
	*(TaskConfig->TaskOSConfig.Task_currentPSP) = (uint32_t)TaskConfig->TaskUserConfig.Task_ProgramEntry;	// PC

	TaskConfig->TaskOSConfig.Task_currentPSP--;
	*(TaskConfig->TaskOSConfig.Task_currentPSP) = 0xFFFFFFFD;	// LR: EXC_RETURN --> Thread mode with PSP.

	for (uint8_t i = 0; i < 13; i++)
	{
		TaskConfig->TaskOSConfig.Task_currentPSP--;
		*(TaskConfig->TaskOSConfig.Task_currentPSP) = 0x00000000;
	}

}

uint32_t IdleTaskLED;
void anaRTOS_IdleTaskFunc(void)
{
	while(1){
		IdleTaskLED ^= 1;
		__asm volatile ("NOP");
	}
}

void anaRTOS_UpdateReadyQueue(void)
{
	/* NOTE: Current executed task will be never in ready queue. Since before execute it,
	 * it dequeued from ready queue so it will not be in free ready queue stage.
	 *
	 * The current task can not be add to the ready queue by anaRTOS_AddHighPriorityTaskToReadyQueue,
	 * since its states during execute anaRTOS_UpdateReadyQueue process can only be (suspend or running).
	 * The current task can be changed to ready or waiting during anaRTOS_UpdateNextExecutedTask process
	 * in case of round-robin or if it preempted by higher priority task respectively.
	 */
	RTOS_TaskConfig_t* DeQueueDestination = NULL;

	// 1. Sort the OS scheduler table based on priority.
	BubbleSortArr(anaRTOS.OS_SchedulerTable, anaRTOS.OS_NoOfActiveTasks);

	// 2. Free the ready queue.
	while(Queue_get(&anaRTOS_ReadyFIFO, &DeQueueDestination) != Queue_Empty)
	{
		// If suspend leave it suspend, if ready make it waiting (in ready queue tasks can be only ready or suspended).
		if (DeQueueDestination->TaskOSConfig.Task_State != Task_Suspend)
		{
			DeQueueDestination->TaskOSConfig.Task_State = Task_Waiting;
		}
	}

	// 3. Add the highest priority waiting task to the ready queue.
	anaRTOS_AddHighPriorityTaskToReadyQueue();
}
void anaRTOS_UpdateNextExecutedTask(void)
{
    // NOTE: Current executed task will never be in the ready queue. Since before execution, it is dequeued from the ready queue.

    if (anaRTOS_ReadyFIFO.count == 0) {
        // No high priority waiting task more than the current task (check if current task is "suspend" or "running").
        if (anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_State != Task_Suspend) {
            // Current task is "running".
            anaRTOS.OS_NextExecutedTask = anaRTOS.OS_CurrentExecutedTask;
        } else {
            // Current task is "suspend".
            anaRTOS.OS_NextExecutedTask = &anaRTOS_IdleTask;
            anaRTOS_IdleTask.TaskOSConfig.Task_State = Task_Running;
        }
        return;
    }

    // One or more high/low/same priority waiting tasks (100% not the current task).
    Queue_get(&anaRTOS_ReadyFIFO, &anaRTOS.OS_NextExecutedTask);

    if (anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_State == Task_Suspend) {
        // Current task is "suspend".
        anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_State = Task_Running;
        return;
    }

    // Current task is "running".
    if (anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_Priority < anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_Priority) {
        Queue_add(&anaRTOS_ReadyFIFO, &anaRTOS.OS_NextExecutedTask);
        anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_State = Task_Ready;
        anaRTOS.OS_NextExecutedTask = anaRTOS.OS_CurrentExecutedTask;
    } else if (anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_Priority == anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_Priority) {
        Queue_add(&anaRTOS_ReadyFIFO, &anaRTOS.OS_CurrentExecutedTask);
        anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_State = Task_Ready;
        anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_State = Task_Running;
    } else {
        anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_State = Task_Waiting;
        anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_State = Task_Running;
    }
}

void BubbleSortArr(RTOS_TaskConfig_t** SchedulerTableArr, uint32_t ArrSize)
{
	RTOS_TaskConfig_t* temp = NULL;

	for (uint32_t Iteration = 0; Iteration < ArrSize - 1; Iteration++)
	{
		for (uint32_t Index = 0; Index < ArrSize - Iteration - 1; Index++)
			{
				if (SchedulerTableArr[Index]->TaskOSConfig.Task_Priority > SchedulerTableArr[Index+1]->TaskOSConfig.Task_Priority)
				{
					temp = SchedulerTableArr[Index];
					SchedulerTableArr[Index] = SchedulerTableArr[Index+1];
					SchedulerTableArr[Index+1] = temp;
				}
			}
	}
}
void anaRTOS_AddHighPriorityTaskToReadyQueue(void)
{
	uint32_t TaskIndexInSchedulerTable = 0;
	RTOS_TaskConfig_t* ptrTask = NULL;
	RTOS_TaskConfig_t* ptrNextTask= NULL;

	/*  Why NoOfActiveTasks-1?
	 *  because we do not want to add the Idle task (Last task) to ready queue
	 *  since we will execute by in case of 0 tasks in ready queue and the current task is suspended*/
	while(TaskIndexInSchedulerTable < anaRTOS.OS_NoOfActiveTasks-1)
	{
		ptrTask = anaRTOS.OS_SchedulerTable[TaskIndexInSchedulerTable];
		ptrNextTask = anaRTOS.OS_SchedulerTable[TaskIndexInSchedulerTable+1];

		if (ptrTask->TaskOSConfig.Task_State == Task_Waiting)	// != suspend or running
		{
			// In case ptrTask is higher priority than ptrNextTask (Priority Preemption) OR ptrTask (not suspend) reach to the last element and ptrNextTask not have priority "empty".
			if ( (ptrTask->TaskOSConfig.Task_Priority < ptrNextTask->TaskOSConfig.Task_Priority) || (TaskIndexInSchedulerTable == anaRTOS.OS_NoOfActiveTasks-1) )
			{
				Queue_add(&anaRTOS_ReadyFIFO, &ptrTask);
				ptrTask->TaskOSConfig.Task_State = Task_Ready;
				break;
			}

			// In case ptrTask have the same priority of ptrNextTask (Apply Round-Robin).
			else if (ptrTask->TaskOSConfig.Task_Priority == ptrNextTask->TaskOSConfig.Task_Priority)
			{
				Queue_add(&anaRTOS_ReadyFIFO, &ptrTask);
				ptrTask->TaskOSConfig.Task_State = Task_Ready;
				// No Break --> repeat for next iteration to add the other same priority task.
			}
		}

		TaskIndexInSchedulerTable++;
	}
}
void anaRTOS_UpdateTaskHoldingTime(void)
{
	for (uint32_t i = 0; i < anaRTOS.OS_NoOfActiveTasks -1; i++)
	{
		if(anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_State == Task_Suspend)
		{
			if (anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_TimingHold.TimingHold_Status == TimingHold_Enabled)
			{
				anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_TimingHold.TimingHold_RemainingTicks--;

				if (anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_TimingHold.TimingHold_RemainingTicks == 0)
				{
					anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_TimingHold.TimingHold_Status = TimingHold_Disabled;
							anaRTOS.OS_SchedulerTable[i]->TaskOSConfig.Task_State = Task_Waiting;
					SVCall(anaRTOS_SVC_HoldTaskeServiceNumber);
				}

			}
		}
	}
}


//==========================================================================================
//=====================================Exception handlers===================================
//==========================================================================================
__attribute ((naked)) void PendSV_Handler(void)
{
	/*
	 * -----> Task_currentPSP (Before Exception)
	 * xPSR
	 * PC
	 * LR
	 * R12
	 * R3
	 * R2
	 * R1
	 * R0
	 * -----> CPU_PSP
	 * R11 (Manual PUSH/POP)
	 * R10 (Manual PUSH/POP)
	 * R9 (Manual PUSH/POP/POP)
	 * R8 (Manual PUSH/POP)
	 * R7 (Manual PUSH/POP)
	 * R6 (Manual PUSH/POP)
	 * R5 (Manual PUSH/POP)
	 * R4 (Manual PUSH/POP)
	 *  -----> Task_currentPSP (After Exception and Context Save)
	 */

	// 1. Context SAVE of the current task.
	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP = (uint32_t*)__get_PSP();


	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R11" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R10" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R9" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R8" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R7" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R6" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R5" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );

	anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP--;
	__asm volatile( "MOV %0, R4" : "=r" ( *(anaRTOS.OS_CurrentExecutedTask->TaskOSConfig.Task_currentPSP) ) );


	// 2. Context RESTORE of the next task.
	__asm volatile( "MOV R4, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R5, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R6, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R7, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R8, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R9, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R10, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__asm volatile( "MOV R11, %0" : : "r" ( *(anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP) ) );
	anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP++;

	__set_PSP( (uint32_t)anaRTOS.OS_NextExecutedTask->TaskOSConfig.Task_currentPSP );



	if (anaRTOS.OS_NextExecutedTask != NULL) 	// To avoid hardware fault from illegal load of  EXC_RETURN
	{
		// 3. Context Switch.
		anaRTOS.OS_CurrentExecutedTask = anaRTOS.OS_NextExecutedTask;
		anaRTOS.OS_NextExecutedTask = NULL;
	}

	// 4. Exception return using LR (PendSV interrupt current task so the LR still save the EXC RETURN "return to Thread with PSP").
	__asm volatile("BX LR");
}

uint32_t SysTickLED;
void SysTick_Handler(void)
{
	SysTickLED ^= 1;

	// 1. Update the Tasks holding remaining ticks.
	anaRTOS_UpdateTaskHoldingTime();

	// 1. Update the next executed task.
	anaRTOS_UpdateNextExecutedTask();

	// 2. Trigger PendSV to context switch (SAVE current task context and RESTORE the next task context).
	if (anaRTOS.OS_NextExecutedTask != anaRTOS.OS_CurrentExecutedTask)
	{
		Trigger_PendSV;
	}
}

