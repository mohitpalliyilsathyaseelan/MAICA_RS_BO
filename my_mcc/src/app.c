/*
 * app.c — Application layer
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 *
 * LED toggling is fully interrupt-driven:
 *   EIC_PIN_3 (PB03, SW0) fires on falling edge → Button_Callback → GPIO_LED_Toggle()
 * No polling loop or timer is used.
 */
#include "app.h"

/* ---- Module-private data ---- */
APP_DATA appData;

/* ---- EIC ISR Callback ---- */
static void Button_Callback(uintptr_t context)
{
    /* Toggle LED on every falling edge detected by EIC */
    GPIO_LED_Toggle();
}

/* ---- APP_Initialize ---- */
void APP_Initialize(void)
{
    /* Drive PB02 HIGH — LED OFF at startup (active-low) */
    GPIO_Initialize();

    /* Register falling-edge callback on EIC_PIN_3 (PB03 / SW0) */
    EIC_CallbackRegister(EIC_PIN_3, Button_Callback, 0);

    /* Enable EIC interrupt for channel 3 */
    EIC_InterruptEnable(EIC_PIN_3);

    appData.state = APP_STATE_IDLE;
}

/* ---- APP_Tasks — Harmony super-loop body ---- */
void APP_Tasks(void)
{
    switch (appData.state)
    {
        case APP_STATE_INIT:
        {
            /* Should not be reached after APP_Initialize(), but handle gracefully */
            APP_Initialize();
            break;
        }

        case APP_STATE_IDLE:
        {
            /* LED toggling is interrupt-driven; nothing to do here */
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            /* Idle stub — call GPIO_Tasks for any housekeeping */
            GPIO_Tasks();
            appData.state = APP_STATE_IDLE;
            break;
        }

        default:
        {
            break;
        }
    }
}
