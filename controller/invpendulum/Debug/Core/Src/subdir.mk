################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Cytron_MDXX.c \
../Core/Src/FIR.c \
../Core/Src/MotorKalman.c \
../Core/Src/PWM.c \
../Core/Src/QEI.c \
../Core/Src/energy_swingup.c \
../Core/Src/gpio.c \
../Core/Src/iwdg.c \
../Core/Src/logger.c \
../Core/Src/lqr.c \
../Core/Src/main.c \
../Core/Src/rip_config.c \
../Core/Src/sd_spi.c \
../Core/Src/spi.c \
../Core/Src/stm32g4xx_hal_msp.c \
../Core/Src/stm32g4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32g4xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/Cytron_MDXX.o \
./Core/Src/FIR.o \
./Core/Src/MotorKalman.o \
./Core/Src/PWM.o \
./Core/Src/QEI.o \
./Core/Src/energy_swingup.o \
./Core/Src/gpio.o \
./Core/Src/iwdg.o \
./Core/Src/logger.o \
./Core/Src/lqr.o \
./Core/Src/main.o \
./Core/Src/rip_config.o \
./Core/Src/sd_spi.o \
./Core/Src/spi.o \
./Core/Src/stm32g4xx_hal_msp.o \
./Core/Src/stm32g4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32g4xx.o \
./Core/Src/tim.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/Cytron_MDXX.d \
./Core/Src/FIR.d \
./Core/Src/MotorKalman.d \
./Core/Src/PWM.d \
./Core/Src/QEI.d \
./Core/Src/energy_swingup.d \
./Core/Src/gpio.d \
./Core/Src/iwdg.d \
./Core/Src/logger.d \
./Core/Src/lqr.d \
./Core/Src/main.d \
./Core/Src/rip_config.d \
./Core/Src/sd_spi.d \
./Core/Src/spi.d \
./Core/Src/stm32g4xx_hal_msp.d \
./Core/Src/stm32g4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32g4xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/BasicMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/BayesFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/CommonTables" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/ComplexMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/ControllerFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/DistanceFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/FastMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/FilteringFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/InterpolationFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/MatrixFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/QuaternionMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/StatisticsFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/SupportFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/SVMFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/TransformFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/WindowFunctions" -I../Middlewares/Third_Party/ARM_CMSIS/CMSIS/Core/Include/ -I../Middlewares/Third_Party/ARM_CMSIS/PrivateInclude/ -I../Middlewares/Third_Party/ARM_CMSIS/Include/ -I../Middlewares/Third_Party/ARM_CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/Cytron_MDXX.cyclo ./Core/Src/Cytron_MDXX.d ./Core/Src/Cytron_MDXX.o ./Core/Src/Cytron_MDXX.su ./Core/Src/FIR.cyclo ./Core/Src/FIR.d ./Core/Src/FIR.o ./Core/Src/FIR.su ./Core/Src/MotorKalman.cyclo ./Core/Src/MotorKalman.d ./Core/Src/MotorKalman.o ./Core/Src/MotorKalman.su ./Core/Src/PWM.cyclo ./Core/Src/PWM.d ./Core/Src/PWM.o ./Core/Src/PWM.su ./Core/Src/QEI.cyclo ./Core/Src/QEI.d ./Core/Src/QEI.o ./Core/Src/QEI.su ./Core/Src/energy_swingup.cyclo ./Core/Src/energy_swingup.d ./Core/Src/energy_swingup.o ./Core/Src/energy_swingup.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/iwdg.cyclo ./Core/Src/iwdg.d ./Core/Src/iwdg.o ./Core/Src/iwdg.su ./Core/Src/logger.cyclo ./Core/Src/logger.d ./Core/Src/logger.o ./Core/Src/logger.su ./Core/Src/lqr.cyclo ./Core/Src/lqr.d ./Core/Src/lqr.o ./Core/Src/lqr.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/rip_config.cyclo ./Core/Src/rip_config.d ./Core/Src/rip_config.o ./Core/Src/rip_config.su ./Core/Src/sd_spi.cyclo ./Core/Src/sd_spi.d ./Core/Src/sd_spi.o ./Core/Src/sd_spi.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32g4xx_hal_msp.cyclo ./Core/Src/stm32g4xx_hal_msp.d ./Core/Src/stm32g4xx_hal_msp.o ./Core/Src/stm32g4xx_hal_msp.su ./Core/Src/stm32g4xx_it.cyclo ./Core/Src/stm32g4xx_it.d ./Core/Src/stm32g4xx_it.o ./Core/Src/stm32g4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32g4xx.cyclo ./Core/Src/system_stm32g4xx.d ./Core/Src/system_stm32g4xx.o ./Core/Src/system_stm32g4xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

