/* Einbinden der Header-Dateien */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "com_interface.h"
#include "motor_stat.h"
#include "uart_com.h"
#include "des_speed.h"

// Variablendeklaration
char msg_tx[]; // Pointer f�r char-Arrays, zum Speichern der zusendenden Nachrichten
char *msg_rx; // Pointer f�r char-Arrays, zum Speichern einer empfangenen Nachricht
float des_speed;    // Variable zum Speichern der gesendeten Sollgeschwindigkeit

// Funktion zur Initialisierung der Kommunikationssteuerung
void com_interface_init()
{
    CNEN1 = 0xE0; // Aktivieren der Interruptausloesung an den Eingaengen der Hall-Sensoren
    IFS0bits.CNIF = 0; // Loeschen moeglicher ausgeloester Interrupts
    IEC0bits.CNIE = 1; // Aktivieren des Input Change Interrupts
    // msg_tx = "";
}

// Senden der Hall-Sensoren sobald Statusaenderung
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt (void)
{
    IFS0bits.CNIF = 0; // loeschen des Interrupt-Flags
    
    msg_tx[0] = read_HallSensors();
    msg_tx[1] = '\0';
    send_msg(msg_tx);
}

/* Funktion zur Auswertung der �ber UART Empfangenen Nachricht */
void handle_msg_rx(char *msg)
{
    msg_rx = msg;
    // Falls 'msg_rx[0]' den Buchstaben 'v' (vorwärts) oder 'r' (rückwärts) enthält, dann...
    if (msg_rx[0] == 118 || msg_rx[0] == 114) {
        // �?berprüfung einer gewünschten Richtungsänderung durch Vergleich zwischen Kommandobefehl und aktueller Drehrichtung
        set_des_dir(msg_rx[0]);
    } 
        
    // Falls 'msg_rx[0]' keine der beiden Buchstaben 'v' oder 'r' enthält, könnte Befehl eine Geschwindigkeitsänderung sein... 
    else {
        // Umwandlung von char 'msg_rx[0]' in eine float-Variable 'msg_rx[0]'
        des_speed = msg_rx[0] - 48;

        // Falls Befehl eine Zahl im Bereich von 0 und 9 beschreibt, dann hat der Benutzer eine gewünschte Geschwindigkeitsänderung gesendet
        if (des_speed >= 0 && des_speed <= 9) {
            set_des_speed(des_speed);   // Aktualisierung der Geschwindigkeit
        }
    }
}

// Funktion zum Senden des Stromes
//void send_current(int I)
//{
//    msg_tx[0] = (I & 0xFF00) >> 8;
//    msg_tx[1] = (I & 0x00FF);
//    msg_tx[2] = '\0';
//    if(get_send_msg_flag() == 0)
//        send_msg(msg_tx);
//}