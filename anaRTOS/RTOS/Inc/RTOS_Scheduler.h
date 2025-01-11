/*
 * RTOS_Scheduler.h
 *
 *  Created on: Dec 2, 2024
 *      Author: Mohamed Hamdy
 */

#ifndef INC_RTOS_SCHEDULER_H_
#define INC_RTOS_SCHEDULER_H_

//==========================================================================================
//=========================================Includes=========================================
//==========================================================================================
#include "RTOS_CMxPorting.h"


//==========================================================================================
//==========================================Macros==========================================
//==========================================================================================


//==========================================================================================
//==================================User type definitions===================================
//==========================================================================================
typedef struct
{
	uint32_t Task_StackSize;
	uint32_t Task_InputPriority;
	void(*Task_ProgramEntry)(void);
	char Task_Name[30];
} RTOS_TaskUserConfig_t;
typedef struct
{
	uint32_t Task_startPSP;
	uint32_t Task_endPSP;
	uint32_t* Task_currentPSP;
	uint32_t Task_Priority;
	enum
	{
		Task_Suspend,
		Task_Waiting,
		Task_Ready,
		Task_Running
	} Task_State;
	struct
	{
		enum
		{
			TimingHold_Disabled,
			TimingHold_Enabled,
		} TimingHold_Status;
		uint32_t TimingHold_RemainingTicks;
	} Task_TimingHold;
	struct
	{
		struct RTOS_SemaphoreConfig_t* SemaphoreRequest_Semaphore;
		enum
		{
			SemaphoreRequest_NoRequest,
			SemaphoreRequest_Acquired,
			SemaphoreRequest_WaitingRelease,
		} SemaphoreRequest_Status;

	} Task_SemaphoreRequest_t;

} RTOS_TaskOSConfig_t;
typedef struct
{
	RTOS_TaskUserConfig_t TaskUserConfig;
	RTOS_TaskOSConfig_t TaskOSConfig;
} RTOS_TaskConfig_t;

typedef struct
{
	int32_t Semaphore_Value;
	char Semaphore_Name[30];
} RTOS_SemaphoreUserConfig_t;
typedef struct
{
	RTOS_TaskConfig_t* Semaphore_CurrentUser;
	RTOS_TaskConfig_t* Semaphore_NextUser;
	uint32_t Semaphore_NoOfWaitingToReleaseTasks;
} RTOS_SemaphoreOSConfig_t;
typedef struct
{
	RTOS_SemaphoreUserConfig_t SemaphoreUserConfig;
	RTOS_SemaphoreOSConfig_t SemaphoreOSConfig;
} RTOS_SemaphoreConfig_t;


//==========================================================================================
//===========================================APIs===========================================
//==========================================================================================
void anaRTOS_Init(void);
void anaRTOS_CreateTask(RTOS_TaskConfig_t* TaskConfig);
void anaRTOS_ActivateTask(RTOS_TaskConfig_t* TaskConfig);
void anaRTOS_TerminateTask(RTOS_TaskConfig_t* TaskConfig);
void anaRTOS_HoldTask(RTOS_TaskConfig_t* TaskConfig, uint32_t NoOfTicks);
void anaRTOS_AcquireSemaphore(RTOS_SemaphoreConfig_t* SemaphoreConfig);
void anaRTOS_ReleaseSemaphore(RTOS_SemaphoreConfig_t* SemaphoreConfig);
void anaRTOS_StartOS(void);


#endif /* INC_RTOS_SCHEDULER_H_ */
