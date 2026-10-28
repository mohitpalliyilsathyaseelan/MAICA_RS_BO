/*
 * gpio.c -- GPIO wrapper module for LED control
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 *
 * PB02 (LED0) active-low: HIGH = LED OFF, LOW = LED ON.
 * Uses MCC-generated GPIO_PB02_* macros (GROUP[1], bit 2).
 */
#include "gpio.h"

static bool s_initialized = false;

void GPIO_Initialize(void)
{
    /* Drive PB02 HIGH -- LED OFF at startup (active-low) */
    GPIO_PB02_Set();
    s_initialized = true;
}

void GPIO_LED_Off(void)
{
    /* PB02 active-low: drive HIGH to turn LED off */
    GPIO_PB02_Set();
}

void GPIO_LED_On(void)
{
    /* PB02 active-low: drive LOW to turn LED on */
    GPIO_PB02_Clear();
}

void GPIO_LED_Toggle(void)
{
    /* Toggle PB02 */
    GPIO_PB02_Toggle();
}

void GPIO_Tasks(void)
{
    /* Idle stub -- no polling needed */
    if (!s_initialized) { return; }
}