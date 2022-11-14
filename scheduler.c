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

// Variablendeklaration
int tmr_count_task1;    // Zaehlervariablen zum Zaehlen der Timerinterruptaufrufe fuer die jeweiligen Tasks
int tmr_count_task2;
int tmr_count_task3;
int tmr_trig_task1;     // Endwerte der Zaehlervariablen fuer die jeweiligen Tasks --> Wenn die Zaehlervariablen diesen Wert erreicht haben,
int tmr_trig_task2;     // wird der jeweilige Task gestartet --> Zykluszeiten der Tasks sind somit tmr_trig_taskX * Timerdauer
int tmr_trig_task3;

// Initialisierung des Timermoduls
void scheduler_init()
{
    PR1 = 0xC80;            // Aufruf des Timerinterrupts jeweils nach 200 µs (3200 * Tcy)
    IPC0bits.T1IP = 4;      // Setzen der Interruptprioritaet auf niedrigste --> Sonst keine Funktionsweise der Kommunikation/AD-Wandlung etc.
    IFS0bits.T1IF = 0;      // Ruecksetzen des Interruptflags des Timers
    IEC0bits.T1IE = 1;      // Aktivieren des Timer-Interrupts
    
    tmr_count_task1 = 0;    // Initialisieren der Variable auf 0
    tmr_count_task2 = 0;    // Initialisieren der Variable auf 0
    tmr_trig_task1 = 3;     // Aufruf des Task 1 nach 3 Timerdurchläufen
    tmr_trig_task2 = 50;    // Aufruf des Task 2 nach 50 Timerdurchläufen
    tmr_trig_task3 = 5000;
    
    T1CONbits.TON = 1;      // Starten des Timers
}

// Timer Interrupt --> Wird jeweils nach 200 µs aufgerufen
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void)
{
    // Erhoehen der Task-Zaehlervariablen mit jedem Timerdurchlauf
    tmr_count_task1++;
    tmr_count_task2++;
    tmr_count_task3++;
    
    // Aufrufen des ersten Task, wenn Zaehlervariable Endwert erreicht hat
    if(tmr_count_task1 == tmr_trig_task1)
    {
        tmr_count_task1 = 0; // Ruecksetzen der Zaehlervariable
        // Aufruf motor_stat und commutation
        motor_commutation(getDesRichtung(), getDesSpeed(), read_HallSensors());
    }
    
    // Aufrufen des ersten Task, wenn Zaehlervariable Endwert erreicht hat
    if(tmr_count_task2 == tmr_trig_task2)
    {
        tmr_count_task2 = 0; // Ruecksetzen der Zaehlervariable
        // Aufruf des_speed
    }
    
    // Aufrufen des ersten Task, wenn Zaehlervariable Endwert erreicht hat
    if(tmr_count_task3 == tmr_trig_task3)
    {
        tmr_count_task3 = 0; // Ruecksetzen der Zaehlervariable
        // Aufruf einer Wiederkehrenden Kommunikationsnachricht
        send_current();
    }
}