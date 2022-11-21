/* Einfuegen benoetigter Header-Dateien */
#include "motor_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"

// Variablendeklaration
int I_motor_ADval;      // Variable zum Speichern des AD-gewandelten Stromproportionalem Wert
int I_motor_ADval_temp;
//double I_motor_double;      // Variable zum Speichern des umgerechneten tatsaechlichen Stromwertes
int *AD_Buf_ptr;            // Pointer auf Buffer, in dem ADC-Ausgangswerte gespeichert werden
int tmr_value;


/* Funktion zur Initialisierung des AD-Wandlers */
void adc_init() {

    TRISB |= 0x4;     // Festlegung des Inputs (TRISB5 ist Input)
    
//     Konfiguration des AD-Konverters zur Strommessung
    ADPCFG &= 0xFFFB;       // AN2 ist nicht digital
    ADCHSbits.CH0SA = 0x2;  // Positive Input in CH0 is AN2 (Strommesspin)
    ADCON3bits.SAMC = 0x1F;  // Auto-Sample Time = 31 Tad
    ADCON3bits.ADCS = 0x3F;  // Einstellen der Tad auf 2 µs  --> Aus 31 * 2 µs ergibt sich Gesamt Sample Zeit von 62 µs
    ADCON2bits.SMPI = 0x7;  // Ausloesen eines Interrupts nach 8 Sample/Convert Durchlaeufen
    ADCON1bits.ASAM = 1;    // Auto-Start Sampling
    ADCON1bits.SSRC = 0x7;  // Auto Convert
    IPC2bits.ADIP = 5;      // Setzen der Interruptprioritaet auf zweitniedrigste
    IFS0bits.ADIF = 0;      // Ruecksetzen des ADC-Interrupt-Flags
    IEC0bits.ADIE = 1;      // Aktivieren der ADC-Interrupts
    ADCON1bits.ADON = 1;    // Aktivierung des ADC
}


// Interrupt zur Verarbeitung des AD gewandelten Stromwertes
void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void)
{
    int i;                          // Initialisieren der Laufvariable fuer Schleife
    
    IFS0bits.ADIF = 0;              // Ruecksetzen des Interrupt-Flags
    
     AD_Buf_ptr = &ADCBUF0;         // Setzen des Buffer-Pointers auf Buffer 0
     I_motor_ADval_temp = 0;             // Ruecksetzen des Wertes zum Speichern der Summe der Bufferwerte       
    
    /* Durchlaufen von 8 Buffern und bilden des Mittelwerts */
    for(i = 0; i < 8; i++)
    {
        I_motor_ADval_temp = I_motor_ADval_temp + *AD_Buf_ptr++;  // Addieren des Buffers und erhoehen des Bufferpointers
    }
    I_motor_ADval = I_motor_ADval_temp / 8;                  // Bilden des Mittelwerts aus den addierten Werten
}


int get_I_motor_ADval()
{
    return I_motor_ADval;
}

// Berechnen des Stromes anhand dem Wert des AD-Wandlers (nicht benoetigt)
//double calc_I_from_ADval()
//{
//    I_motor_double = (I_motor_ADval - 503) * (25.0/256.0);      // Formel zur Berechnung des Stromwertes (beruecksichtigt Gain und Offset des Verstärkers, sowie Groeße des Shuntwiderstands)
//    
//    return I_motor_double;
//}