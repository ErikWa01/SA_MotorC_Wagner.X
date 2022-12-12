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
#include "des_speed.h"
#include "motor_stat.h"

/* Definition von Konstanten */
#define FPWM 20000          // Definition der Frequenz (PWM) 
#define DUTY 2*(FCY/FPWM - 1)  // Definition des Duty-Cycle (Tastgrad) der PWM
#define VKOMW 2667              // Definition des Kommutierungswinkels bei Vorkommutierung --> Wenn 0, dann keine Vorkommutierung

char richtung;
float speed;
unsigned int el_drehwinkel;
float startspeed = 0;

/* Funktion zur Initialisierung des PWM-Signals */
void PWM_Init() {

    TRISE = 0xFEC0;     // Initialisierung des PORTE
    PORTE = 0x0100;     // Festlegung der Inputs und Outputs

    PWMCON1 = 0x00FF;   // Komplementär-Betriebsmodus für alle PWM-Kanäle - Zwei PWM-Kanäle werden einem Tastgrad-Generator zugeordnet
    PTCON = 0;          // Timer-Skalierungen - deaktiviert
    
    // Dead-Time konfigurieren
    DTCON1bits.DTAPS = 0;   // Timerperiodendauer ist TCY
    DTCON1bits.DTA = 0xA;   // Dead Time ist 10 * TCY = 625 ns

    PTPER = FCY / FPWM - 1;     // Einstellen der Periodendauer der PWM
    
    /* Einstellen der Tastgrade für alle drei PWM-Module */
    PDC1 = (startspeed / 9.0) * DUTY; 
    PDC2 = (startspeed / 9.0) * DUTY;
    PDC3 = (startspeed / 9.0) * DUTY;

    /* Initialisierung des PWM-Moduls */
    PTMR = 0; // Zurücksetzen des Timers
    _PTEN = 1;// Starten des Timers
}

void motor_commutation()
{
    richtung = getDesRichtung();
    speed = getDesSpeed();
    el_drehwinkel = get_drehwinkel();
    
    /* Aktualisierung der Geschwindigkeit */
    PDC1 = (speed / 9.0) * DUTY;
    PDC2 = (speed / 9.0) * DUTY;
    PDC3 = (speed / 9.0) * DUTY;
    
    // Wenn Drehwinkel gueltig bestimmt wurde, Kommutierung anhand Drehwinkel
    if(drehwinkel_is_valid()){
        // Abfrage der Richtung
        if(richtung == 118)             // Richtung ist vorwaerts
        {
            el_drehwinkel += VKOMW;     // Bestimmung des el_drehwinkels unter Ber�cksichtigung der Vorkommutierung

            // Kommutierung anhand des elektrischen Drehwinkels
            if(el_drehwinkel >= 0 && el_drehwinkel < 8000)              // Drehwinkel zwischen 0� und 60�
            {
                OVDCON = 0x0204;        // Entspricht Hallsensorstatus 5
            }else if(el_drehwinkel >= 8000 && el_drehwinkel < 16000)    // Drehwinkel zwischen 60� und 120�
            {
                OVDCON = 0x2004;        // Entspricht Hallsensorstatus 4
            }else if(el_drehwinkel >= 16000 && el_drehwinkel < 24000)   // Drehwinkel zwischen 120� und 180�
            {
                OVDCON = 0x2001;        // Entspricht Hallsensorstatus 6
            }else if(el_drehwinkel >= 24000 && el_drehwinkel < 32000)   // Drehwinkel zwischen 180� und 240�
            {
                OVDCON = 0x0801;        // Entspricht Hallsensorstatus 2
            }else if(el_drehwinkel >= 32000 && el_drehwinkel < 40000)   // Drehwinkel zwischen 240� und 300�
            {
                OVDCON = 0x0810;        // Entspricht Hallsensorstatus 3
            }else if(el_drehwinkel >= 40000 && el_drehwinkel < 48000)   // Drehwinkel zwischen 300� und 360�
            {
                OVDCON = 0x0210;        // Entspricht Hallsensorstatus 1
            }else{
                OVDCON = 0x0000;        // Fehler, Drehwinkel au�erhalb Wertebereich --> Keine Ansteuerung
            }
        }
        else if(richtung == 114)    // Richtung ist rueckwaerts   
        {
            el_drehwinkel -= VKOMW;     // Bestimmung des el_drehwinkels unter Ber�cksichtigung der Vorkommutierung

            // Kommutierung anhand des elektrischen Drehwinkels
            if(el_drehwinkel > 0 && el_drehwinkel <= 8000)              // Drehwinkel zwischen 0� und 60�
            {
                OVDCON = 0x0810;        // Entspricht Hallsensorstatus 4
            }else if(el_drehwinkel > 8000 && el_drehwinkel <= 16000)    // Drehwinkel zwischen 60� und 120�
            {
                OVDCON = 0x0210;        // Entspricht Hallsensorstatus 6
            }else if(el_drehwinkel > 16000 && el_drehwinkel <= 24000)   // Drehwinkel zwischen 120� und 180�
            {
                OVDCON = 0x0204;        // Entspricht Hallsensorstatus 2
            }else if(el_drehwinkel > 24000 && el_drehwinkel <= 32000)   // Drehwinkel zwischen 180� und 240�
            {
                OVDCON = 0x2004;        // Entspricht Hallsensorstatus 3
            }else if(el_drehwinkel > 32000 && el_drehwinkel <= 40000)   // Drehwinkel zwischen 240� und 300�
            {
                OVDCON = 0x2001;        // Entspricht Hallsensorstatus 1
            }else if(el_drehwinkel > 40000 && el_drehwinkel <= 48000)   // Drehwinkel zwischen 300� und 360�
            {
                OVDCON = 0x0801;        // Entspricht Hallsensorstatus 5
            }else{
                OVDCON = 0x0000;        // Fehler, Drehwinkel au�erhalb Wertebereich --> Keine Ansteuerung
            }
//            switch (read_HallSensors()) {
//                case 6: OVDCON = 0x0210;
//                    break;
//                case 5: OVDCON = 0x0801;
//                    break;
//                case 4: OVDCON = 0x0810;
//                    break;
//                case 3: OVDCON = 0x2004;
//                    break;
//                case 2: OVDCON = 0x0204;
//                    break;
//                case 1: OVDCON = 0x2001;
//                    break;
//                default: OVDCON = 0x0000;
//                    break;
//            }
        }
        // Ende Kommutierung Drehwinkel
    }
    else
    {
        // Ansonsten Kommutierung anhand der Hallsensoren
        if (richtung == 118) {
            // Abfrage des Hall-Status mit anschlie�?ender Motorkommutierung
            switch (read_HallSensors()) {
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
            switch (read_HallSensors()) {
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
}

///* Funktion zur Ver�nderung des PWM-Signals und zur Kommutierung */
//void motor_commutation(char richtung, int speed, int Hall_Status)
//{
//    if (richtung == 118) {
//
//        /* Aktualisierung der Geschwindigkeit*/
//        PDC1 = (speed / 9.0) * DUTY;
//        PDC2 = (speed / 9.0) * DUTY;
//        PDC3 = (speed / 9.0) * DUTY;
//        
//        // Abfrage des Hall-Status mit anschlie�?ender Motorkommutierung
//        switch (Hall_Status) {
//            case 1: OVDCON = 0x0210;
//                break;
//            case 2: OVDCON = 0x0801;
//                break;
//            case 3: OVDCON = 0x0810;
//                break;
//            case 4: OVDCON = 0x2004;
//                break;
//            case 5: OVDCON = 0x0204;
//                break;
//            case 6: OVDCON = 0x2001;
//                break;
//            default: OVDCON = 0x0000;
//                break;
//        }
//    }// Falls 'richtung' den Buchstaben 'r' enthält, dann Rückwärtsbewegung...
//    else if (richtung == 114) {
//
//        PDC1 = (speed / 9.0) * DUTY;
//        PDC2 = (speed / 9.0) * DUTY;
//        PDC3 = (speed / 9.0) * DUTY;
//
//        switch (Hall_Status) {
//            case 6: OVDCON = 0x0210;
//                break;
//            case 5: OVDCON = 0x0801;
//                break;
//            case 4: OVDCON = 0x0810;
//                break;
//            case 3: OVDCON = 0x2004;
//                break;
//            case 2: OVDCON = 0x0204;
//                break;
//            case 1: OVDCON = 0x2001;
//                break;
//            default: OVDCON = 0x0000;
//                break;
//        }
//    }
//}