/*
 * app.h — Application layer header
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 *
 * LED toggling is fully interrupt-driven via EIC_PIN_3 (PB03).
 * No polling or timer used.
 */
#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdbool.h>
#include "definitions.h"    /* SYS_Initialize, SYS_Tasks, EIC PLIB */
#include "gpio.h"           /* GPIO_LED_Off, GPIO_LED_On, GPIO_LED_Toggle */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Application States ---- */
typedef enum
{
    APP_STATE_INIT,           /* One-time initialization */
    APP_STATE_IDLE,           /* LED toggle handled by EIC ISR; nothing to poll */
    APP_STATE_SERVICE_TASKS   /* Periodic housekeeping (stub) */
} APP_STATES;

/* ---- Application Data ---- */
typedef struct
{
    APP_STATES state;
} APP_DATA;

/* ---- Public API ---- */
void APP_Initialize(void);
void APP_Tasks(void);

#ifdef __cplusplus
}
#endif
#endif /* APP_H */
