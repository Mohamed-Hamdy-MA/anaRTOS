################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../RTOS/RTOS_CMxPorting.c \
../RTOS/RTOS_Queues.c \
../RTOS/RTOS_Scheduler.c 

OBJS += \
./RTOS/RTOS_CMxPorting.o \
./RTOS/RTOS_Queues.o \
./RTOS/RTOS_Scheduler.o 

C_DEPS += \
./RTOS/RTOS_CMxPorting.d \
./RTOS/RTOS_Queues.d \
./RTOS/RTOS_Scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
RTOS/%.o RTOS/%.su RTOS/%.cyclo: ../RTOS/%.c RTOS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C6Tx -c -I../Inc -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/Utils/Inc" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/RTOS/Inc" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/CMSIS/Device/ARMCM3/Include" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/CMSIS/Core/Include" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/HAL/Inc" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/MCU/Inc" -O0 -ffunction-sections -fdata-sections -Wall -gdwarf-2 -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-RTOS

clean-RTOS:
	-$(RM) ./RTOS/RTOS_CMxPorting.cyclo ./RTOS/RTOS_CMxPorting.d ./RTOS/RTOS_CMxPorting.o ./RTOS/RTOS_CMxPorting.su ./RTOS/RTOS_Queues.cyclo ./RTOS/RTOS_Queues.d ./RTOS/RTOS_Queues.o ./RTOS/RTOS_Queues.su ./RTOS/RTOS_Scheduler.cyclo ./RTOS/RTOS_Scheduler.d ./RTOS/RTOS_Scheduler.o ./RTOS/RTOS_Scheduler.su

.PHONY: clean-RTOS

