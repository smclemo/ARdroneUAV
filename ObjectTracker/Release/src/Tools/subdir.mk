################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Tools/coopertools.cpp \
../src/Tools/medianfilter.cpp \
../src/Tools/smoothingmedianfilter.cpp 

OBJS += \
./src/Tools/coopertools.o \
./src/Tools/medianfilter.o \
./src/Tools/smoothingmedianfilter.o 

CPP_DEPS += \
./src/Tools/coopertools.d \
./src/Tools/medianfilter.d \
./src/Tools/smoothingmedianfilter.d 


# Each subdirectory must supply rules for building sources it contributes
src/Tools/%.o: ../src/Tools/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


