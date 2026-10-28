/*
 * main.c — Entry point
 * Target: PIC32CM6408PL10048 / PIC32CM PL10 Curiosity Nano
 *
 */
#include "definitions.h"   /* SYS_Initialize, SYS_Tasks */

int main(void)
{
    SYS_Initialize(NULL);

    while (true)
    {
        SYS_Tasks();

    }

    return 0;
}
