################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/UI/ardrone_api.c \
../src/UI/ardrone_input.c \
../src/UI/ardrone_tool_ui.c \
../src/UI/gamepad.c \
../src/UI/ui.c 

OBJS += \
./src/UI/ardrone_api.o \
./src/UI/ardrone_input.o \
./src/UI/ardrone_tool_ui.o \
./src/UI/gamepad.o \
./src/UI/ui.o 

C_DEPS += \
./src/UI/ardrone_api.d \
./src/UI/ardrone_input.d \
./src/UI/ardrone_tool_ui.d \
./src/UI/gamepad.d \
./src/UI/ui.d 


# Each subdirectory must supply rules for building sources it contributes
src/UI/%.o: ../src/UI/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Lib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Common -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


