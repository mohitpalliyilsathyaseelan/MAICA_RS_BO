/*
 * main.c — Entry point
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 *
 */
#include "definitions.h"   /* SYS_Initialize, SYS_Tasks */
#include "app.h"           /* APP_Initialize, APP_Tasks */

int main(void)
{
    SYS_Initialize(NULL);

    /* PLIB-only project: SYS_Initialize() does not call the app layer and
       SYS_Tasks() is an empty macro, so the app must be driven from here. */
    APP_Initialize();      /* LED off, register + enable EIC_PIN_3 callback */

    while (true)
    {
        SYS_Tasks();
        APP_Tasks();
    }

    return 0;
}
