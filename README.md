<!-- Please do not change this html logo with link -->
<a href="https://www.microchip.com" rel="nofollow"><img src="images/microchip.png" alt="MCHP" width="200"/></a>

## Interrupt-Driven LED Toggle Using EIC on PIC32CM PL10

## Introduction

The PIC32CM PL10 family is built on the Arm® Cortex®-M0+ core and includes an External Interrupt Controller (EIC) that detects edges or levels on external pins and raises interrupts without CPU polling. This example uses the EIC to toggle the user LED each time the user switch is pressed on the PIC32CM PL10 Curiosity Nano. All LED activity is driven by the EIC interrupt callback. The application super-loop does no polling and uses no timers.

## Related Documentation

- [PIC32CM6408PL10048 Product Page](https://www.microchip.com/en-us/product/PIC32CM6408PL10048)
- [MPLAB® Harmony v3](https://www.microchip.com/en-us/tools-resources/configure/mplab-harmony)
- [PIC32CM PL10 Code Examples on GitHub](https://github.com/microchip-pic-avr-examples?q=PIC32CM+PL10)

## Hardware Used

- [PIC32CM PL10 Curiosity Nano (EV10P22A)](https://www.microchip.com/en-us/development-tool/EV10P22A)

## Software Used

- [MPLAB® Tools for VS Code®](https://www.microchip.com/en-us/tools-resources/develop/mplab-extensions-for-vs-code) or [MPLAB® X IDE v6.35](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) or newer
- [MPLAB® XC32 Compiler v5.10](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers) or newer
- [MPLAB® Code Configurator (MCC)](https://www.microchip.com/en-us/tools-resources/configure/mplab-code-configurator) with MPLAB Harmony v3
- [PIC32CM-PL_DFP v1.5.437](https://packs.download.microchip.com/) or newer
- [CMSIS v6.3.0](https://packs.download.microchip.com/)

## Description

The user switch SW0 on PB03 is routed to EIC channel 3 (EXTINT3). That channel detects falling edges, and the internal pull-up on PB03 is enabled. Pressing the switch pulls the pin to GND, which generates a falling edge. The EIC interrupt then invokes the application callback, and the callback toggles the user LED LED0 on PB02.

The application is a standard Harmony super-loop (`main` → `SYS_Tasks` → `APP_Tasks`). All GPIO access goes through the `gpio.c` / `gpio.h` module, a set of named wrappers over the MCC-generated PLIB macros. The application layer never calls the `GPIO_PB02_*` macros directly.

```
SW0 (PB03) --falling edge--> EIC EXTINT3 --ISR--> Button_Callback() --> GPIO_LED_Toggle() --> LED0 (PB02)
```

### Application States

| State | Behavior |
|-------|----------|
| `APP_STATE_INIT` | Runs `APP_Initialize()` and moves to `APP_STATE_IDLE` |
| `APP_STATE_IDLE` | No polling. The EIC interrupt handles LED toggling |
| `APP_STATE_SERVICE_TASKS` | Calls the `GPIO_Tasks()` housekeeping stub |

### GPIO Module API

| Function | Description |
|----------|-------------|
| `GPIO_Initialize()` | Drives PB02 high so the LED is off at startup |
| `GPIO_LED_Off()` | Drives PB02 high (the LED is active-low) |
| `GPIO_LED_On()` | Drives PB02 low (the LED is active-low) |
| `GPIO_LED_Toggle()` | Toggles PB02 |
| `GPIO_Tasks()` | Idle housekeeping stub |

## Hardware Setup

No external hardware is required. The example uses the LED and switch on the Curiosity Nano.

| Sr No. | PIC32CM6408PL10048 Pin | Signal Name | Electrical Behavior |
|:------:|:----------------------:|:-----------:|:--------------------|
| 1 | PB02 | LED0 (User LED) | Active-low. Low turns the LED on, high turns it off |
| 2 | PB03 | SW0 (User Switch) | Active-low with internal pull-up. Pressing it produces a falling edge on EIC EXTINT3 |

> **Note:** PB03 is shared with the on-board debugger signal DBG2/GPIO0. This is normal for the user switch and does not affect interrupt operation.

## Operation

1. Connect the PIC32CM PL10 Curiosity Nano to the PC with a USB cable.
2. Build the project with XC32 and program the PIC32CM6408PL10048 using the on-board PKOB4 debugger.
3. After reset, LED0 is off.
4. Press SW0. LED0 toggles on each press.

## Peripheral Configuration

The following peripherals are configured in MCC (MPLAB Harmony v3):

| Module | Configuration | Usage |
|--------|---------------|-------|
| PORT | PB02: GPIO, output, custom name `GPIO_PB02` <br> PB03: function `EIC_EXTINT3`, pull-up enabled | LED output and switch input |
| EIC | Channel 3 (EXTINT3): falling-edge detection, interrupt enabled | Switch-press detection |
| NVIC | EIC interrupt enabled | Dispatches the EIC ISR to the registered callback |

Application usage in `app.c`:

```c
GPIO_Initialize();                                    /* LED OFF at startup  */
EIC_CallbackRegister(EIC_PIN_3, Button_Callback, 0);  /* falling-edge callback */
EIC_InterruptEnable(EIC_PIN_3);                       /* enable EIC channel 3  */
```

## Conclusion

This example shows how to toggle an LED from a switch on the PIC32CM PL10 Curiosity Nano using only the EIC interrupt, with no polling and no timers. Wrapping GPIO access in a small module keeps the application layer independent of the generated pin macros.
