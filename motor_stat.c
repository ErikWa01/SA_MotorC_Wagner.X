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
long drehwinkel;    // elektrischer Drehwinkel in x * 0,0075 °
long last_drehwinkel;   // Speichern des zuletzt geschriebenen Drehwinkels
long drehwinkel_difference;     // Speichern der Differenz des Drehwinkels (int ist zu klein fuer +-40000)
int direction;                  // 0 fuer vorwaerts, 1 fuer rueckwaerts
int drehwinkel_valid;         // Wenn der Drehwinkel gueltig bestimmt werden kann 1, sonst 0
int first_call;         // Wenn Hallsensoren Interrupt nach ungueltiger Drehzahl noch nicht aufgerufen wurde 1, sonst 0
unsigned long timer_val;      // Variable zum Auslesen der aktuellen Timer Zaehlvariable
unsigned long timer_val_array[6];   // Array fuer Tiefpass
int val_count;                      // Speichert ob wie viele Werte fuer Tiefpassberechnung bereits gespeichert wurden
unsigned long temp;           // Variable zum Voruebergehenden Speichern des MSW der Timer Zaehlvariable
unsigned long timer_endval;   // Variable zum Setzen des neuen Endwerts des Timers entsprechend der Winkelgeschwindigkeit
unsigned long time_hall_change;
long drehwinkel_array[6];   // Array welches fuer den jeweiligen gewandelten Hallsensorstatus den entsprechenden Wert des Drehwinkels gespeicher hat


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
    
    // drehwinkel_array fuellen
    drehwinkel_array[0] = 40000;       // Hallstatus 1: In Kommutierungsreihenfolge Platz 5 --> 300°
    drehwinkel_array[1] = 24000;       // Hallstatus 2: In Kommutierungsreihenfolge Platz 3 --> 180°
    drehwinkel_array[2] = 32000;       // Hallstatus 3: In Kommutierungsreihenfolge Platz 4 --> 240°
    drehwinkel_array[3] = 8000;        // Hallstatus 4: In Kommutierungsreihenfolge Platz 1 --> 60°
    drehwinkel_array[4] = 0;           // Hallstatus 5: In Kommutierungsreihenfolge Platz 0 --> 0°
    drehwinkel_array[5] = 16000;       // Hallstatus 6: In Kommutierungsreihenfolge Platz 2 --> 120°
    
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
        last_drehwinkel = drehwinkel;                                   // Speichern des alten Drehwinkelwertes
        
        first_call = 0;     // Interrupt wurde einmal aufgerufen
    }else{
        timer_val = TMR2;       // Auslesen der vergangenen Zeit LSW
        temp = TMR3HLD;         // MSW Speichern
        timer_val += (temp << 16);    // Gesamtwert berechnen
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
//        // Tiefpass --> ueber 6 Hallaenderungen --> 1 Kommutierungszyklus
//        timer_val_array[0] = timer_val;             // Speichern des zuletzt gelesenen Wertes
//        if(val_count < 6)                           // val_count ist entweder Anzahl der geschriebenen Werte oder maximal 6
//        {
//            val_count++;                            // Ein weiterer Wert wurde geschrieben
//        }else{
//            drehwinkel_valid = 1;                   // Erst wenn 6 Werte geschrieben wurden --> Also 1 Kommutierugszyklus durchlaufen ist kann Drehwinkel als gueltig bestimmt angesehen werden
//        }
//        time_hall_change = 0;                       // Ruecksetzen des Ausgangs des Tiefpass
//        for(i = 0; i < val_count; i++)
//        {
//            time_hall_change += timer_val_array[i] / val_count;     // Ausgang des Tiefpass (Zeit, zwischen zwei Hallsensorwechseln) ist gemittelter Wert ueber
//        }                                                           // die Anzahl der bisher geschriebenen Werte oder maximal 6
//        // Verschieben der Werte im Tiefpass-Array um 1 nach hinten
//        for(i = 0; i < 5; i++)
//        {
//            timer_val_array[i+1] = timer_val_array[i];
//        }
//        //Ende Tiefpass
        
        drehwinkel = drehwinkel_array[read_HallSensors() - 1];         // Bestimmung des aktuellen elektrischen Drehwinkels anhand Hallstatus
        drehwinkel_difference = drehwinkel - last_drehwinkel;          // Bestimmung der Differenz aus aktuellem und neuem
        
        // Bestimmung der Richtung
        if(drehwinkel_difference < 0)           // Wenn Differenz < 0, dann ist Richung rueckwaerts
        {
            if(drehwinkel_difference != -40000) // Ausnahme: Von 360° auf 0°
            {
                direction = 1;      // Richtung ist rueckwaerts
            }else{
                direction = 0;      // Richtung ist vorwaerts
            }
        }else{                                  // Wenn Differenz > 0, dann ist Richtung vorwaerts
            if(drehwinkel_difference != 40000)  // Ausnahme: von 0° auf 360°
            {
                direction = 0;      // Richtung ist vorwaerts
            }else{
                direction = 1;      // Richtung ist rueckwaerts
            }
        }
        
//        drehwinkel_valid = 1;                     // Drehwinkel wurde gueltig bestimmt
        last_drehwinkel = drehwinkel;             // Speichern des aktuellen Drehwinkels als alten Drehwinkelwert
        
        // Neuer Endwert des Timers --> Ueberlauf des Timers soll verhindern, dass durch Programm bestimmter Drehwinkel dem tatsaechlichen weit voraus ist und somit Kommutierung falsch laeuft
        // Außerdem somit Erkennung, wenn Drehzahl 0 wird
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
 * muss alle 200 µs aufgerufen werden --> fcalc = 5 kHz */
void calc_motor_position()
{
    if(direction == 0)
    {
        drehwinkel += ((FCY/5000) * 8000)/time_hall_change;     // drehwinkel += ((FCY/fcalc) * 8000 / x; hier: FCY = 16 MHz, fcalc = 5 kHz
    }else if(direction == 1){
        drehwinkel -= ((FCY/5000) * 8000)/time_hall_change;     // Richtung rueckwaerts --> Drehwinkel wird verringert
    }
    
    // Wenn Zahl hoeher als 360° (entspricht 48000)
    if(drehwinkel >= 48000)
    {
        drehwinkel -= 48000;  // Wenn Zahl ueber 48000 ist, Winkel ueber 360° --> 360° ist neue 0°
    }
    // Wenn Zahl negativ ist --> negativer Ueberlauf
    if(drehwinkel < 0)
    {
        drehwinkel += 48000;
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
    return (PORTB & 0x0038) >> 3;    // Lesen der ZustÃ¤nde der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}

int get_drehzahl()
{
    drehzahl = (3 * FCY * 60) / (time_hall_change * 360);
    if(direction == 1)
    {
        drehzahl = -drehzahl;   // Wenn Rueckwaerts, dann negative Drehzahl
    }
    
    return drehzahl;
}

int get_drehwinkel()
{
    return drehwinkel;
}

int drehwinkel_is_valid()
{
    return drehwinkel_valid;
}


/* Tiefpassfilter nach "Digitale rekursive Filter mit einfachen Mikrocontrollern"
 * Übergabeparameter:
 *  - neuer Eingangswert, long (hier: Timerwert)
 * Rückgabeparameter:
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