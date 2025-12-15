################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Ethernet_W550/httpServer/httpParser.c \
../Drivers/Ethernet_W550/httpServer/httpServer.c \
../Drivers/Ethernet_W550/httpServer/httpUtil.c 

OBJS += \
./Drivers/Ethernet_W550/httpServer/httpParser.o \
./Drivers/Ethernet_W550/httpServer/httpServer.o \
./Drivers/Ethernet_W550/httpServer/httpUtil.o 

C_DEPS += \
./Drivers/Ethernet_W550/httpServer/httpParser.d \
./Drivers/Ethernet_W550/httpServer/httpServer.d \
./Drivers/Ethernet_W550/httpServer/httpUtil.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Ethernet_W550/httpServer/%.o Drivers/Ethernet_W550/httpServer/%.su Drivers/Ethernet_W550/httpServer/%.cyclo: ../Drivers/Ethernet_W550/httpServer/%.c Drivers/Ethernet_W550/httpServer/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xC -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Ethernet_W550 -I../Drivers/Ethernet_W550/DHCP -I../Drivers/Ethernet_W550/DNS -I../Drivers/Ethernet_W550/httpServer -I../Drivers/Ethernet_W550/W5500 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Ethernet_W550-2f-httpServer

clean-Drivers-2f-Ethernet_W550-2f-httpServer:
	-$(RM) ./Drivers/Ethernet_W550/httpServer/httpParser.cyclo ./Drivers/Ethernet_W550/httpServer/httpParser.d ./Drivers/Ethernet_W550/httpServer/httpParser.o ./Drivers/Ethernet_W550/httpServer/httpParser.su ./Drivers/Ethernet_W550/httpServer/httpServer.cyclo ./Drivers/Ethernet_W550/httpServer/httpServer.d ./Drivers/Ethernet_W550/httpServer/httpServer.o ./Drivers/Ethernet_W550/httpServer/httpServer.su ./Drivers/Ethernet_W550/httpServer/httpUtil.cyclo ./Drivers/Ethernet_W550/httpServer/httpUtil.d ./Drivers/Ethernet_W550/httpServer/httpUtil.o ./Drivers/Ethernet_W550/httpServer/httpUtil.su

.PHONY: clean-Drivers-2f-Ethernet_W550-2f-httpServer

