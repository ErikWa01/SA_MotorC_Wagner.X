// Library-Header
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"

// Eigene Header
#include "scheduler.h"
#include "uart_com.h"
#include "motor_stat.h"
#include "commutation.h"
#include "des_speed.h"
#include "com_interface.h"
#include "global_const.h"

int tmr_count_task1;
int tmr_count_task2;
int tmr_trig_task1;
int tmr_trig_task2;

void scheduler_init()
{
    PR1 = 0xBB8;     // Aufruf des Timerinterrupts jeweils nach 200 µs (3000 * Tcy)
    IFS0bits.T1IF = 0;  // Ruecksetzen des Interruptflags des Timers
    IEC0bits.T1IE = 1;  // Aktivieren des Timer-Interrupts
    
    tmr_count_task1 = 0;     // Initialisieren der Variable auf 0
    tmr_count_task2 = 0;     // Initialisieren der Variable auf 0
    tmr_trig_task1 = 3;      // Aufruf des Task 1 nach 3 Timerdurchläufen
    tmr_trig_task2 = 50;     // Aufruf des Task 2 nach 50 Timerdurchläufen
    
    T1CONbits.TON = 1;  // Starten des Timers
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void)
{
    tmr_count_task1++;
    
    if(tmr_count_task1 == tmr_trig_task1)
    {
        // Aufruf motor_stat und commutation
        tmr_count_task1 = 0;
    }
    if(tmr_count_task2 == tmr_trig_task2)
    {
        // Aufruf des_speed
        tmr_count_task2 = 0;
    }
}