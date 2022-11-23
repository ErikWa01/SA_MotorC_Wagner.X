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
unsigned int drehwinkel;    // Drehwinkel in x * 0,0075 ∞
unsigned int drehwinkel_last_hall;      // Speichert Drehwinkel des letzten hall_sensor_status in x * 0,0075 ∞
int drehzahl_valid;         // Wenn die Drehzahl gueltig bestimmt werden kann 1, sonst 0
int first_call;         // Wenn Hallsensoren Interrupt nach ungueltiger Drehzahl noch nicht aufgerufen wurde 1, sonst 0
int old_hall_value;     // Speichert in Kommutierungsreihenfolge gewandelten Hallsensorstatus des letzen Interrupt
int new_hall_value;     // In Kommutierungsreihenfolge gewandelter Hallsensorstatus des aktuellen Interrupts
int hall_allocation_array[6];   // Array, welches den entsprechenden Hallsensorstatus die Zahlen 0 bis 5 in Kommutierungsreihenfolge zuweist
int hall_difference;          // Differenz zwischen den Werten des alten gewandelten Hallsensorstatus und des neuen
unsigned long timer_val;      // Variable zum Auslesen der aktuellen Timer Zaehlvariable
unsigned long timer_endval;   // Variable zum Setzen des neuen Endwerts des Timers entsprechend der Winkelgeschwindigkeit
unsigned int drehwinkel_array[6];   // Array welches fuer den jeweiligen gewandelten Hallsensorstatus den entsprechenden Wert des Drehwinkels gespeicher hat


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
    
    // hall_allocation_array fuellen
    hall_allocation_array[0] = 1;       // Hallstatus 1: In Kommutierungsreihenfolge Platz 1
    hall_allocation_array[1] = 3;       // Hallstatus 2: In Kommutierungsreihenfolge Platz 3
    hall_allocation_array[2] = 2;       // Hallstatus 3: In Kommutierungsreihenfolge Platz 2
    hall_allocation_array[3] = 5;       // Hallstatus 4: In Kommutierungsreihenfolge Platz 5
    hall_allocation_array[4] = 0;       // Hallstatus 5: In Kommutierungsreihenfolge Platz 0
    hall_allocation_array[5] = 4;       // Hallstatus 6: In Kommutierungsreihenfolge Platz 4
    
    // Drehwinkel_array fuellen
    for(i=0; i<6; i++)
    {
        drehwinkel_array[i] = 400 * i;      // 400 * 0,0075∞ = 3∞ --> Winkel entsprechen 0∞, 3∞, 6∞, etc. bis 15∞
    }
    
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
        
        old_hall_value = hall_allocation_array[read_HallSensors()-1];   // Zuweisung des Hallstatus zu Kommutierungsreihenfolge
        
        drehwinkel = drehwinkel_array[old_hall_value];          // Zuweisung des entsprechenden Drehwinkels zum Start der Drehbewegung
        drehwinkel_last_hall = drehwinkel;                      // Speichern des gerade geschriebenen Drehwinkels
        
        first_call = 0;     // Interrupt wurde einmal aufgerufen
    }else{
        timer_val = TMR2;       // Auslesend der vergangenen Zeit LSW
        timer_val += (TMR3HLD << 16);    // MSW
        TMR3HLD = 0x0000;   // Zuruecksetzen der Zaehlvariable  (MSW)
        TMR2 = 0x0000;      // des Timers                       (LSW)
        
        new_hall_value = read_HallSensors();         // Auslesen des Status der Hallsensoren
        hall_difference = new_hall_value - old_hall_value;  // Bestimmen der Differenz zwischen altem und neuem Hallsensorstatus
        
        // - Berechnen der Winkelgeschwindigkeit aus durchlaufener Winkel (40 * 0,075∞ = 3∞ --> Winkel zwischen zwei Hallsensorstatus) geteilt durch verstrichene Zeit
        // - hall_difference ist, wenn vorwaerts gedreht wird positiv, wenn Rueckwaerts gedreht wird negativ
        // - Wenn bei einem Fehler Interrupt erst nach mehreren Hallstatusaenderungen aufgerufen wird, ist hall_difference
        // die Anzahl der verstrichenen Hallsensorstatusaenderungen --> 40 * hall_difference gibt richtigen Winkel
        if(hall_difference == -5){          // Ausnahme: Wenn hall_difference -5 ist, ist Drehrichtung trotzdem positiv (Wechsel von 5 auf 0)
            omega = 40 * (FCY/timer_val);   // Berechnung der Zeit: Timer z‰hlt mit TCY --> timer_val * TCY = timer_val/FCY = vergangene Zeit --> 1/vergangene Zeit = FCY/timer_val
            
            // Aktuellen Drehwinkel aktualisieren auf Wert des Hallsensorstatus
            drehwinkel = drehwinkel_last_hall + 400;    // Hier nur ein Status durchlaufen obwohl Differenz -5
        }else if(hall_difference == 5){     // Zweite Ausnahme: Wenn hall_difference 5 ist, ist Drehrichtung trotzdem negativ (Wechsel von 0 auf 5)
            omega = -40 * (FCY/timer_val);
            
            // Aktuellen Drehwinkel aktualisieren auf Wert des Hallsensorstatus
            drehwinkel = drehwinkel_last_hall + 400;    // Hier nur ein Status durchlaufen obwohl Differenz 5
        }else{
            omega = hall_difference * 40 * (FCY/timer_val);
            
            // Aktuellen Drehwinkel aktualisieren auf Wert des Hallsensorstatus
            drehwinkel = drehwinkel_last_hall + hall_difference * 400;      // Drehwinkel ist der des letzten Hallsensorstatus + Differenz der Hallsensorstatus * Winkel zwischen zwei Hallsensorstatus
        }
        
        drehzahl_valid = 1;         // Drehzahl wurde gueltig bestimmt
        drehwinkel_last_hall = drehwinkel;      // Speichern des zuletzt geschriebenen Drehwinkels
        
        // Neuer Endwert des Timers --> Ueberlauf des Timers soll verhindern, dass durch Programm bestimmte Drehzahl der tatsaechlichen weit voraus ist und somit Kommutierung falsch laeuft
        // Auﬂerdem somit Erkennung, wenn Drehzahl 0 wird
        timer_endval = 2 * timer_val;             // Neuer Endwert des Timers wird so gesetzt, dass Timer in Ueberlauf kommt, wenn 2 mal die Zeit fuer einen Hallsensorstatuswechsel durchlaufen ist
        PR3 = (timer_endval & 0xFFFF0000) >> 16;    // Neuer Endwert in entsprechendes
        PR2 = timer_endval & 0x0000FFFF;            // Register des Timers
    }
}

/* Funktion, zur Drehwinkelbestimmung anhand der aktuellen Winkelgeschwindigkeit
 * muss alle 200 µs aufgerufen werden */
void calc_motor_position()
{
    drehwinkel += (omega*2000)/1000000;     // drehwinkel += omega * 0,075/0,0075 * 200 * 10^-6 = omega * 10 * 200/10^6 = omega * 2000/1000000
    
    // Wenn Zahl hoeher als 360∞ (entspricht 48000)
    if(drehwinkel >= 48000)
    {
        if(drehwinkel > 60000)  // Annahme: Wenn Zahl ueber 60000 hohe Zahl durch negativen Ueberlauf entstanden
        {
            // Bei negativem Ueberlauf ist Zahl: negativer Drehwinkel + hoechste Darstellbare Zahl + 1 = negativer Drehwinkel + 65536
            drehwinkel = 48000 - (65536 - drehwinkel);  // Rueckrechnung: Winkel eigentlich 360∞ minus Betrag des negativem Drehwinkels
        }else{
            drehwinkel -= 48000;  // Wenn Zahl unter 60000, aber ueber 48000 ist Winkel ueber 360∞ --> 360∞ ist neue 0∞
        }
    }
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zust√§nde der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}