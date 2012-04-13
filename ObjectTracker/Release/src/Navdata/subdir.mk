################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Navdata/NavDataContainer.cpp 

C_SRCS += \
../src/Navdata/navdata.c 

OBJS += \
./src/Navdata/NavDataContainer.o \
./src/Navdata/navdata.o 

C_DEPS += \
./src/Navdata/navdata.d 

CPP_DEPS += \
./src/Navdata/NavDataContainer.d 


# Each subdirectory must supply rules for building sources it contributes
src/Navdata/%.o: ../src/Navdata/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/Navdata/%.o: ../src/Navdata/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


