/* Einfuegen benoetigter Header-Dateien */
#include "des_speed.h"
#include "global_const.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"
#include "adc_module.h"


/* Deklaration von Variablen */
float des_speed = 0;            // Geschwindigkeit der Rotation
char des_richtung = 'v';              // Richtung der Rotation
char des_control_mode = 'a';
int des_volt_pos = 1;
int des_duty_val;

void calc_duty_from_AD()
{
    int ADval;                      // ausgelesener AD Wert
    
    ADval = get_control_2_ADval();  // Wert des Potentiometers auslesen
    ADval = ADval << 1;             // Mit 2 multiplizieren
    
    if(ADval < 1598)                // Positive Spannung gewünscht
    {
        des_volt_pos = 1;
        des_duty_val = 1598 - ADval;    // Tastgrad zwischen 1 und 1598 (maximalwert)
    }else if(ADval >  1800){        // Negative Spannung gewünscht
        des_volt_pos = 0;
        des_duty_val = ADval - 1800;    // Tastgrad zwischen 1 und x (Alle Werte > 1598 werden als Maximalwert interpretiert)
    }else{
        des_duty_val = 0;               // Werte zwischen 1598 und 1800 --> Mittelstellung --> Spannung 0
    }
}

// Funktion zum setzen einer neuen Drehrichtung
void set_des_dir(char desRtg)
{
    if (desRtg != des_richtung)
    {
        //Kurzeitiges Verlangsamen des Motors
        des_speed = 0;
        // Aktualisierung der Richtung
        des_richtung = desRtg;
    }
}

// Funktion zum setzen einer neuen Geschwindigkeit
void set_des_speed(float des_spd)
{
    des_speed = des_spd;
}

// Funktion zum setzen eines neuen Kontrollmodus
void set_des_control_mode(char des_mode)
{
    des_control_mode = des_mode;
}

// Funktion zum Setzen der Vorgabe, ob Spannung negativ oder positiv sein soll
void set_volt_pos(char desVoltDir)
{
    if(desVoltDir == '+')
    {
        des_volt_pos = 1;
    }else if(desVoltDir == '-'){
        des_volt_pos = 0;
    }
}

char getDesRichtung()
{
    return des_richtung;
}

int get_des_duty_val()
{
    return des_duty_val;
}

char get_des_control_mode()
{
    return des_control_mode;
}

int des_volt_is_pos()
{
    return des_volt_pos;
}