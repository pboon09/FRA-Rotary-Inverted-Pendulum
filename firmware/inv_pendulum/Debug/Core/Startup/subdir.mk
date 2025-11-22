################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32g474retx.s 

OBJS += \
./Core/Startup/startup_stm32g474retx.o 

S_DEPS += \
./Core/Startup/startup_stm32g474retx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/BasicMathFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/BayesFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/CommonTables" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/ComplexMathFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/ControllerFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/DistanceFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/FastMathFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/FilteringFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/InterpolationFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/MatrixFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/QuaternionMathFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/StatisticsFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/SupportFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/SVMFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/TransformFunctions" -I"/home/b/FRA-Rotary-Inverted-Pendulum/firmware/inv_pendulum/Source/WindowFunctions" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32g474retx.d ./Core/Startup/startup_stm32g474retx.o

.PHONY: clean-Core-2f-Startup

