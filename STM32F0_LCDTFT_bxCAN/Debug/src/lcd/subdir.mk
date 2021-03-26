################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/lcd/digit.c \
../src/lcd/digit1.c \
../src/lcd/ili9486.c \
../src/lcd/lcd_io_gpio8.c \
../src/lcd/screen.c \
../src/lcd/stm32_adafruit_lcd.c 

OBJS += \
./src/lcd/digit.o \
./src/lcd/digit1.o \
./src/lcd/ili9486.o \
./src/lcd/lcd_io_gpio8.o \
./src/lcd/screen.o \
./src/lcd/stm32_adafruit_lcd.o 

C_DEPS += \
./src/lcd/digit.d \
./src/lcd/digit1.d \
./src/lcd/ili9486.d \
./src/lcd/lcd_io_gpio8.d \
./src/lcd/screen.d \
./src/lcd/stm32_adafruit_lcd.d 


# Each subdirectory must supply rules for building sources it contributes
src/lcd/%.o: ../src/lcd/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F072 -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f0-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


