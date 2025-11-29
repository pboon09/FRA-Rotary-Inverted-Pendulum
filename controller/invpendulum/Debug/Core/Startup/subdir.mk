################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
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
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/BasicMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/BayesFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/CommonTables" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/ComplexMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/ControllerFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/DistanceFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/FastMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/FilteringFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/InterpolationFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/MatrixFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/QuaternionMathFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/StatisticsFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/SupportFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/SVMFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/TransformFunctions" -I"C:/fibo/3rd year_1st semester/embedded/FRA-Rotary-Inverted-Pendulum/controller/invpendulum/Source/WindowFunctions" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32g474retx.d ./Core/Startup/startup_stm32g474retx.o

.PHONY: clean-Core-2f-Startup

