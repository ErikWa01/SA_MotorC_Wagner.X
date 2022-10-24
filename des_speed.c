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
float speed = 1;            // Geschwindigkeit der Rotation
char richtung = 'v';              // Richtung der Rotation

// Funktion zum setzen einer neuen Drehrichtung
void set_des_dir(char desRtg)
{
    if (desRtg != richtung)
    {
        //Kurzeitiges Verlangsamen des Motors
        speed = 1;
        // Aktualisierung der Richtung
        richtung = desRtg;
    }
}

// Funktion zum setzen einer neuen Geschwindigkeit
void set_des_speed(float des_speed)
{
    speed = des_speed;
}


char getDesRichtung()
{
    return richtung;
}

float getDesSpeed()
{
    return speed;
}