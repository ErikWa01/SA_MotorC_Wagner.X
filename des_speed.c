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
char control_mode = 'a';
int des_volt_pos = 1;
int des_duty_val;
int des_volt_pos_a = 1;
int des_duty_val_a;
int des_volt_pos_d = 1;
int des_duty_val_d = 0;
int n_LL_soll;

void calc_new_duty_val()
{
    if(control_mode == 'a')
    {
        calc_duty_from_AD();
        des_duty_val = des_duty_val_a;
        des_volt_pos = des_volt_pos_a;
    }else if(control_mode == 'd')
    {
        des_duty_val = des_duty_val_d;
        des_volt_pos = des_volt_pos_d;
    }
}

// Funktion zur Bestimmung des Solltastgrades über analoges Eingangssignal
void calc_duty_from_AD()
{
    int ADval;                      // ausgelesener AD Wert
    
    ADval = get_control_2_ADval();  // Wert des Potentiometers auslesen
    ADval = ADval << 1;             // Mit 2 multiplizieren
    
    if(ADval < 1598)                // Positive Spannung gewünscht
    {
        des_volt_pos_a = 1;
        des_duty_val_a = 1598 - ADval;    // Tastgrad zwischen 1 und 1598 (maximalwert)
    }else if(ADval >  1800){        // Negative Spannung gewünscht
        des_volt_pos_a = 0;
        des_duty_val_a = ADval - 1800;    // Tastgrad zwischen 1 und x (Alle Werte > 1598 werden als Maximalwert interpretiert)
    }else{
        des_duty_val_a = 0;               // Werte zwischen 1598 und 1800 --> Mittelstellung --> Spannung 0
    }
}

//// Funktion zum setzen einer neuen Drehrichtung
//void set_des_dir(char desRtg)
//{
//    if (desRtg != des_richtung)
//    {
//        //Kurzeitiges Verlangsamen des Motors
//        des_speed = 0;
//        // Aktualisierung der Richtung
//        des_richtung = desRtg;
//    }
//}

// Funktion zum setzen eines neuen Tastgrades mit ditigaler Steueurung
void set_des_duty_val_d(int des_duty_val)
{
    des_duty_val_d = (1598 * des_duty_val) / 9;
}

// Funktion zum setzen eines neuen Kontrollmodus
void set_control_mode(char des_mode)
{
    // Absicherung zur Vermeidung möglicher Sprünge im Solltastgrad
    if(control_mode == 'a')  // Bisheriger Controlmode ist analog
    {
        des_duty_val_d = des_duty_val; // Digitaler Solltastgrad wird auf analogen Solltastgrad gesetzt --> Kein Sprung beim Wechsel
        des_volt_pos_d = des_volt_pos; // Spannungsrichtung ebenfalls übernehmen
        control_mode = des_mode;
    }else if(control_mode == 'd')    // Wenn bisheriger Modus digital, Wechsel nur möglich, wenn neuer Analogwert dem bisherigen digitalen Wert entspricht
    {
        calc_duty_from_AD();        // Bestimmung analoger Werte
        if(des_duty_val_a == des_duty_val_d)    // Prüfung --> Sind Werte gleich?
        {
            if(des_volt_pos_a == des_volt_pos_d || des_duty_val_a == 0) // Wenn ja ist Richtung gleich oder sind die Werte 0?
            {
                control_mode = des_mode;                                // Dann ist Wechsel möglich
            }
        }
    }
}

// Funktion zum Setzen der Vorgabe, ob Spannung negativ oder positiv sein soll
void set_volt_pos_d(char desVoltDir)
{
    if(desVoltDir == '+')
    {
        des_volt_pos_d = 1;
    }else if(desVoltDir == '-'){
        des_volt_pos_d = 0;
    }
}

//char getDesRichtung()
//{
//    return des_richtung;
//}

int get_des_duty_val()
{
    return des_duty_val;
}

int des_volt_is_pos()
{
    return des_volt_pos;
}

// Bestimmung der Leerlaufsolldrehzahl anhand Tastgrad
int get_n_LL_soll()
{
    des_duty_val = (des_duty_val > 1598) ? 1598 : des_duty_val;
    n_LL_soll = (des_duty_val*371L)/1598;
    n_LL_soll = (des_volt_pos == 0) ? -n_LL_soll : n_LL_soll;    
}