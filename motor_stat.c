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
int drehzahl;           // mechanische Drehzhal in U/min
int drehwinkel;         // elektrischer Drehwinkel in ca. x * 0,0055 �
int last_drehwinkel;    // Speichern des zuletzt geschriebenen Drehwinkels
int drehwinkel_difference;      // Speichern der Differenz des Drehwinkels
int direction;                  // 0 fuer vorwaerts, 1 fuer rueckwaerts
int drehwinkel_valid;           // Wenn der Drehwinkel gueltig bestimmt werden kann 1, sonst 0
int first_call;                 // Wenn Hallsensoren Interrupt nach ungueltiger Drehzahl noch nicht aufgerufen wurde 1, sonst 0
unsigned long timer_val;        // Variable zum Auslesen der aktuellen Timer Zaehlvariable
unsigned long timer_val_array[6];   // Array fuer Tiefpass
int val_count;                      // Speichert wie oft Hallsensorinterrupt bereits aufgerufen wurde
unsigned long timer_val_temp;           // Variable zum Voruebergehenden Speichern des MSW der Timer Zaehlvariable
unsigned long timer_endval;             // Variable zum Setzen des neuen Endwerts des Timers entsprechend der Winkelgeschwindigkeit
unsigned long time_hall_change;         // Variable zum Speichern des gefilterten Wertes der vergangenen Zeit zwischen zwei Hallsensoren
unsigned long time_since_hall;          // Variable zum Auslesen der Zeit, die seit einem Hallsensorinterrupt vergangen ist
unsigned long time_since_hall_temp;     // Variable zum temporaeren Speichern der MSW der time_since_hall-Variable
int hallsensor_flag;                    // Flag zur Auswertung, ob Drehwinkel zuletzt durch Hallsensorinterrupt festgelegt wurde
int drehwinkel_array[6];   // Array welches fuer den jeweiligen gewandelten Hallsensorstatus den entsprechenden Wert des Drehwinkels gespeichert hat


/* Funktion zur Initialisierung der Hall-Sensoren und des AD-Wandlers */
void motor_stat_init() {    
    // Hallsensoren
    TRISB |= 0x38;      // Festlegung des Inputs (TRISB3, TRISB4, TRISB5 sind Inputs)
    ADPCFG |= 0x38;     // RB3 - RB5 sind digital
    CNEN1 = 0xE0;       // Aktivieren der Interruptausloesung an den Eingaengen der Hall-Sensoren
    IPC3bits.CNIP = 7;  // Setzen der Interrupt-Prioritaet auf die Hoechste --> Nicht niedriger, sonst Probleme
    IFS0bits.CNIF = 0;  // Loeschen moeglicher ausgeloester Interrupts

    // Timer fuer Zeitmessung (nutzen des TIMER2/3 Moduls im 32-Bit-Modus)
    T2CONbits.T32 = 1;  // Timer 2/3 kombiniert im 32-Bit-Modus
    PR2 = 0xFFFF;       // LSW des Timerendwerts
    PR3 = 0xFFFF;       // MSW des Timerendwerts --> Gesamtwert 0xFFFF FFFF (Maximalwert)
    IPC1bits.T3IP = 6;  // Timer Interrupt-Prioritaet niedriger als Scheduler, aber hoeher als Kommunikation und AD-Wandlung
    IFS0bits.T3IF = 0;  // Ruecksetzen des Interruptflags
    IEC0bits.T3IE = 1;  // Aktivieren des Interrupts durch den Timer
    
//    // Bestimmung des Drehwinkels zu Beginn aus aktuellem Hallsensorstatus
//    drehwinkel = drehwinkel_array[read_HallSensors() - 1];      // Bestimmung nur ungefaehr --> Reicht um Motor anfahren zu lassen
    
    // Variablen auf Standardwerte setzen
    drehzahl = 0;
    drehwinkel = 0;
    drehwinkel_valid = 0;
    first_call = 1;
    val_count = 0;
    time_hall_change = 0xFFFFFFFF;
    hallsensor_flag = 0;
    
    // drehwinkel_array fuellen
    drehwinkel_array[0] = 0xD555;       // Hallstatus 1: In Kommutierungsreihenfolge Platz 5 --> 300�
    drehwinkel_array[1] = 0x8000;       // Hallstatus 2: In Kommutierungsreihenfolge Platz 3 --> 180�
    drehwinkel_array[2] = 0xAAAB;       // Hallstatus 3: In Kommutierungsreihenfolge Platz 4 --> 240�
    drehwinkel_array[3] = 0x2AAB;       // Hallstatus 4: In Kommutierungsreihenfolge Platz 1 --> 60�
    drehwinkel_array[4] = 0x0000;       // Hallstatus 5: In Kommutierungsreihenfolge Platz 0 --> 0�
    drehwinkel_array[5] = 0x5555;       // Hallstatus 6: In Kommutierungsreihenfolge Platz 2 --> 120�
    
    IEC0bits.CNIE = 1;  // Aktivieren des Input Change Interrupts (Hall Sensoren)
    T2CONbits.TON = 1;  // Starten des Timers 2/3 fuer Zeitmessung
}

/* Erkennung einer Statusaenderung der Hall Sensoren */
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt (void)
{   
    IFS0bits.CNIF = 0; // loeschen des Interrupt-Flags
    
    // Pruefen ob dies der erste Aufuruf des Interrupts ist, nachdem Drehzahl_valid = 0 gesetzt wurde
    if(first_call)
    {
        TMR3HLD = 0x0000;   // Zuruecksetzen der Zaehlvariable  (MSW)
        TMR2 = 0x0000;      // des Timers                       (LSW)
        
        PR3 = 0x0049;       // Herabsetzen des Endwerts         (MSW)
        PR2 = 0x3E00;       // des Timers auf 300 ms            (LSW)
        
        drehwinkel = drehwinkel_array[read_HallSensors() - 1];          // Zuweisung des entsprechenden Drehwinkels zum Start der Drehbewegung
        hallsensor_flag = 1;                                            // Drehwinkel zuletzt durch Interrupt bestimmt
        last_drehwinkel = drehwinkel;                                   // Speichern des alten Drehwinkelwertes
        
        first_call = 0;     // Interrupt wurde einmal aufgerufen
    }else{
        timer_val = TMR2;       // Auslesen der vergangenen Zeit LSW
        timer_val_temp = TMR3HLD;         // MSW Speichern
        timer_val += (timer_val_temp << 16);    // Gesamtwert berechnen
        TMR3HLD = 0x0000;   // Zuruecksetzen der Zaehlvariable  (MSW)
        TMR2 = 0x0000;      // des Timers                       (LSW)
        
        time_hall_change = tiefpass_timer(timer_val);       // Tiefpass filtern der Zeit
        
        // Drehwinkel erst gueltig, wenn 6 Hallsensorinterrupts, also 1 Kommutierungszyklus durchlaufen ist
        if(val_count < 6)
        {
            val_count++;
        }else{
            drehwinkel_valid = 1;
        }
        
        drehwinkel = drehwinkel_array[read_HallSensors() - 1];         // Bestimmung des aktuellen elektrischen Drehwinkels anhand Hallstatus
        hallsensor_flag = 1;                                           // Drehwinkel zuletzt durch Interrupt bestimmt
        drehwinkel_difference = drehwinkel - last_drehwinkel;          // Bestimmung der Differenz aus aktuellem und neuem
        // Bestimmung der Richtung
        direction = (drehwinkel_difference > 0) ? 0 : 1;         // Wenn Differenz > 0, dann ist Richung vorw�rts (rechtslauf, 0), sonst rueckwaerts (linkslauf, 1)
        
        last_drehwinkel = drehwinkel;               // Speichern des aktuellen Drehwinkels als alten Drehwinkelwert
        
        // Neuer Endwert des Timers --> Ueberlauf des Timers soll verhindern, dass durch Programm bestimmter Drehwinkel dem tatsaechlichen weit voraus ist und somit Kommutierung falsch laeuft
        // Au�erdem somit Erkennung, wenn Drehzahl 0 wird
        timer_endval = 2 * time_hall_change;             // Neuer Endwert des Timers wird so gesetzt, dass Timer in Ueberlauf kommt, wenn 2 mal die Zeit fuer einen Hallsensorstatuswechsel durchlaufen ist
        if(timer_endval > 1600000)
        {
            timer_endval = 1600000;                 // Begrenzung des maximalen Timerendwerts --> Wenn Drehzahl noch langsamer wird keine sinnvolle Bestimmung des Drehwinkels mehr moeglich
        }
        PR3 = (timer_endval & 0xFFFF0000) >> 16;    // Neuer Endwert in entsprechendes
        PR2 = timer_endval & 0x0000FFFF;            // Register des Timers
    }
}

/* Funktion, zur Drehwinkelbestimmung anhand der aktuellen Winkelgeschwindigkeit
 * muss alle 200 �s aufgerufen werden --> tcalc = 3200 * Tcy */
void calc_motor_position()
{
    // Pr�fen ob zwischen dem letzten und dem aktuellen Aufruf der Funktion Drehwinkel durch Hallsensorinterrupt festgelegt wurde
    if(hallsensor_flag)
    {
        // Zeit < 200 �s muss beachtet werden
        // Auslesen der Zeit, die seit dem letzten Hallsensorinterrupt vergangen ist
        time_since_hall = TMR2;                             // Auslesen des LSW der Zeit seit dem letzten Hallsensorinterrupt
        time_since_hall_temp = TMR3HLD;                     // MSW Speichern
        time_since_hall += (time_since_hall_temp << 16);    // Gesamtwert berechnen
        
        if(direction == 0)
        {
            drehwinkel += (time_since_hall * 10923)/time_hall_change;     // drehwinkel += (time_since_hall * Tcy / x * Tcy) * 60� --> time_since_hall * 10923 / x
        }else if(direction == 1){
            drehwinkel -= (time_since_hall * 10923)/time_hall_change;     // Richtung rueckwaerts --> Drehwinkel wird verringert
        }
        
        hallsensor_flag = 0;
    }else{
        // Wurde Winkel zuletzt durch zyklische Funktion bestimmt kann von 200 �s als vergangene Zeit ausgegangen werden
        if(direction == 0)
        {
            drehwinkel += 34953600L/time_hall_change;     // drehwinkel += (3200 * Tcy / x * Tcy) * 60� --> 3200 * 10923 / x
        }else if(direction == 1){
            drehwinkel -= 34953600L/time_hall_change;     // Richtung rueckwaerts --> Drehwinkel wird verringert
        }
    }
}

/* Funktion wird ausgeloest wenn Timer ueberlaeuft --> Erkennung eines Stillstands des Motors */
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt (void)
{
    IFS0bits.T3IF = 0;  // Ruecksetzen Interrupt-Flag
    
    time_hall_change = 0xFFFFFFFF;      // time_hall_change auf Maximalwert setzen --> Drehzahl = 0, Dauer bis der naechste Hallstatus sich aender "unendlich"
    drehwinkel_valid = 0; // Drehwinkel ab jetzt wieder ungueltig bis neu bestimmt wurde
    first_call = 1;     // Bei Hallsensorinterrupt muss jetzt zu naechst die erste Routine wieder durchgefuehrt werden
    val_count = 0;
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zustände der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}

// Rueckgabe aktuelle Drehzahl
int get_drehzahl()
{
    drehzahl = (3 * FCY * 60) / (time_hall_change * 360);
    if(direction == 1)
    {
        drehzahl = -drehzahl;   // Wenn Rueckwaerts, dann negative Drehzahl
    }
    
    return drehzahl;
}

// Rueckgabe aktueller elektrischer Drehwinkel
int get_drehwinkel()
{
    return drehwinkel;
}

// Rueckgabe, ob Drehwinkel gueltig bestimmt wurde
int drehwinkel_is_valid()
{
    return drehwinkel_valid;
}

// Rueckgabe aktuellen Drehrichtung
int get_direction()
{
    return direction;
}


/* Tiefpassfilter nach "Digitale rekursive Filter mit einfachen Mikrocontrollern"
 * �bergabeparameter:
 *  - neuer Eingangswert, long (hier: Timerwert)
 * R�ckgabeparameter:
 *  - gefilterter Asugangswert, long
 * 
 * Filterparameter:
 *  - k = 4
 *  - n = 2^k = 16
 * */
long tiefpass_timer(long x)
{
    static long _2nw;   // 2n * w
    long _2ny;          // 2n * y
    _2ny = x + _2nw;    // (2n*b0) * x = 1*x
    _2nw = x + _2ny - (_2ny>>4);    // _2ny>>k = _2ny/n = 2y
    return _2ny>>(5);   // return y = _2ny>>(k+1) = _2ny/2n
}