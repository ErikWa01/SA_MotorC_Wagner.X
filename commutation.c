/* Einfuegen benoetigter Header-Dateien */
#include "commutation.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"
#include "global_const.h"

/* Definition von Konstanten */
#define FPWM 20000          // Definition der Frequenz (PWM) 
#define DUTY (FCY/FPWM -1)  // Definition des Duty-Cycle (Tastgrad) der PWM

float  startspeed = 1;
/* Funktion zur Initialisierung des PWM-Signals */
void PWM_Init() {

    TRISE = 0xFEC0;     // Initialisierung des PORTE
    PORTE = 0x0100;     // Festlegung der Inputs und Outputs

    PWMCON1 = 0x00FF;   // Komplementär-Betriebsmodus für alle PWM-Kanäle - Zwei PWM-Kanäle werden einem Tastgrad-Generator zugeordnet
    PTCON = 0;          // Timer-Skalierungen - deaktiviert

    PTPER = FCY / FPWM - 1;     // Einstellen der Periodendauer der PWM
    
    /* Einstellen der Tastgrade für alle drei PWM-Module */
    PDC1 = (startspeed / 9.0) * DUTY; 
    PDC2 = (startspeed / 9.0) * DUTY;
    PDC3 = (startspeed / 9.0) * DUTY;

    /* Initialisierung des PWM-Moduls */
    PTMR = 0; // Zurücksetzen des Timers
    _PTEN = 1;// Starten des Timers
}

/* Funktion zur Ver�nderung des PWM-Signals und zur Kommutierung */
void motor_commutation(char richtung, int speed, int Hall_Status)
{
    if (richtung == 118) {

        /* Aktualisierung der Geschwindigkeit*/
        PDC1 = (speed / 9.0) * DUTY;
        PDC2 = (speed / 9.0) * DUTY;
        PDC3 = (speed / 9.0) * DUTY;
        
        // Abfrage des Hall-Status mit anschlie�?ender Motorkommutierung
        switch (Hall_Status) {
            case 1: OVDCON = 0x0210;
                break;
            case 2: OVDCON = 0x0801;
                break;
            case 3: OVDCON = 0x0810;
                break;
            case 4: OVDCON = 0x2004;
                break;
            case 5: OVDCON = 0x0204;
                break;
            case 6: OVDCON = 0x2001;
                break;
            default: OVDCON = 0x0000;
                break;
        }
    }// Falls 'richtung' den Buchstaben 'r' enthält, dann Rückwärtsbewegung...
    else if (richtung == 114) {

        PDC1 = (speed / 9.0) * DUTY;
        PDC2 = (speed / 9.0) * DUTY;
        PDC3 = (speed / 9.0) * DUTY;

        switch (Hall_Status) {
            case 6: OVDCON = 0x0210;
                break;
            case 5: OVDCON = 0x0801;
                break;
            case 4: OVDCON = 0x0810;
                break;
            case 3: OVDCON = 0x2004;
                break;
            case 2: OVDCON = 0x0204;
                break;
            case 1: OVDCON = 0x2001;
                break;
            default: OVDCON = 0x0000;
                break;
        }
    }
}