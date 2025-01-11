/*
 * RTOS_CMxPorting.c
 *
 *  Created on: Dec 2, 2024
 *      Author: Mohamed Hamdy
 */


//==========================================================================================
//=========================================Includes=========================================
//==========================================================================================
#include "RTOS_CMxPorting.h"

//==========================================================================================
//==========================================Macros==========================================
//==========================================================================================


//==========================================================================================
//=========================================Variables========================================
//==========================================================================================


//==========================================================================================
//=====================================Exception handlers===================================
//==========================================================================================
void HardFault_Handler(void)
{
	while(1);
}
void MemManage_Handler(void)
{
	while(1);
}
void BusFault_Handler(void)
{
	while(1);
}
void UsageFault_Handler(void)
{
	while(1);
}
__attribute ((naked)) void SVC_Handler(void)
{
	__asm volatile ("TST LR, 0b100		\n"
					"ITE EQ				\n"
					"MRSEQ r0, MSP		\n"
					"MRSNE r0, PSP		\n"
					"B SVC_Handler_C	"
					:
					:
					: "r0", "cc"	);
}


//==========================================================================================
//=========================================Functions========================================
//==========================================================================================
void RTOS_HW_init(void)
{
	// PendSV MUST be less or equal than priority than SysTick.
	//__NVIC_SetPriority(SysTick_IRQn, 5);
	__NVIC_SetPriority(PendSV_IRQn, 0xFF);
}
void RTOS_StartOSTicker(uint32_t CPUClock_MHz, uint32_t Period_us)
{
	//Ticks = Treq / Ttick, Clock = 1 / Ttick -----> Ticks = Clock * Treq.
	SysTick_Config(CPUClock_MHz*Period_us);
}
