################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Cytron_MDXX.c \
../Core/Src/FIR.c \
../Core/Src/HC05.c \
../Core/Src/MotorKalman.c \
../Core/Src/PWM.c \
../Core/Src/QEI.c \
../Core/Src/dma.c \
../Core/Src/energy_swingup.c \
../Core/Src/fonts.c \
../Core/Src/gpio.c \
../Core/Src/iwdg.c \
../Core/Src/lcd_display.c \
../Core/Src/led_matrix.c \
../Core/Src/lqr.c \
../Core/Src/main.c \
../Core/Src/rip_config.c \
../Core/Src/sd_logger.c \
../Core/Src/sd_spi.c \
../Core/Src/spi.c \
../Core/Src/stm32g4xx_hal_msp.c \
../Core/Src/stm32g4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g4xx.c \
../Core/Src/tft.c \
../Core/Src/tim.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/Cytron_MDXX.o \
./Core/Src/FIR.o \
./Core/Src/HC05.o \
./Core/Src/MotorKalman.o \
./Core/Src/PWM.o \
./Core/Src/QEI.o \
./Core/Src/dma.o \
./Core/Src/energy_swingup.o \
./Core/Src/fonts.o \
./Core/Src/gpio.o \
./Core/Src/iwdg.o \
./Core/Src/lcd_display.o \
./Core/Src/led_matrix.o \
./Core/Src/lqr.o \
./Core/Src/main.o \
./Core/Src/rip_config.o \
./Core/Src/sd_logger.o \
./Core/Src/sd_spi.o \
./Core/Src/spi.o \
./Core/Src/stm32g4xx_hal_msp.o \
./Core/Src/stm32g4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g4xx.o \
./Core/Src/tft.o \
./Core/Src/tim.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/Cytron_MDXX.d \
./Core/Src/FIR.d \
./Core/Src/HC05.d \
./Core/Src/MotorKalman.d \
./Core/Src/PWM.d \
./Core/Src/QEI.d \
./Core/Src/dma.d \
./Core/Src/energy_swingup.d \
./Core/Src/fonts.d \
./Core/Src/gpio.d \
./Core/Src/iwdg.d \
./Core/Src/lcd_display.d \
./Core/Src/led_matrix.d \
./Core/Src/lqr.d \
./Core/Src/main.d \
./Core/Src/rip_config.d \
./Core/Src/sd_logger.d \
./Core/Src/sd_spi.d \
./Core/Src/spi.d \
./Core/Src/stm32g4xx_hal_msp.d \
./Core/Src/stm32g4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g4xx.d \
./Core/Src/tft.d \
./Core/Src/tim.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/ARM_CMSIS/CMSIS/Core/Include/ -I../Middlewares/Third_Party/ARM_CMSIS/PrivateInclude/ -I../Middlewares/Third_Party/ARM_CMSIS/Include/ -I../Middlewares/Third_Party/ARM_CMSIS/Include -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/BasicMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/BayesFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/CommonTables" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/ComplexMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/ControllerFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/DistanceFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/FastMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/FilteringFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/InterpolationFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/MatrixFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/QuaternionMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/StatisticsFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/SupportFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/SVMFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/TransformFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/WindowFunctions" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/Cytron_MDXX.cyclo ./Core/Src/Cytron_MDXX.d ./Core/Src/Cytron_MDXX.o ./Core/Src/Cytron_MDXX.su ./Core/Src/FIR.cyclo ./Core/Src/FIR.d ./Core/Src/FIR.o ./Core/Src/FIR.su ./Core/Src/HC05.cyclo ./Core/Src/HC05.d ./Core/Src/HC05.o ./Core/Src/HC05.su ./Core/Src/MotorKalman.cyclo ./Core/Src/MotorKalman.d ./Core/Src/MotorKalman.o ./Core/Src/MotorKalman.su ./Core/Src/PWM.cyclo ./Core/Src/PWM.d ./Core/Src/PWM.o ./Core/Src/PWM.su ./Core/Src/QEI.cyclo ./Core/Src/QEI.d ./Core/Src/QEI.o ./Core/Src/QEI.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/energy_swingup.cyclo ./Core/Src/energy_swingup.d ./Core/Src/energy_swingup.o ./Core/Src/energy_swingup.su ./Core/Src/fonts.cyclo ./Core/Src/fonts.d ./Core/Src/fonts.o ./Core/Src/fonts.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/iwdg.cyclo ./Core/Src/iwdg.d ./Core/Src/iwdg.o ./Core/Src/iwdg.su ./Core/Src/lcd_display.cyclo ./Core/Src/lcd_display.d ./Core/Src/lcd_display.o ./Core/Src/lcd_display.su ./Core/Src/led_matrix.cyclo ./Core/Src/led_matrix.d ./Core/Src/led_matrix.o ./Core/Src/led_matrix.su ./Core/Src/lqr.cyclo ./Core/Src/lqr.d ./Core/Src/lqr.o ./Core/Src/lqr.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/rip_config.cyclo ./Core/Src/rip_config.d ./Core/Src/rip_config.o ./Core/Src/rip_config.su ./Core/Src/sd_logger.cyclo ./Core/Src/sd_logger.d ./Core/Src/sd_logger.o ./Core/Src/sd_logger.su ./Core/Src/sd_spi.cyclo ./Core/Src/sd_spi.d ./Core/Src/sd_spi.o ./Core/Src/sd_spi.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32g4xx_hal_msp.cyclo ./Core/Src/stm32g4xx_hal_msp.d ./Core/Src/stm32g4xx_hal_msp.o ./Core/Src/stm32g4xx_hal_msp.su ./Core/Src/stm32g4xx_it.cyclo ./Core/Src/stm32g4xx_it.d ./Core/Src/stm32g4xx_it.o ./Core/Src/stm32g4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g4xx.cyclo ./Core/Src/system_stm32g4xx.d ./Core/Src/system_stm32g4xx.o ./Core/Src/system_stm32g4xx.su ./Core/Src/tft.cyclo ./Core/Src/tft.d ./Core/Src/tft.o ./Core/Src/tft.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

