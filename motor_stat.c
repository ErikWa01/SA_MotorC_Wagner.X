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
int *AD_Buf_ptr;
int measure_count = 0;

/* Funktion zur Initialisierung der Hall-Sensoren */
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
    int i;
    
    IFS0bits.ADIF = 0;              // Ruecksetzen des Interrupt-Flags
    
    if(measure_count = 0)           // Pruefen ob ersten 8 der 16 moeglichen Buffer ausgelesen werden sollen
    {
        AD_Buf_ptr = &ADCBUF0;      // Wenn ja, setzen des Buffer-Pointers auf Buffer 0
        measure_count = 1;          // Kennzeichen, dass das naechste mal zweiten 8 Buffer gelesen werden sollen
    }else if(measure_count = 1){    // Pruefen ob zweiten 8 Buffer gelesen werden sollen
        AD_Buf_ptr = &ADCBUF8;      // Wenn ja, setzen des Buffer-Pointers auf Buffer 8
        measure_count = 0;          // Kennzeichen, dass naechste mal wieder die ersten 8 gelesen werden sollen
    }
    
    /* Durchlaufen von 8 Buffern und bilden des Mittelwerts */
    for(i = 0; i < 8; i++)
    {
        I_motor = I_motor + *AD_Buf_ptr++;
    }
    I_motor = I_motor / 8;
    
    send_current(I_motor);          // Senden des gemessenen Stromwertes
}