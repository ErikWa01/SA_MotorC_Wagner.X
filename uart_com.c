/* Einfuegen benoetigter Header-Dateien */
#include "uart_com.h"
#include "global_const.h"
#include "motor_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"

/* Definition von Konstanten */
#define BAUD 9600           // Definition der Baudrate

/* Deklaration von Variablen */
uint16_t Predefiner = ((FCY / BAUD) / 16) - 1; // Einstellen der Baudrate

char command = 'v';         // Startwert des Kommandos zur Initialisierung einer Vorwärtsdrehung


/* Funktion zur Initialisierung der UART-Kommunikation */
void UART2_Init() {

    U2MODEbits.PDSEL = 0;   // Festlegen der Datenlänge und der Parität (8N1)
    U2MODEbits.STSEL = 1;   // Anzahl der Stopbits (1)

    U2BRG = Predefiner;     // Berechnung zur gewünschten Baudrate

    U2MODEbits.UARTEN = 1;  // UART aktivieren
    U2STAbits.UTXEN = 1;    // UART-�?bertragung aktivieren

    IEC1bits.U2TXIE = 1;    // Aktivieren des TX-Interrupts zum Senden
    IEC1bits.U2RXIE = 1;    // Aktivieren des RX-Interrupts zum Empfangen

    U2STAbits.URXISEL = 1;  // Einstellung des RX-Interrupts - Interrupt wird jedes Mal erzeugt, wenn ein Datenwort aus dem Empfangs-Schieberegister (UxRSR) in den Empfangspuffer übertragen wird
}

/* Interrupt zum senden des Status der Hall-Sensoren*/
void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
    
    IFS1bits.U2TXIF = 0;    // TX-Interrupt-Flag löschen
    
    /* Folgende Zeile ist in Verwendung mit HTerm und gleichzeitigem Senden von Daten auszukommentieren, da Software HTerm hierbei häufig abstürzt */
    U2TXREG = read_HallSensors();  // Senden der Zustände der Hall-Sensoren      
}

/* Interrupt zum Speichern der empfangenen Daten �ber UART */
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {

    IFS1bits.U2RXIF = 0;    // RX-Interrupt-Flag löschen
    command = U2RXREG;      // Empfangen der gesendeten Daten und Abspeichern in Variable 'command' zur Weiterverwendung
}

/* Funktion zur �bergabe der empfangenen Kommandobefehle */
char get_command()
{
    return command;
}