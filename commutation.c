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
#define DUTY 2 * (FCY/FPWM - 1)  // Definition des Duty-Cycle (Tastgrad) der PWM
#define VKOMW 0x0E39;          // Definition des Kommutierungswinkels bei Vorkommutierung --> Wenn 0, dann keine Vorkommutierung

int richtung;
int duty_val;
int el_drehwinkel;
int volt_pos = 1;         // Sollspannung positiv: 1, Sollspannung negativ: 0, Standard positiv

/* Funktion zur Initialisierung des PWM-Signals */
void PWM_Init() {

    TRISE = 0xFEC0;     // Initialisierung des PORTE
    PORTE = 0x0100;     // Festlegung der Inputs und Outputs

    PWMCON1 = 0x00FF;   // KomplementÃ¤r-Betriebsmodus fÃ¼r alle PWM-KanÃ¤le - Zwei PWM-KanÃ¤le werden einem Tastgrad-Generator zugeordnet
    PTCON = 0;          // Timer-Skalierungen - deaktiviert
    
    // Dead-Time konfigurieren
    DTCON1bits.DTAPS = 0;   // Timerperiodendauer ist TCY
    DTCON1bits.DTA = 0xA;   // Dead Time ist 10 * TCY = 625 ns

    PTPER = FCY / FPWM - 1;     // Einstellen der Periodendauer der PWM
    
    /* Einstellen der Tastgrade fÃ¼r alle drei PWM-Module */
    PDC1 = 0; 
    PDC2 = 0;
    PDC3 = 0;

    /* Initialisierung des PWM-Moduls */
    PTMR = 0; // ZurÃ¼cksetzen des Timers
    _PTEN = 1;// Starten des Timers
}

void motor_commutation()
{
    // Abfrage der notwendigen Daten
    duty_val = get_des_duty_val();
    richtung = get_direction();
    volt_pos = des_volt_is_pos();
    el_drehwinkel = get_drehwinkel(); 
    
    /* Wenn Drehwinkel gueltig bestimmt wurde, Kommutierung anhand Drehwinkel */
    if(drehwinkel_is_valid()){
        /* Neues Programm zur Kommutierung inklusive Rekuperation */
        // Abfrage der Drehrichtung
        if(richtung == 0)   // Vorwaerts
        {
            el_drehwinkel += VKOMW;     // Bestimmung des el_drehwinkels unter Berücksichtigung der Vorkommutierung
        }else if(richtung == 1) // Rueckwaerts
        {
            el_drehwinkel += 0x2AAB - VKOMW;    // Bestimmung des el_drehwinkels --> Kompensieren der um 1 verschobenen Zuweisung bei Rueckwaertsdrehung - Vorkommutierung
        }
        
        /* Ansteuerung je nachdem ob positive oder negative Spannung gefordert ist */
        if(volt_pos)  // Spannung soll positiv sein
        {
            // Kommutierung anhand des elektrischen Drehwinkels
            if(el_drehwinkel >= 0x0000 && el_drehwinkel < 0x2AAB)          // Drehwinkel zwischen 0° und 60°
            {
                OVDCON = 0x0304;        // Entspricht Hallsensorstatus 5
            }else if(el_drehwinkel >= 0x2AAB && el_drehwinkel < 0x5555)    // Drehwinkel zwischen 60° und 120°
            {
                OVDCON = 0x3004;        // Entspricht Hallsensorstatus 4
            }else if(el_drehwinkel >= 0x5555 && el_drehwinkel < 0x8000)   // Drehwinkel zwischen 120° und 180°
            {
                OVDCON = 0x3001;        // Entspricht Hallsensorstatus 6
            }else if(el_drehwinkel >= 0x8000 && el_drehwinkel < 0xAAAB)   // Drehwinkel zwischen 180° und 240°
            {
                OVDCON = 0x0C01;        // Entspricht Hallsensorstatus 2
            }else if(el_drehwinkel >= 0xAAAB && el_drehwinkel < 0xD555)   // Drehwinkel zwischen 240° und 300°
            {
                OVDCON = 0x0C10;        // Entspricht Hallsensorstatus 3
            }else if(el_drehwinkel >= 0xD555 && el_drehwinkel <= 0xFFFF)   // Drehwinkel zwischen 300° und 360°
            {
                OVDCON = 0x0310;        // Entspricht Hallsensorstatus 1
            }
        }
        else    // Spannung soll negativ sein
        {
            // Kommutierung anhand des elektrischen Drehwinkels
            if(el_drehwinkel >= 0x0000 && el_drehwinkel < 0x2AAB)          // Drehwinkel zwischen 0° und 60°
            {
                OVDCON = 0x0C01;        // Entspricht Hallsensorstatus 5
            }else if(el_drehwinkel >= 0x2AAB && el_drehwinkel < 0x5555)    // Drehwinkel zwischen 60° und 120°
            {
                OVDCON = 0x0C10;        // Entspricht Hallsensorstatus 4
            }else if(el_drehwinkel >= 0x5555 && el_drehwinkel < 0x8000)   // Drehwinkel zwischen 120° und 180°
            {
                OVDCON = 0x0310;        // Entspricht Hallsensorstatus 6
            }else if(el_drehwinkel >= 0x8000 && el_drehwinkel < 0xAAAB)   // Drehwinkel zwischen 180° und 240°
            {
                OVDCON = 0x0304;        // Entspricht Hallsensorstatus 2
            }else if(el_drehwinkel >= 0xAAAB && el_drehwinkel < 0xD555)   // Drehwinkel zwischen 240° und 300°
            {
                OVDCON = 0x3004;        // Entspricht Hallsensorstatus 3
            }else if(el_drehwinkel >= 0xD555 && el_drehwinkel <= 0xFFFF)   // Drehwinkel zwischen 300° und 360°
            {
                OVDCON = 0x3001;        // Entspricht Hallsensorstatus 1
            }
        }
    } // Ende Kommutierung Drehwinkel
    else    // Drehwinkel nicht gueltig bestimmt
    {
        if (volt_pos) {
            // Abfrage des Hall-Status mit anschlieÃ?ender Motorkommutierung
            switch (read_HallSensors()) {
                case 1: OVDCON = 0x0310;
                    break;
                case 2: OVDCON = 0x0C01;
                    break;
                case 3: OVDCON = 0x0C10;
                    break;
                case 4: OVDCON = 0x3004;
                    break;
                case 5: OVDCON = 0x0304;
                    break;
                case 6: OVDCON = 0x3001;
                    break;
                default: OVDCON = 0x0000;
                    break;
            }
        }else{
            switch (read_HallSensors()) {
                case 6: OVDCON = 0x0310;
                    break;
                case 5: OVDCON = 0x0C01;
                    break;
                case 4: OVDCON = 0x0C10;
                    break;
                case 3: OVDCON = 0x3004;
                    break;
                case 2: OVDCON = 0x0304;
                    break;
                case 1: OVDCON = 0x3001;
                    break;
                default: OVDCON = 0x0000;
                    break;
            }
        }
    }
    
    /* Aktualisierung der Geschwindigkeit */
    PDC1 = duty_val;
    PDC2 = duty_val;
    PDC3 = duty_val;
    
} // Ende Kommutierung
