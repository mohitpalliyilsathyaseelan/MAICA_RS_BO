# Current Objective: Interactive LED Toggle (EIC Interrupt-Driven)

Toggle the user LED each time the user switch is pressed on the **PIC32CM PL10 Curiosity Nano** (EV10P22A).
The switch is serviced by the **EIC (External Interrupt Controller)** peripheral — the EIC fires an interrupt
on the falling edge of PB03, and the ISR callback toggles the LED. No polling or timer is required.

GPIO operations are routed through the **\gpio.c\ / \gpio.h\** module — named wrappers over the
MCC-generated PLIB macros. Application code must **not** call \GPIO_PB02_*\ or \GPIO_PB03_*\ macros directly.

## Target & Toolchain
- **MCU:** PIC32CM6408PL10048 (Arm Cortex-M0+)
- **Board:** PIC32CM PL10 Curiosity Nano (EV10P22A)
- **Framework:** MPLAB Harmony v3 (MCC), super-loop (\main\ → \SYS_Tasks\ → \APP_Tasks\)
- **Compiler:** Microchip XC32 (do not use GCC-only attributes)
- **Build:** CMake (\_build/pl10\)

## Hardware Mapping — verified against the board User Guide (§4)
| Signal | Pin  | Configured Name | Electrical behaviour |
|--------|------|-----------------|----------------------|
| **User LED (LED0)** | PB02 | \GPIO_PB02\ | **Active-low** — drive **LOW = LED ON**, HIGH = LED OFF |
| **User Switch (SW0)** | PB03 | \GPIO_PB03\ | **Active-low** — pressed connects pin to GND → falling edge. **Internal pull-up on PB03 must be enabled.** |

> Note: PB03 is shared with the on-board debugger (DBG2 / GPIO0). This is normal for the user switch and does not affect interrupt use.

## Why EIC Is Used Here
- \plib_eic.*\ is generated under \config/default/peripheral\; \interrupts.c\ contains the EIC ISR vector.
- PB03 is muxed to EIC channel (EXTINT) — configured for **falling-edge** detection with internal pull-up.
- The EIC callback toggles the LED directly via \GPIO_LED_Toggle()\ — no polling loop needed.

## GPIO Module (\gpio.c\ / \gpio.h\)
These files live alongside \pp.c\ in \my_mcc/src/\ and are the **only** GPIO interface the application uses.

| Function | Description |
|---|---|
| \GPIO_Initialize()\ | Drives PB02 HIGH (LED off at startup) |
| \GPIO_LED_Off()\ | PB02 active-low → drive HIGH |
| \GPIO_LED_On()\ | PB02 active-low → drive LOW |
| \GPIO_LED_Toggle()\ | Toggles PB02 via \GPIO_PB02_Toggle()\ |
| \GPIO_Tasks()\ | Idle stub — call from \APP_STATE_SERVICE_TASKS\ |

## APIs to Use in app.c / app.h
- LED control: \GPIO_LED_Off()\, \GPIO_LED_On()\, \GPIO_LED_Toggle()\
- Module init: \GPIO_Initialize()\ (replaces direct \GPIO_PB02_Set()\ in \APP_Initialize()\)
- EIC registration: \EIC_CallbackRegister(EIC_PIN_x, Button_Callback, 0)\ — channel matches PB03 EXTINT line
- EIC interrupt enable: \EIC_InterruptEnable(EIC_PIN_x)\ after callback registration

## Implementation — all changes in the application layer
Files to edit (generated Harmony code stays untouched):
- \my_mcc/src/app.h\ — add \#include \
gpio.h\\, states, and app data fields
- \my_mcc/src/app.c\ — register EIC callback in \APP_Initialize()\; toggle LED from callback

### 1. Include \gpio.h\ in \pp.h\
\\\c
#include \gpio.h\   /* ADD — GPIO module for LED API */
\\\

### 2. \APP_STATES\ in \pp.h\
- States needed: \APP_STATE_INIT\, \APP_STATE_IDLE\, \APP_STATE_SERVICE_TASKS\.
- No debounce state required.

### 3. EIC Callback (defined in \pp.c\)
\\\c
static void Button_Callback(uintptr_t context)
{
    GPIO_LED_Toggle();   /* toggle on every falling edge from EIC */
}
\\\

### 4. \APP_Initialize()\
\\\c
GPIO_Initialize();                                      /* LED OFF at startup */
EIC_CallbackRegister(EIC_PIN_x, Button_Callback, 0);   /* register falling-edge callback */
EIC_InterruptEnable(EIC_PIN_x);                        /* enable EIC channel */
appData.state = APP_STATE_IDLE;
\\\
> Replace \EIC_PIN_x\ with the actual EIC channel number that PB03 is muxed to (check MCC EIC configuration).

### 5. \APP_Tasks()\ — minimal super-loop
- **\APP_STATE_INIT\** → run init, transition to \APP_STATE_IDLE\.
- **\APP_STATE_IDLE\** → nothing to poll; LED toggling is fully interrupt-driven.
- **\APP_STATE_SERVICE_TASKS\** → call \GPIO_Tasks();\ (idle stub).

## Files Touched Summary
| File | Change |
|------|--------|
| \my_mcc/src/gpio.h\ | ✓ CREATED — GPIO module header; LED API |
| \my_mcc/src/gpio.c\ | ✓ CREATED — GPIO module source; PB02 PLIB wrappers |
| \my_mcc/src/app.h\ | ✓ DONE — `#include "gpio.h"` included; updated states |
| \my_mcc/src/app.c\ | ✓ DONE — `GPIO_Initialize()`, `EIC_CallbackRegister(EIC_PIN_3, …)`, `EIC_InterruptEnable(EIC_PIN_3)` active |


## Build & Verify
1. Build via CMake (\_build/pl10\) with XC32.
2. Program the Curiosity Nano.
3. **Test:** each button press fires the EIC ISR and flips the LED (ON↔OFF); no polling occurs in the super-loop.

## Acceptance Criteria
- [x] GPIO module created and linked to build (gpio.h, gpio.c)
- [x] LED is OFF at power-up.
- [x] LED toggles on each button press via EIC interrupt callback.
- [x] No polling loop or timer used; logic is entirely interrupt-driven.
- [x] Only `gpio.h` API used in app layer — no direct `GPIO_PB02_*` macro calls in `app.c`.
- [x] Only MCC/PLIB APIs used (PORT_PinSet/Clear/Toggle); XC32-clean build confirmed.



