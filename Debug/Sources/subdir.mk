################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/DAC.c \
../Sources/Debounce.c \
../Sources/Display.c \
../Sources/FIFO.c \
../Sources/Interface.c \
../Sources/Math.c \
../Sources/MyPacket.c \
../Sources/MyRTC.c \
../Sources/PIT.c \
../Sources/Protocol.c \
../Sources/SampleQueue.c \
../Sources/Tariff.c \
../Sources/UART.c \
../Sources/main.c \
../Sources/meter.c 

OBJS += \
./Sources/DAC.o \
./Sources/Debounce.o \
./Sources/Display.o \
./Sources/FIFO.o \
./Sources/Interface.o \
./Sources/Math.o \
./Sources/MyPacket.o \
./Sources/MyRTC.o \
./Sources/PIT.o \
./Sources/Protocol.o \
./Sources/SampleQueue.o \
./Sources/Tariff.o \
./Sources/UART.o \
./Sources/main.o \
./Sources/meter.o 

C_DEPS += \
./Sources/DAC.d \
./Sources/Debounce.d \
./Sources/Display.d \
./Sources/FIFO.d \
./Sources/Interface.d \
./Sources/Math.d \
./Sources/MyPacket.d \
./Sources/MyRTC.d \
./Sources/PIT.d \
./Sources/Protocol.d \
./Sources/SampleQueue.d \
./Sources/Tariff.d \
./Sources/UART.d \
./Sources/main.d \
./Sources/meter.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:\Users\13029285\Desktop\Project\Library" -I"C:/Users/13029285/Desktop/Project/Static_Code/IO_Map" -I"C:/Users/13029285/Desktop/Project/Sources" -I"C:/Users/13029285/Desktop/Project/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


