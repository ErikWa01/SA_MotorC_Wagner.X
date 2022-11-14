/* Einfuegen benoetigter Header-Dateien */
#include "motor_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"
#include "com_interface.h"

// Variablendeklaration


/* Funktion zur Initialisierung der Hall-Sensoren und des AD-Wandlers */
void motor_stat_init() {

    TRISB |= 0x38;     // Festlegung des Inputs (TRISB2, TRISB3, TRISB4 sind Inputs)
    ADPCFG |= 0x38;    // RB3 - RB5 sind digital
}

/* Funktion, um den Status der Hall-Sensoren auszulesen */
int read_HallSensors()
{
    return (PORTB & 0x0038) >> 3;    // Lesen der Zustände der Hall-Sensoren und direktes Verschieben der Bits um drei nach rechts
}