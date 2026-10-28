
/*
 * gpio.h — GPIO wrapper module for LED and button control
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 */
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "definitions.h"    /* MCC PLIB APIs */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Public API ---- */
void GPIO_Initialize(void);    /* Drives PB02 HIGH (LED off at startup) */
void GPIO_LED_Off(void);       /* PB02 active-low → drive HIGH */
void GPIO_LED_On(void);        /* PB02 active-low → drive LOW */
void GPIO_LED_Toggle(void);    /* Toggles PB02 */
void GPIO_Tasks(void);         /* call from APP_STATE_SERVICE_TASKS */

#ifdef __cplusplus
}
#endif
#endif /* GPIO_H */

