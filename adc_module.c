/* Einfuegen benoetigter Header-Dateien */
#include "motor_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"
#include "adc_module.h"

// Variablendeklaration
int I_motor_ADval;      // Variable zum Speichern des AD-gewandelten Stromproportionalem Wert
int I_motor_ADval_temp;
int control_2_ADval;    // Variable zum Speichern des AD-gewandelten Inputs an Controlanschluss Pin 2
int control_2_ADval_temp;   
int control_4_ADval;    // Variable zum Speichern des AD-gewandelten Inputs an Controlanschluss Pin 4
int control_4_ADval_temp;
//double I_motor_double;      // Variable zum Speichern des umgerechneten tatsaechlichen Stromwertes
int *AD_Buf_ptr;            // Pointer auf Buffer, in dem ADC-Ausgangswerte gespeichert werden
int tmr_value;
struct Filter filter_I;        // Filter-Instanz zur Filterung des Stromwertes
struct Filter filter_c2;       // Filter-Instanz zur Filterung des Control Eingangspin 2
int tmr_val;


/* Funktion zur Initialisierung des AD-Wandlers */
void adc_init() {
    
    // Initialisierung der Filterinstanzen
    filterInit(&filter_I, 6, 8110);         // Filter für Stromwert: (Filter_Instanz, k, Startwert)
    filterInit(&filter_c2, 4, 1730);         // Filter für Control Pin 2: (Filter_Instanz, k, Startwert)
    
    TRISB |= 0x44;     // Festlegung des Inputs (TRISB2 (Strom) und TRISB6 (Control-Anschluss Pin 2) sind Inputs)
    
//     Konfiguration des AD-Konverters zur Strommessung und Wandlung des Inputs von Pin 2 und von Pin 6 des Control-Anschlusses
    ADPCFG &= 0xFFBB;       // AN2 und AN6 sind nicht digital
    ADCHSbits.CH0SA = 0x6;  // Positive Input in CH0 is AN6 (Strommesspin)
    ADCHSbits.CH123SA = 0x0; // Inputs sind: CH1 - AN0; CH2 - AN1; CH3 - AN2
    ADCON3bits.SAMC = 0x1F;  // Auto-Sample Time = 31 Tad
    ADCON3bits.ADCS = 0x3F;  // Einstellen der Tad auf 1 µs  --> Aus 31 * 1 µs ergibt sich Gesamt Sample Zeit von 1 µs
    ADCON2bits.BUFM = 1;    // Bufferspeicher wird als 2 8-Word-Buffer konfiguriert
    ADCON2bits.SMPI = 0x7;  // Ausloesen eines Interrupts nach 8 Sample/Convert Durchlaeufen
    ADCON2bits.CHPS = 0x2;  // Convert Channel 0, 1, 2 und 3
    ADCON1bits.SIMSAM = 0;  // Sampling der vier Channel sequentiell
    ADCON1bits.ASAM = 1;    // Auto-Start Sampling
    ADCON1bits.SSRC = 0x7;  // Auto Convert
    IPC2bits.ADIP = 5;      // Setzen der Interruptprioritaet niedriger als Timer und Hallsensoren
    IFS0bits.ADIF = 0;      // Ruecksetzen des ADC-Interrupt-Flags
    IEC0bits.ADIE = 1;      // Aktivieren des ADC-Interrupts
    ADCON1bits.ADON = 1;    // Aktivierung des ADC
}


// Interrupt zur Verarbeitung des AD gewandelten Stromwertes
void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void)
{
    int i;                          // Initialisieren der Laufvariable fuer Schleife
    
//    T1CON = 0x8000;
    
    IFS0bits.ADIF = 0;              // Ruecksetzen des Interrupt-Flags
   
    // Pruefen, ob Speicheradressen 8 - F oder 0 - 7 des Buffers beschrieben werden
    if(ADCON2bits.BUFS)
        AD_Buf_ptr = &ADCBUF0;  // Addressen 8 - F beschrieben, auf 0 - 7 zugreifen
    else
        AD_Buf_ptr = &ADCBUF8;  // Adressen 0 - 7 beschrieben, auf 8 - F zugreifen
    
    I_motor_ADval_temp = 0;        // Ruecksetzen des Wertes zum Speichern der Summe der Bufferwerte
    control_2_ADval_temp = 0;
    control_4_ADval_temp = 0;
    
    /* Durchlaufen von je Kanal 2 Buffern */
    for(i = 0; i < 2; i++)
    {
        control_2_ADval_temp += tiefpass_adc(&filter_c2, *AD_Buf_ptr++);    // Aufruf des Tiefpassfilters mit neu ausgelesenem Wert
                                                                            // Ausgang des Filters wird zu Variable addiert
        control_4_ADval_temp = control_4_ADval_temp + *AD_Buf_ptr++;
        AD_Buf_ptr++;                                                       // Überspringen einer Speicherzelle
        I_motor_ADval_temp += tiefpass_adc(&filter_I, *AD_Buf_ptr++ << 3);  // Aufruf des Tiefpassfilters mit neu ausgelesenem Wert
                                                                            // Ausgang des Filters wird zu Variable addiert
    }
    
//    tmr_val = TMR1;
//    T1CON = 0x0000;
    
    I_motor_ADval = I_motor_ADval_temp - 8110;   // Mittelung des Stromwertes um 0
    control_2_ADval = control_2_ADval_temp;
}


int get_I_motor_ADval()
{
    return I_motor_ADval;
}

int get_control_2_ADval()
{
    return control_2_ADval;
}

int tiefpass_adc(Filter *f, int x)
{
    long y = x + f->w;
    f->w = x + y - (y>>(f->k));
    return y>>((f->k)+1);
}

void filterInit(Filter *f, int k, int x)
{
    f->k = k;
    f->w = ((long)x<<(k+1)) - x;
}