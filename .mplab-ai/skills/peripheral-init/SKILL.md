---
name: peripheral-init
description: Step-by-step workflow for creating any self-contained peripheral module in this project.Follow every section in order. Do **not** modify MCC-generated files.
license: MIT
---
# Skill: Peripheral Module Creation — PIC32CM PL10 / MPLAB Harmony v3

## Purpose
Step-by-step workflow for creating any self-contained peripheral module in this project.
Follow every section in order. Do **not** modify MCC-generated files.

---

## 1. Project Structure Reference

```
my_mcc/
  src/
    main.c                             ← Entry point; calls SYS_Initialize() + SYS_Tasks()
    config/
      default/
        definitions.h                 ← Aggregates all PLIB headers; #include this in modules
        initialization.c              ← SYS_Initialize() — calls PORT_, CLOCK_, EIC_, NVIC_, etc.
        interrupts.c                  ← ISR vector table; EIC_InterruptHandler is wired here
        peripheral/
          clock/  plib_clock.h/.c
          eic/    plib_eic.h/.c       ← EIC_Initialize, EIC_CallbackRegister, EIC_InterruptEnable
          evsys/  plib_evsys.h/.c
          nvic/   plib_nvic.h/.c
          port/   plib_port.h/.c      ← GPIO_PBxx_Set/Clear/Toggle/Get macros + PORT_Pin* APIs
```

**Application-layer files live alongside `main.c`:**
```
my_mcc/src/
  app.h / app.c       ← Harmony super-loop state machine (APP_Initialize / APP_Tasks)
  gpio.h / gpio.c     ← GPIO wrapper module (already defined in Plan)
  <new_module>.h/.c   ← Any additional peripheral module you create
```

---

## 2. Identifying Available MCC APIs for Any Peripheral

### Step 2a — Locate the PLIB header
```
my_mcc/src/config/default/peripheral/<peripheral_name>/plib_<peripheral_name>.h
```
Examples:
| Peripheral | Header |
|---|---|
| GPIO / PORT | `peripheral/port/plib_port.h` |
| EIC | `peripheral/eic/plib_eic.h` |
| EVSYS | `peripheral/evsys/plib_evsys.h` |
| TC0/TC1/TC2 | `peripheral/tc/plib_tc0.h` *(if MCC-configured)* |
| SERCOM (UART/SPI/I2C) | `peripheral/sercom/plib_sercom0.h` *(if MCC-configured)* |
| ADC | `peripheral/adc/plib_adc0.h` *(if MCC-configured)* |

### Step 2b — Check definitions.h for what is actually compiled in
`my_mcc/src/config/default/definitions.h` lists every `#include` for active PLIBs.
**Only use APIs whose headers appear in `definitions.h`.**

### Step 2c — Currently configured peripherals (from MCC manifest)
| Peripheral | PLIB Init function | Enabled EIC channel |
|---|---|---|
| PORT | `PORT_Initialize()` | — |
| CLOCK | `CLOCK_Initialize()` | — |
| EVSYS | `EVSYS_Initialize()` | — |
| EIC | `EIC_Initialize()` | `EIC_PIN_3` (PB03) |
| NVIC | `NVIC_Initialize()` | — |

> If you add a new peripheral (e.g., TC0, SERCOM0, ADC0) via MCC, regenerate code and verify
> the new PLIB appears in `initialization.c` and `definitions.h` before writing application code.

### Step 2d — GPIO pin macros from plib_port.h
MCC generates named macros for each configured pin:
```c
GPIO_PB02_Set()        // drive HIGH
GPIO_PB02_Clear()      // drive LOW
GPIO_PB02_Toggle()     // toggle
GPIO_PB02_Get()        // read input
GPIO_PB02_PIN          // PORT_PIN enum value
```
**Do NOT call these macros directly in app.c. Route them through wrapper functions** (see gpio.h pattern below).

---

## 3. Standard Module Structure

Every module follows this two-file pattern:

### `<module>.h` — Header template
```c
/*
 * <module>.h — <one-line description>
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 */
#ifndef <MODULE>_H
#define <MODULE>_H

#include <stdint.h>
#include <stdbool.h>
#include "definitions.h"    /* MCC PLIB APIs */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Public API ---- */
void <MODULE>_Initialize(void);
void <MODULE>_Tasks(void);          /* call from APP_STATE_SERVICE_TASKS */
/* add domain-specific functions here */

#ifdef __cplusplus
}
#endif
#endif /* <MODULE>_H */
```

### `<module>.c` — Source template
```c
/*
 * <module>.c — <one-line description>
 */
#include "<module>.h"

/* Module-private state (if needed) */
static bool s_initialized = false;

void <MODULE>_Initialize(void)
{
    /* Use PLIB calls only — no direct register writes */
    /* Example: set initial pin state via MCC macro wrappers */
    s_initialized = true;
}

void <MODULE>_Tasks(void)
{
    if (!s_initialized) { return; }
    /* Polling logic here (if any) */
}
```

---

## 4. gpio.h/gpio.c — The Reference Module (already in Plan)

The GPIO module (`my_mcc/src/gpio.h` / `my_mcc/src/gpio.c`) is the canonical example:

**gpio.h**
```c
#ifndef GPIO_H
#define GPIO_H
#include "definitions.h"

void GPIO_Initialize(void);   /* Drive PB02 HIGH (LED off at startup) */
void GPIO_LED_Off(void);      /* PB02 active-low → drive HIGH */
void GPIO_LED_On(void);       /* PB02 active-low → drive LOW  */
void GPIO_LED_Toggle(void);   /* Toggle PB02 */
void GPIO_Tasks(void);        /* Idle stub — call from APP_STATE_SERVICE_TASKS */
#endif
```

**gpio.c**
```c
#include "gpio.h"

void GPIO_Initialize(void) { GPIO_PB02_Set(); }   /* LED OFF */
void GPIO_LED_Off(void)     { GPIO_PB02_Set(); }
void GPIO_LED_On(void)      { GPIO_PB02_Clear(); }
void GPIO_LED_Toggle(void)  { GPIO_PB02_Toggle(); }
void GPIO_Tasks(void)       { /* idle */ }
```

Key rules demonstrated:
- Module calls MCC-generated macros (`GPIO_PB02_Set`) — no register manipulation.
- App code only calls module API (`GPIO_LED_Toggle`) — never the raw macros.
- `*_Tasks()` exists for the super-loop even if it is a no-op.

---

## 5. Interrupt-Driven Module Pattern (EIC example)

When a peripheral fires interrupts (EIC, TC, SERCOM, ADC), register the callback inside
the module's `_Initialize()` function:

```c
/* In <module>.c */
#include "<module>.h"

static void <MODULE>_Callback(uintptr_t context)
{
    /* ISR body — keep short; no printf, no malloc */
    /* Use PLIB calls or set a volatile flag for deferred work */
}

void <MODULE>_Initialize(void)
{
    /* Configure initial state via PLIB */
    EIC_CallbackRegister(EIC_PIN_3, <MODULE>_Callback, 0);
    EIC_InterruptEnable(EIC_PIN_3);
}
```

Then call `<MODULE>_Initialize()` from `APP_Initialize()` in `app.c`.

**ISR rules (XC32 / Harmony v3):**
- No `printf` or heavy stdlib calls inside ISRs.
- No blocking loops.
- Shared data modified in ISR must be `volatile`.
- Keep ISR body as short as possible; set a flag and handle in `APP_Tasks()`.

---

## 6. Integrating a Module with app.h / app.c

### app.h changes
```c
#include "gpio.h"          /* existing */
#include "<new_module>.h"  /* ADD */

typedef enum {
    APP_STATE_INIT,
    APP_STATE_IDLE,
    APP_STATE_SERVICE_TASKS
} APP_STATES;
```

### app.c — APP_Initialize()
```c
void APP_Initialize(void)
{
    GPIO_Initialize();                    /* existing */
    <NEW_MODULE>_Initialize();            /* ADD */

    EIC_CallbackRegister(EIC_PIN_3, Button_Callback, 0);
    EIC_InterruptEnable(EIC_PIN_3);

    appData.state = APP_STATE_IDLE;
}
```

### app.c — APP_Tasks()
```c
void APP_Tasks(void)
{
    switch (appData.state)
    {
        case APP_STATE_INIT:
            APP_Initialize();
            appData.state = APP_STATE_IDLE;
            break;

        case APP_STATE_IDLE:
            /* interrupt-driven; nothing to poll */
            break;

        case APP_STATE_SERVICE_TASKS:
            GPIO_Tasks();
            <NEW_MODULE>_Tasks();   /* ADD */
            appData.state = APP_STATE_IDLE;
            break;

        default:
            break;
    }
}
```

---

## 7. Pin Configuration Workflow (when a new peripheral needs a pin)

1. **Open MCC** in MPLAB X IDE (or VS Code MCC plugin).
2. In **Pin Manager / Pin Grid View**, assign the peripheral signal to the desired pin.
3. In **Pin Settings**, configure direction, pull-up/down, and peripheral mux function.
4. **Regenerate MCC code** — this updates `plib_port.h` (new `GPIO_Pxnn_*` macros),
   `initialization.c` (updated `PORT_Initialize()`), and may add a new PLIB source file.
5. Verify the new macros appear in `plib_port.h` and the PLIB is included in `definitions.h`.
6. Write your application module using **only** the new macros (never raw register writes).

**Peripheral mux functions** (use `PORT_PinPeripheralFunctionConfig(pin, PERIPHERAL_FUNCTION_x)`
only if MCC does not handle the mux — normally MCC handles this automatically in `PORT_Initialize()`.)

---

## 8. Adding a New Source File to the Build (CMake)

MCC-generated `cmake/PL10/default/.generated/file.cmake` lists compiled sources.
**Do not edit `.generated/` files directly** — they are regenerated by MCC.

To add your own source file, use the `user.cmake` override mechanism:

**`cmake/PL10/default/user.cmake`** (create if it doesn't exist):
```cmake
# user.cmake — safe to commit; not overwritten by MCC
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../my_mcc/src/gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../my_mcc/src/app.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../my_mcc/src/<new_module>.c"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../my_mcc/src"
)
```

---

## 9. Architectural Rules Checklist

Before writing any application-layer code, confirm:

- [ ] **MCC APIs only** — use PLIB functions and MCC-generated macros; never write to peripheral registers directly (e.g., `PORT->Group[x].OUT.reg`).
- [ ] **XC32 compiler compatibility** — do not use GCC-only attributes (`__attribute__((constructor))`, etc.) unless confirmed XC32-compatible.
- [ ] **No register-level code** in `app.c`, `app.h`, or any module under `my_mcc/src/`.
- [ ] **Wrapper API** — raw `GPIO_PBxx_*` macros must only appear inside `gpio.c`; never called from `app.c` or other modules.
- [ ] **No printf in ISRs** — use flags or ring buffers; process in `APP_Tasks()`.
- [ ] **MCC files untouched** — never edit `config/default/` files (PLIB sources, `initialization.c`, `interrupts.c`, `definitions.h`). Changes will be lost on next MCC regeneration.
- [ ] **`definitions.h` is the single include** — include `"definitions.h"` in your module headers/sources to pull in all PLIB APIs; do not include individual PLIB headers directly.

---

## 10. Quick Reference — Available PLIB APIs (currently configured)

### PORT / GPIO
```c
void    PORT_Initialize(void);
void    PORT_PinWrite(PORT_PIN pin, bool value);
bool    PORT_PinRead(PORT_PIN pin);
void    PORT_PinToggle(PORT_PIN pin);
void    PORT_PinSet(PORT_PIN pin);
void    PORT_PinClear(PORT_PIN pin);
void    PORT_PinInputEnable(PORT_PIN pin);
void    PORT_PinOutputEnable(PORT_PIN pin);
/* Named macros: GPIO_PB02_Set/Clear/Toggle/Get, GPIO_PB03_Get, etc. */
```

### EIC
```c
void    EIC_Initialize(void);
void    EIC_InterruptEnable(EIC_PIN pin);     /* EIC_PIN_3 = PB03 */
void    EIC_InterruptDisable(EIC_PIN pin);
void    EIC_CallbackRegister(EIC_PIN pin, EIC_CALLBACK callback, uintptr_t context);
/* Callback signature: void cb(uintptr_t context); */
```

### CLOCK
```c
void    CLOCK_Initialize(void);   /* called by SYS_Initialize; do not call again */
```

### NVIC
```c
void    NVIC_Initialize(void);    /* called by SYS_Initialize; do not call again */
void    NVIC_INT_Enable(void);
void    NVIC_INT_Disable(void);
```

### PORT_PIN enum values (selected)
```c
PORT_PIN_PB02 = 34U   /* User LED (LED0)    — active-low */
PORT_PIN_PB03 = 35U   /* User Switch (SW0)  — active-low, internal pull-up */
PORT_PIN_PA02 = 2U    /* also configured in plib_port.h */
```

---

## 11. Worked Example — New Module: `uart_log`

Goal: wrap SERCOM0 in UART mode (assuming MCC has configured SERCOM0 as USART).

**Step 1** — Verify MCC configured SERCOM0; check `definitions.h` for:
```c
#include "peripheral/sercom/plib_sercom0_usart.h"
```

**Step 2** — Create `my_mcc/src/uart_log.h`:
```c
#ifndef UART_LOG_H
#define UART_LOG_H
#include "definitions.h"
void UART_LOG_Initialize(void);
void UART_LOG_Send(const char *str);
void UART_LOG_Tasks(void);
#endif
```

**Step 3** — Create `my_mcc/src/uart_log.c`:
```c
#include "uart_log.h"
#include <string.h>

void UART_LOG_Initialize(void)
{
    /* SERCOM0_USART_Initialize() is called by SYS_Initialize; nothing more needed */
}

void UART_LOG_Send(const char *str)
{
    if (str == NULL) { return; }
    SERCOM0_USART_Write((uint8_t *)str, strlen(str));
}

void UART_LOG_Tasks(void) { /* poll SERCOM tx-empty if using async mode */ }
```

**Step 4** — Include in `app.h`:
```c
#include "uart_log.h"
```

**Step 5** — Call from `APP_Initialize()`:
```c
UART_LOG_Initialize();
```

**Step 6** — Add `uart_log.c` to `user.cmake`.

**Step 7** — Build with CMake (`_build/pl10`) using XC32.
