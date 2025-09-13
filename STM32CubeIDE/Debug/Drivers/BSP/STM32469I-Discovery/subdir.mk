################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.c \
F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.c 

C_DEPS += \
./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.d \
./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.d 

OBJS += \
./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.o \
./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.o: F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.c Drivers/BSP/STM32469I-Discovery/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_BPP=16 -DSTM32F469xx -DDEBUG -c -I../../Core/Inc -I"F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/STM32CubeIDE/Application/User/TouchGFX/App" -I../../Drivers/CMSIS/Include -I../../TouchGFX/target -I../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../TouchGFX/target/generated -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/BSP/STM32469I-Discovery -I../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/generated/fonts/include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/generated/images/include -I../../TouchGFX/generated/texts/include -I../../TouchGFX/gui/include -I../../TouchGFX/generated/videos/include -I../../Drivers/BSP/Components/Common -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.o: F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.c Drivers/BSP/STM32469I-Discovery/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_BPP=16 -DSTM32F469xx -DDEBUG -c -I../../Core/Inc -I"F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/STM32CubeIDE/Application/User/TouchGFX/App" -I../../Drivers/CMSIS/Include -I../../TouchGFX/target -I../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../TouchGFX/target/generated -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/BSP/STM32469I-Discovery -I../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/generated/fonts/include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/generated/images/include -I../../TouchGFX/generated/texts/include -I../../TouchGFX/gui/include -I../../TouchGFX/generated/videos/include -I../../Drivers/BSP/Components/Common -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32469I-2d-Discovery

clean-Drivers-2f-BSP-2f-STM32469I-2d-Discovery:
	-$(RM) ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.cyclo ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.d ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.o ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_qspi.su ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.cyclo ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.d ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.o ./Drivers/BSP/STM32469I-Discovery/stm32469i_discovery_sdram.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32469I-2d-Discovery

