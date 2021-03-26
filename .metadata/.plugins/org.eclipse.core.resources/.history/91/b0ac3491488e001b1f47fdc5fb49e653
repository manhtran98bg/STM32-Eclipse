################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ioc/actuator_io_hardware.c \
../src/ioc/ioc.c \
../src/ioc/switch.c 

OBJS += \
./src/ioc/actuator_io_hardware.o \
./src/ioc/ioc.o \
./src/ioc/switch.o 

C_DEPS += \
./src/ioc/actuator_io_hardware.d \
./src/ioc/ioc.d \
./src/ioc/switch.d 


# Each subdirectory must supply rules for building sources it contributes
src/ioc/%.o: ../src/ioc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F072 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f0-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


