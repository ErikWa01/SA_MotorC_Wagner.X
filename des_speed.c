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


/* Deklaration von Variablen */
float des_speed = 1;            // Geschwindigkeit der Rotation
char des_richtung = 'v';              // Richtung der Rotation
char des_control_mode = 's';

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
    if(des_mode != des_control_mode)
    {
        // Setzen des Tastgrades auf 1 beim umschalten des Steuerungsmodus
        des_speed = 1;
        // Aktualisieren des Steuerungsmodus
        des_control_mode = des_mode;
    }
}


char getDesRichtung()
{
    return des_richtung;
}

float getDesSpeed()
{
    return des_speed;
}

char get_des_control_mode()
{
    return des_control_mode;
}