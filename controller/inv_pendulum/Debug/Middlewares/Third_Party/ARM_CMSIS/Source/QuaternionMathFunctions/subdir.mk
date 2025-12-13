################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.c 

OBJS += \
./Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.o 

C_DEPS += \
./Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/%.o Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/%.su Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/%.cyclo: ../Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/%.c Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/ARM_CMSIS/CMSIS/Core/Include/ -I../Middlewares/Third_Party/ARM_CMSIS/PrivateInclude/ -I../Middlewares/Third_Party/ARM_CMSIS/Include/ -I../Middlewares/Third_Party/ARM_CMSIS/Include -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/BasicMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/BayesFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/CommonTables" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/ComplexMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/ControllerFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/DistanceFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/FastMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/FilteringFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/InterpolationFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/MatrixFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/QuaternionMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/StatisticsFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/SupportFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/SVMFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/TransformFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/inv_pendulum/Source/WindowFunctions" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-ARM_CMSIS-2f-Source-2f-QuaternionMathFunctions

clean-Middlewares-2f-Third_Party-2f-ARM_CMSIS-2f-Source-2f-QuaternionMathFunctions:
	-$(RM) ./Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.cyclo ./Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.d ./Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.o ./Middlewares/Third_Party/ARM_CMSIS/Source/QuaternionMathFunctions/QuaternionMathFunctions.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-ARM_CMSIS-2f-Source-2f-QuaternionMathFunctions

