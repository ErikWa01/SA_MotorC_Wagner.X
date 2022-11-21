/* Einfuegen benoetigter Header-Dateien */
#include "motor_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"
#include "com_interface.h"
#include "global_const.h"

// Variablendeklaration
int omega;              // Winkelgeschwindigkeit in x * 0,075 ∞/s
int drehzahl;           // Drehzhal in x *  U/min
int drehwinkel;         // Drehwinkel in x * 0,075 ∞
int drehzahl_valid;     // Wenn die Drehzahl gueltig bestimmt werden kann 1, sonst 0
int first_call;         // Wenn Hallsensoren Interrupt nach ungueltiger Drehzahl noch nicht aufgerufen wurde 1, sonst 0
int old_hall_value;     // Speichert Hallsensorstatus des letzen Interrupt
int new_hall_value;     // Hallsensorstatus des aktuellen Interrupts
int hall_difference;    // Differenz zwischen den Werten des alten Hallsensorstatus und des neuen
unsigned long timer_val;      // Variable zum Auslesen des LSW der aktuellen Timer Zaehlvariable

/* Funktion zur Initialisierung der Hall-Sensoren und des AD-Wandlers */
void motor_stat_init() {
    
    // Hallsensoren
    TRISB |= 0x38;      // Festlegung des Inputs (TRISB2, TRISB3, TRISB4 sind Inputs)
    ADPCFG |= 0x38;     // RB3 - RB5 sind digital
    CNEN1 = 0xE0;       // Aktivieren der Interruptausloesung an den Eingaengen der Hall-Sensoren
    IPC3bits.CNIP = 7;  // Setzen der Interrupt-Prioritaet auf die Hoechste
    IFS0bits.CNIF = 0;  // Loeschen moeglicher ausgeloester Interrupts

    // Timer fuer Zeitmessung (nutzen des TIMER2/3 Moduls im 32-Bit-Modus)
    T2CONbits.T32 = 1;  // Timer 2/3 kombiniert im 32-Bit-Modus
    PR2 = 0xFFFF;       // LSW des Timerendwerts
    PR3 = 0xFFFF;       // MSW des Timerendwerts --> Gesamtwert 0xFFFF FFFF (Maximalwert)
    IFS0bits.T3IF = 0;  // Ruecksetzen des Interruptflags
    IEC0bits.T3IE = 1;  // Aktivieren des Interrupts durch den Timer
    
    // Variablen auf Standardwerte setzen
    omega = 0;
    drehzahl = 0;
    drehwinkel = 0;
    drehzahl_valid = 0;
    first_call = 1;
    
    IEC0bits.CNIE = 1;  // Aktivieren des Input Change Interrupts (Hall Sensoren)
    T2CONbits.TON = 1;  // Starten des Timers 2/3 fuer Zeitmessung
}

/* Erkennung einer Statusaenderung der Hall Sensoren */
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt (void)
{
    IFS0bits.CNIF = 0; // loeschen des Interrupt-Flags
    
    if(first_call)
    {
        TMR3HLD = 0x0000;   // Zuruecksetzen der Zaehlvariable  (MSW)
        TMR2 = 0x0000;      // des Timers                       (LSW)
        
        PR3 = 0x0049;       // Herabsetzen des Endwerts         (MSW)
        PR2 = 0x3E00;       // des Timers auf 300 ms            (LSW)
        
        old_hall_value = read_HallSensors();
        
        first_call = 0;     // Interrupt wurde einmal aufgerufen
    }else{
        timer_val = TMR2;       // Auslesend der vergangenen Zeit LSW
        timer_val += (TMR3HLD << 16);    // MSW
        TMR3HLD = 0x0000;   // Zuruecksetzen der Zaehlvariable  (MSW)
        TMR2 = 0x0000;      // des Timers                       (LSW)
        
        new_hall_value = read_HallSensors();         // Auslesen des Status der Hallsensoren
        hall_difference = new_hall_value - old_hall_value;  // Bestimmen der Differenz zwischen altem und neuem Hallsensorstatus
        
        // Berechnen der Winkelgeschwindigkeit aus durchlaufener Winkel (40 * 0,075∞ = 3∞) geteilt durch verstrichene Zeit
        // hall_difference ist, wenn vorwaerts gedreht wird positiv, wenn Rueckwaerts gedreht wird negativ
        // Wenn bei einem Fehler Interrupt erst nach mehreren Hallstatusaenderungen aufgerufen wird, ist hall_difference
        // die Anzahl der verstrichenen Hallsensorstatusaenderungen --> 40 * hall_difference gibt richtigen Winkel
        if(hall_difference == -5){
            omega = 40 * (FCY/timer_val);
        }else if(hall_difference == 5){
            omega = -40 * (FCY/timer_val);
        }else{
            omega = hall_difference * 40 * (FCY/timer_val);
        }
    }
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zust√§nde der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}