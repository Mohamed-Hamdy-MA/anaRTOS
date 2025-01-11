################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utils/Queue.c 

OBJS += \
./Utils/Queue.o 

C_DEPS += \
./Utils/Queue.d 


# Each subdirectory must supply rules for building sources it contributes
Utils/%.o Utils/%.su Utils/%.cyclo: ../Utils/%.c Utils/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C6Tx -c -I../Inc -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/Utils/Inc" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/RTOS/Inc" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/CMSIS/Device/ARMCM3/Include" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/CMSIS/Core/Include" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/HAL/Inc" -I"F:/STUDY/COURSES/Mastering Embedded System Diploma/Unit_15_RTOS/anaRTOS/MCU/Inc" -O0 -ffunction-sections -fdata-sections -Wall -gdwarf-2 -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Utils

clean-Utils:
	-$(RM) ./Utils/Queue.cyclo ./Utils/Queue.d ./Utils/Queue.o ./Utils/Queue.su

.PHONY: clean-Utils

