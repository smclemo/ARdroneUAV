################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/UI/gamepad.c \
../src/UI/ui.c 

OBJS += \
./src/UI/gamepad.o \
./src/UI/ui.o 

C_DEPS += \
./src/UI/gamepad.d \
./src/UI/ui.d 


# Each subdirectory must supply rules for building sources it contributes
src/UI/%.o: ../src/UI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Lib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Common -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


