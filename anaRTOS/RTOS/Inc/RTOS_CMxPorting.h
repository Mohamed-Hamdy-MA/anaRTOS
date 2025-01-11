/*
 * RTOS_CMxPorting.h
 *
 *  Created on: Dec 2, 2024
 *      Author: Mohamed Hamdy
 */

#ifndef INC_RTOS_CMXPORTING_H_
#define INC_RTOS_CMXPORTING_H_

//==========================================================================================
//=========================================Includes=========================================
//==========================================================================================
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "core_cm3.h"




//==========================================================================================
//==========================================Macros==========================================
//==========================================================================================
#define Switch_To_UnPrivileged		__asm volatile (	"MRS r0, CONTROL	\n"		\
														"ORR r0, r0, #0x1	\n"		\
														"MSR CONTROL, r0		"	\
														:							\
														:							\
														: "r0", "cc"	)

#define Switch_To_Privileged		__asm volatile (	"MRS r0, CONTROL			\n"		\
														"AND r0, r0, #0xFFFFFFFE	\n"		\
														"MSR CONTROL, r0				"	\
														:									\
														:									\
														: "r0", "cc"	)

#define Switch_To_PSP				__asm volatile (	"MRS r0, CONTROL	\n"		\
														"ORR r0, r0, #0x2	\n"		\
														"MSR CONTROL, r0		"	\
														:							\
														:							\
														: "r0", "cc"	)

#define Switch_To_MSP				__asm volatile (	"MRS r0, CONTROL			\n"		\
														"AND r0, r0, #0xFFFFFFFD	\n"		\
														"MSR CONTROL, r0				"	\
														:									\
														:									\
														: "r0", "cc"	)

#define Set_PSP_Address(Address)	__asm volatile (	"MOV r0, %0			\n"		\
														"MSR PSP, r0			"	\
														:							\
														:"r" (Address)				\
														: "r0", "cc"	)

#define Get_PSP_Address(Address)	__asm volatile (	"MRS R0, PSP			\n"		\
														"MOV %0, R0			"	\
														:"=r" (Address)				\
														:							\
														: "r0", "cc"	)

#define Set_MSP_Address(Address)	__asm volatile (	"MOV r0, %0			\n"		\
														"MSR MSP, r0			"	\
														:							\
														:"r" (Address)				\
														: "r0", "cc"	)

#define Get_MSP_Address(Address)	__asm volatile (	"MRS R0, MSP			\n"		\
														"MOV %0, R0			"	\
														:"=r" (Address)				\
														:							\
														: "r0", "cc"	)

#define SVCall(SVCNumber)			__asm volatile (	"SVC %0"					\
														:							\
														:"i" (SVCNumber)			\
														: 	)

#define Trigger_PendSV				SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

//==========================================================================================
//=========================================Variables========================================
//==========================================================================================
extern uint32_t _estack;
#define anaRTOS_MSPStackSize		2048U		// 2KB = 2048 bytes = 0x800


//==========================================================================================
//=====================================Exception handlers===================================
//==========================================================================================
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
__attribute ((naked)) void SVC_Handler(void);
void PendSV_Handler(void);


//==========================================================================================
//=========================================Functions========================================
//==========================================================================================
void RTOS_HW_init(void);
void RTOS_StartOSTicker(uint32_t CPUClock_MHz, uint32_t Period_us);


#endif /* INC_RTOS_CMXPORTING_H_ */
