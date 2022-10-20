/* Einfuegen benoetigter Header-Dateien */
#include "motor_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"

// Test

/* Funktion zur Initialisierung der Hall-Sensoren */
void HallSensor_Init() {

    TRISB = 0x0038;     // Festlegung des Inputs (TRISB3, TRISB4 und TRISB5 sind Inputs)
    ADPCFG = 0xFFFF;    // Alle Inputs sind digital
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zustände der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}