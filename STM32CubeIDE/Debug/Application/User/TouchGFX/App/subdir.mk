################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Application/User/TouchGFX/App/APP.cpp \
../Application/User/TouchGFX/App/SDW.cpp 

C_SRCS += \
F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/TouchGFX/App/app_touchgfx.c 

C_DEPS += \
./Application/User/TouchGFX/App/app_touchgfx.d 

OBJS += \
./Application/User/TouchGFX/App/APP.o \
./Application/User/TouchGFX/App/SDW.o \
./Application/User/TouchGFX/App/app_touchgfx.o 

CPP_DEPS += \
./Application/User/TouchGFX/App/APP.d \
./Application/User/TouchGFX/App/SDW.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/TouchGFX/App/%.o Application/User/TouchGFX/App/%.su Application/User/TouchGFX/App/%.cyclo: ../Application/User/TouchGFX/App/%.cpp Application/User/TouchGFX/App/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DUSE_HAL_DRIVER -DUSE_BPP=16 -DSTM32F469xx -DDEBUG -c -I../../Core/Inc -I"F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/STM32CubeIDE/Application/User/TouchGFX/App" -I../../Drivers/CMSIS/Include -I../../TouchGFX/target -I../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../TouchGFX/target/generated -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/BSP/STM32469I-Discovery -I../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/generated/fonts/include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/generated/images/include -I../../TouchGFX/generated/texts/include -I../../TouchGFX/gui/include -I../../TouchGFX/generated/videos/include -I../../Drivers/BSP/Components/Common -Og -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/TouchGFX/App/app_touchgfx.o: F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/TouchGFX/App/app_touchgfx.c Application/User/TouchGFX/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DUSE_BPP=16 -DSTM32F469xx -DDEBUG -c -I../../Core/Inc -I"F:/TouchGFX/Projects/P1/ECG_PROJECT_F469I_DISCO/main_backup/PFA/STM32CubeIDE/Application/User/TouchGFX/App" -I../../Drivers/CMSIS/Include -I../../TouchGFX/target -I../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../TouchGFX/App -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../TouchGFX/target/generated -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../../Drivers/BSP/STM32469I-Discovery -I../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../Middlewares/ST/touchgfx/framework/include -I../../TouchGFX/generated/fonts/include -I../../TouchGFX/generated/gui_generated/include -I../../TouchGFX/generated/images/include -I../../TouchGFX/generated/texts/include -I../../TouchGFX/gui/include -I../../TouchGFX/generated/videos/include -I../../Drivers/BSP/Components/Common -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User-2f-TouchGFX-2f-App

clean-Application-2f-User-2f-TouchGFX-2f-App:
	-$(RM) ./Application/User/TouchGFX/App/APP.cyclo ./Application/User/TouchGFX/App/APP.d ./Application/User/TouchGFX/App/APP.o ./Application/User/TouchGFX/App/APP.su ./Application/User/TouchGFX/App/SDW.cyclo ./Application/User/TouchGFX/App/SDW.d ./Application/User/TouchGFX/App/SDW.o ./Application/User/TouchGFX/App/SDW.su ./Application/User/TouchGFX/App/app_touchgfx.cyclo ./Application/User/TouchGFX/App/app_touchgfx.d ./Application/User/TouchGFX/App/app_touchgfx.o ./Application/User/TouchGFX/App/app_touchgfx.su

.PHONY: clean-Application-2f-User-2f-TouchGFX-2f-App

