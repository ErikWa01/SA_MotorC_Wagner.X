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
#include "com_interface.h"

/* Definition von Konstanten */
#define BAUD 9600           // Definition der Baudrate

/* Deklaration von Variablen */
uint16_t Predefiner = ((FCY / BAUD) / 16) - 1; // Einstellen der Baudrate

int msg_count = 0;          // Laufvariable zum Durchlaufen der Char-Arrays der zu sendenden Nachrichten
char *msg_uart_tx;               // Pointer auf Anfang des Char-Arrays einer zu sendenen Nachricht
int send_msg_flag;      // Flag zum erkennen ob Nachricht gesendet werden soll oder nicht
char msg_uart_rx[100];               // Pointer auf Anfang des Char-Arrays einer empfangenen Nachricht


/* Funktion zur Initialisierung der UART-Kommunikation */
void UART2_Init() {

    U2MODEbits.PDSEL = 0;   // Festlegen der DatenlÃ¤nge und der ParitÃ¤t (8N1)
    U2MODEbits.STSEL = 1;   // Anzahl der Stopbits (1)

    U2BRG = Predefiner;     // Berechnung zur gewÃ¼nschten Baudrate

    U2MODEbits.UARTEN = 1;  // UART aktivieren
    U2STAbits.UTXEN = 1;    // UART-Ã?bertragung aktivieren

    IPC6bits.U2TXIP = 5;    // Interrupt-Prioritaet der Komm.-Interrupts
    IPC6bits.U2RXIP = 5;    // niedriger als Timer und Hallsensoren setzen
    IFS1bits.U2TXIF = 0;    // Rücksetzen des Interrupt-Flags für TX
    IFS1bits.U2RXIF = 0;    // Rücksetzen des Interrupt-Flags für RX
    IEC1bits.U2TXIE = 1;    // Aktivieren des TX-Interrupts zum Senden
    IEC1bits.U2RXIE = 1;    // Aktivieren des RX-Interrupts zum Empfangen

    U2STAbits.URXISEL = 1;  // Einstellung des RX-Interrupts - Interrupt wird jedes Mal erzeugt, wenn ein Datenwort aus dem Empfangs-Schieberegister (UxRSR) in den Empfangspuffer Ã¼bertragen wird
    
    msg_uart_tx[0] = '\0';       // Initialisierung des Arrays als leeren Strings
    msg_uart_rx[0] = 'v';        // Initialisierung des Empfangs-Arrays mit 'v' fuer Vorwaertsbewegung
    msg_uart_rx[1] = '\0';
    
    send_msg_flag = 0;
}

void send_msg(char *msg)
{
    if(send_msg_flag == 0)
    {
        msg_uart_tx = msg; // Speichern der zu sendenden Nachricht in einem Char-Field
        msg_count = 0;  // Setzen der Laufvariable auf ersten Character
        send_msg_flag = 1;  // Setzen des Flags zum senden der Nachricht
        U2TXREG = msg_uart_tx[0]; // Senden des ersten Characters der Nachricht
    }
}

/* Interrupt zum senden von Nachrichten*/
void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
    
    IFS1bits.U2TXIF = 0;    // TX-Interrupt-Flag loeschen
    
    if(send_msg_flag == 1)   // Pruefen ob Nachricht gesendet werden soll
    {
        msg_count++;    // Durchlaufen der character
        if(msg_uart_tx[msg_count] != '\0') // Pruefen ob Ende der Nachricht erreicht ist
        {
            U2TXREG = msg_uart_tx[msg_count];  // Wenn Ende noch nicht erreicht ist senden des naechsten characters
        }else{
            send_msg_flag = 0;
        }
    }
}

/* Interrupt zum Speichern der empfangenen Daten über UART */
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {

    IFS1bits.U2RXIF = 0;    // RX-Interrupt-Flag lÃ¶schen
    msg_uart_rx[0] = U2RXREG;    // Empfangen der gesendeten Daten und Abspeichern in Variable Empfangsarray zur Weiterverwendung
    msg_uart_rx[1] = '\0';       // Definiertes Ende des Arrays
    
    handle_msg_rx(msg_uart_rx);
}