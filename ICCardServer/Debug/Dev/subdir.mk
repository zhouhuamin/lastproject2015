################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Dev/dev.c \
../Dev/ic_card.c \
../Dev/ic_card_phy.c \
../Dev/port.c 

OBJS += \
./Dev/dev.o \
./Dev/ic_card.o \
./Dev/ic_card_phy.o \
./Dev/port.o 

C_DEPS += \
./Dev/dev.d \
./Dev/ic_card.d \
./Dev/ic_card_phy.d \
./Dev/port.d 


# Each subdirectory must supply rules for building sources it contributes
Dev/%.o: ../Dev/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/root/zhouhm/workspace/ICCardServer/lib" -I"/root/zhouhm/workspace/ICCardServer/Common" -I"/root/zhouhm/workspace/ICCardServer/Dev" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


