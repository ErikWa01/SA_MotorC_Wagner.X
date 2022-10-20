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
float speed = 9;            // Geschwindigkeit der Rotation
char richtung = 'v';              // Richtung der Rotation
float command_speed = 1;


/* Funktion zur Auswertung des �ber UART Empfangenen Kommandos */
void handleCommand(char command)
{
    // Falls 'command' den Buchstaben 'v' (vorwärts) oder 'r' (rückwärts) enthält, dann...
    if (command == 118 || command == 114) {
        // �?berprüfung einer gewünschten Richtungsänderung durch Vergleich zwischen Kommandobefehl und aktueller Drehrichtung
        if (command != richtung) {

            //Kurzeitiges Verlangsamen des Motors
            speed = 1;
            // Aktualisierung der Richtung
            richtung = command;
        }
    } 
        
    // Falls 'command' keine der beiden Buchstaben 'v' oder 'r' enthält, könnte Befehl eine Geschwindigkeitsänderung sein... 
    else {
        // Umwandlung von char 'command' in eine integer-Variable 'command_speed'
        command_speed = command - 48;

        // Falls Befehl eine Zahl im Bereich von 0 und 9 beschreibt, dann hat der Benutzer eine gewünschte Geschwindigkeitsänderung gesendet
        if (command_speed >= 0 && command_speed <= 9) {
            speed = command_speed;      // Aktualisierung der Geschwindigkeit
        }
    }
}

char getDesRichtung()
{
    return richtung;
}

float getDesSpeed()
{
    return speed;
}