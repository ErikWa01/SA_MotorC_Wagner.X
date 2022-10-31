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
int I_motor = 0;

/* Funktion zur Initialisierung der Hall-Sensoren */
void motor_stat_init() {

    TRISB = 0x003C;     // Festlegung des Inputs (TRISB2, TRISB3, TRISB4 und TRISB5 sind Inputs)
//    ADPCFG = 0xFFFF;    // Alle Inputs sind digital
    
//     Konfiguration des AD-Konverters zur Strommessung
    ADPCFG = 0xFFFB;        // Alle Inputs Digital, auﬂer AN2
    ADCHSbits.CH0SA = 0x2;  // Positive Input in CH0 is AN2 (Strommesspin)
    ADCON3bits.SAMC = 0x1;  // Auto-Sample Time = 1 Tad
    ADCON3bits.ADCS = 0x2;  // Einstellen der Tad auf min. 83,33 ns
    ADCON2bits.SMPI = 0xF;  // Ausloesen eines Interrupts erst nach 16 Sample/Convert Durchlaeufen
    ADCON1bits.ASAM = 1;    // Auto-Start Sampling
    ADCON1bits.SSRC = 0x7;  // Auto Convert
    ADCON1bits.ADON = 1;    // Aktivierung des ADC
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zust√§nde der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}

void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void)
{
    IFS0bits.ADIF = 0;
    
    I_motor = ADCBUF0;
    //send_current(I_motor);
}