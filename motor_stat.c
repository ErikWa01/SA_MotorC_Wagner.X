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

// Variablendeklaration
int I_motor_ADval;      // Variable zum Speichern des AD-gewandelten Stromproportionalem Wert
double I_motor_double;      // Variable zum Speichern des umgerechneten tatsaechlichen Stromwertes
int *AD_Buf_ptr;            // Pointer auf Buffer, in dem ADC-Ausgangswerte gespeichert werden

/* Funktion zur Initialisierung der Hall-Sensoren und des AD-Wandlers */
void motor_stat_init() {

    TRISB = 0x003C;     // Festlegung des Inputs (TRISB2, TRISB3, TRISB4 und TRISB5 sind Inputs)
//    ADPCFG = 0xFFFF;    // Alle Inputs sind digital
    
//     Konfiguration des AD-Konverters zur Strommessung
    ADPCFG = 0xFFFB;        // Alle Inputs Digital, auﬂer AN2
    ADCHSbits.CH0SA = 0x2;  // Positive Input in CH0 is AN2 (Strommesspin)
    ADCON3bits.SAMC = 0x0C; // Auto-Sample Time = 12 Tad
    ADCON3bits.ADCS = 0x2;  // Einstellen der Tad auf min. 83,33 ns --> Wert ist: 93,75 ns
    ADCON2bits.SMPI = 0x7;  // Ausloesen eines Interrupts nach 8 Sample/Convert Durchlauf
    ADCON1bits.ASAM = 1;    // Auto-Start Sampling
    ADCON1bits.SSRC = 0x7;  // Auto Convert
    IPC2bits.ADIP = 5;      // Setzen der Interruptprioritaet auf zweitniedrigste
    IFS0bits.ADIF = 0;      // Ruecksetzen des ADC-Interrupt-Flags
    IEC0bits.ADIE = 1;      // Aktivieren der ADC-Interrupts
    ADCON1bits.ADON = 1;    // Aktivierung des ADC
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zust√§nde der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}

// Interrupt zur Verarbeitung des AD gewandelten Stromwertes
void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void)
{
    int i;                          // Initialisieren der Laufvariable fuer Schleife
    
    IFS0bits.ADIF = 0;              // Ruecksetzen des Interrupt-Flags
    
     AD_Buf_ptr = &ADCBUF0;         // Setzen des Buffer-Pointers auf Buffer 0
     I_motor_ADval = 0;             // Ruecksetzen des Wertes zum Speichern der Summe der Bufferwerte       
    
    /* Durchlaufen von 8 Buffern und bilden des Mittelwerts */
    for(i = 0; i < 8; i++)
    {
        I_motor_ADval = I_motor_ADval + *AD_Buf_ptr++;  // Addieren des Buffers und erhoehen des Bufferpointers
    }
    I_motor_ADval = I_motor_ADval / 8;                  // Bilden des Mittelwerts aus den addierten Werten

    //send_current(I_motor_ADval);          // Senden des gemessenen Stromwertes zum Auslesen bei Tests (ggf. Auskommentiert)
}

// Berechnen des Stromes anhand dem Wert des AD-Wandlers
double calc_I_from_ADval()
{
    I_motor_double = (I_motor_ADval - 503) * (25.0/256.0);      // Formel zur Berechnung des Stromwertes (beruecksichtigt Gain und Offset des Verst‰rkers, sowie Groeﬂe des Shuntwiderstands)
    
    return I_motor_double;
}