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
    IPC0bits.T1IP = 7;      // Setzen der Interruptprioritaet auf hoechste --> Zeitdeterministik gewaehrleistet
    IFS0bits.T1IF = 0;      // Ruecksetzen des Interruptflags des Timers
    IEC0bits.T1IE = 1;      // Aktivieren des Timer-Interrupts
    
    tmr_count_task1 = 0;    // Initialisieren der Variable auf 0
    tmr_count_task2 = 0;    // Initialisieren der Variable auf 0
    tmr_count_task3 = 0;    // Initialisieren der Variable auf 0
    tmr_trig_task1 = 1;     // Aufruf des Task 1 nach 1 Timerdurchlauf
    tmr_trig_task2 = 50;    // Aufruf des Task 2 nach 50 Timerdurchlaeufen
    tmr_trig_task3 = 500;  // Aufruf des Task 3 nach 5000 Timerdurchlaeufen
    
    T1CONbits.TON = 1;      // Starten des Timers
}

// Timer Interrupt --> Wird jeweils nach 200 µs aufgerufen
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void)
{
    IFS0bits.T1IF = 0;      // Ruecksetzen des Interruptflags des Timers
    
    // Erhoehen der Task-Zaehlervariablen mit jedem Timerdurchlauf
    tmr_count_task1++;
    tmr_count_task2++;
    tmr_count_task3++;
    
    // Aufrufen des ersten Task, wenn Zaehlervariable Endwert erreicht hat
    if(tmr_count_task1 == tmr_trig_task1)
    {
        tmr_count_task1 = 0; // Ruecksetzen der Zaehlervariable
        // Aufruf motor_stat und commutation
        calc_motor_position();
        motor_commutation();
//        OVDCON = 0x0015;
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
        send_current();      // Motorstrom ueber UART senden
//        send_motor_stat();      // Drehzahl und Motorstrom ueber UART senden
//        send_drehwinkel();    // Drehwinkel ueber UART senden
    }
}