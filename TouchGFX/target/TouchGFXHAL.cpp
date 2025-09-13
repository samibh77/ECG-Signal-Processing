/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : TouchGFXHAL.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.25.0. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include <TouchGFXHAL.hpp>

/* USER CODE BEGIN TouchGFXHAL.cpp */
#include "main.h"
#include <touchgfx/hal/GPIO.hpp>
#include "../Components/otm8009a/otm8009a.h"
#include "../Components/nt35510/nt35510.h"
#include <CortexMMCUInstrumentation.hpp>
#include <KeySampler.hpp>
#include <touchgfx/hal/OSWrappers.hpp>
#include "FreeRTOS.h"
#include "task.h"

extern "C" {
    extern DSI_HandleTypeDef hdsi;
    extern LTDC_HandleTypeDef hltdc;
    extern uint8_t lcd_is_OTM8009A;
}

/* Request tear interrupt at specific scanline. */
void LCD_ReqTear();

static uint16_t* currFbBase = 0;
static bool refreshRequested = true;


using namespace touchgfx;

static CortexMMCUInstrumentation instrumentation;
static KeySampler btnctrl;

void TouchGFXHAL::initialize()
{
    // Calling parent implementation of initialize().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.
    // Please note, HAL::initialize() must be called to initialize the framework.

    TouchGFXGeneratedHAL::initialize();
    instrumentation.init();
    setMCUInstrumentation(&instrumentation);
    enableMCULoadCalculation(true);
    setButtonController(&btnctrl);
}

/**
 * Gets the frame buffer address used by the TFT controller.
 *
 * @return The address of the frame buffer currently being displayed on the TFT.
 */
uint16_t* TouchGFXHAL::getTFTFrameBuffer() const
{
    // Calling parent implementation of getTFTFrameBuffer().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    return currFbBase;
}

/**
 * Sets the frame buffer address used by the TFT controller.
 *
 * @param [in] address New frame buffer address.
 */
void TouchGFXHAL::setTFTFrameBuffer(uint16_t* address)
{
    // Calling parent implementation of setTFTFrameBuffer(uint16_t* address).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    currFbBase = address;

    TouchGFXGeneratedHAL::setTFTFrameBuffer(address);
}

/**
 * This function is called whenever the framework has performed a partial draw.
 *
 * @param rect The area of the screen that has been drawn, expressed in absolute coordinates.
 *
 * @see flushFrameBuffer().
 */
void TouchGFXHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    // Calling parent implementation of flushFrameBuffer(const touchgfx::Rect& rect).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.
    // Please note, HAL::flushFrameBuffer(const touchgfx::Rect& rect) must
    // be called to notify the touchgfx framework that flush has been performed.
    // To calculate the start address of rect,
    // use advanceFrameBufferToRect(uint8_t* fbPtr, const touchgfx::Rect& rect)
    // defined in TouchGFXGeneratedHAL.cpp

    TouchGFXGeneratedHAL::flushFrameBuffer(rect);
}

bool TouchGFXHAL::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return TouchGFXGeneratedHAL::blockCopy(dest, src, numBytes);
}

/**
 * Configures the interrupts relevant for TouchGFX. This primarily entails setting
 * the interrupt priorities for the DMA and LCD interrupts.
 */
void TouchGFXHAL::configureInterrupts()
{
    // Calling parent implementation of configureInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::configureInterrupts();
}

/**
 * Used for enabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::enableInterrupts()
{
    // Calling parent implementation of enableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableInterrupts();
}

/**
 * Used for disabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::disableInterrupts()
{
    // Calling parent implementation of disableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::disableInterrupts();
}

/**
 * Configure the LCD controller to fire interrupts at VSYNC. Called automatically
 * once TouchGFX initialization has completed.
 */
void TouchGFXHAL::enableLCDControllerInterrupt()
{
    // Calling parent implementation of enableLCDControllerInterrupt().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    LCD_ReqTear();

    TouchGFXGeneratedHAL::enableLCDControllerInterrupt();
}

bool TouchGFXHAL::beginFrame()
{
    return TouchGFXGeneratedHAL::beginFrame();
}

void TouchGFXHAL::endFrame()
{
    static bool firstFrame = true;
    if (firstFrame)
    {
        if (lcd_is_OTM8009A)
        {
            HAL_DSI_ShortWrite(&hdsi, LCD_ID, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_DISPON, 0x00);
        }
        else
        {
            HAL_DSI_ShortWrite(&hdsi, LCD_ID, DSI_DCS_SHORT_PKT_WRITE_P1, NT35510_CMD_DISPON, 0x00);
        }
        firstFrame = false;
    }

    TouchGFXGeneratedHAL::endFrame();

    if (frameBufferUpdatedThisFrame)
    {
        refreshRequested = true;
    }
}

/**
 * Turn on tearing Effect signal
 */
void LCD_ReqTear(void)
{
    // set_tear_on
    if (lcd_is_OTM8009A)
    {
        HAL_DSI_ShortWrite(&hdsi, LCD_ID, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_TEEON, OTM8009A_TEEON_TELOM_VBLANKING_INFO_ONLY);
    }
    else
    {
        HAL_DSI_ShortWrite(&hdsi, LCD_ID, DSI_DCS_SHORT_PKT_WRITE_P1, NT35510_CMD_TEEON, NT35510_TEEON_TELOM_VBLANKING_INFO_ONLY);
    }
}

void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef* hdsi)
{
    GPIO::set(GPIO::VSYNC_FREQ);
    HAL::getInstance()->vSync();
    OSWrappers::signalVSync();
    if (refreshRequested)
    {
        // Start transfer
        HAL_DSI_Refresh(hdsi);
        refreshRequested = false;
    }
    else
    {
        GPIO::clear(GPIO::VSYNC_FREQ);
    }
}

void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef* hdsi)
{
    GPIO::clear(GPIO::VSYNC_FREQ);
    // Signal to the framework that display update has finished
    HAL::getInstance()->frontPorchEntered();
}

extern "C"
{
    portBASE_TYPE IdleTaskHook(void* p)
    {
        if ((int)p) // idle task sched out
        {
            touchgfx::HAL::getInstance()->setMCUActive(true);
        }
        else // idle task sched in
        {
            touchgfx::HAL::getInstance()->setMCUActive(false);
        }
        return pdTRUE;
    }
}

/* USER CODE END TouchGFXHAL.cpp */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
