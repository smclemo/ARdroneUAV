################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Navdata/navdata.c 

OBJS += \
./src/Navdata/navdata.o 

C_DEPS += \
./src/Navdata/navdata.d 


# Each subdirectory must supply rules for building sources it contributes
src/Navdata/%.o: ../src/Navdata/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Lib -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/VP_SDK -I/home/steven/workspace/ARDrone_SDK_1_8/ARDroneLib/Soft/Common -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


