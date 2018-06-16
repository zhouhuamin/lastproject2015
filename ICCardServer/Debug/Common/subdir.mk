################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Common/common.c \
../Common/config.c \
../Common/main.c \
../Common/net_handle.c \
../Common/starup_handle.c 

OBJS += \
./Common/common.o \
./Common/config.o \
./Common/main.o \
./Common/net_handle.o \
./Common/starup_handle.o 

C_DEPS += \
./Common/common.d \
./Common/config.d \
./Common/main.d \
./Common/net_handle.d \
./Common/starup_handle.d 


# Each subdirectory must supply rules for building sources it contributes
Common/%.o: ../Common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/root/zhouhm/workspace/ICCardServer/lib" -I"/root/zhouhm/workspace/ICCardServer/Common" -I"/root/zhouhm/workspace/ICCardServer/Dev" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


