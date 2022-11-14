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
#include "adc_module.h"

// Variablendeklaration
char msg_tx[]; // Pointer für char-Arrays, zum Speichern der zusendenden Nachrichten
char *msg_rx; // Pointer für char-Arrays, zum Speichern einer empfangenen Nachricht
float des_speed;    // Variable zum Speichern der gesendeten Sollgeschwindigkeit

// Funktion zur Initialisierung der Kommunikationssteuerung
void com_interface_init()
{
    CNEN1 = 0xE0; // Aktivieren der Interruptausloesung an den Eingaengen der Hall-Sensoren
    IPC3bits.CNIP = 6;  // Setzen der Interrupt-Prioritaet auf die Hoechste
    IFS0bits.CNIF = 0; // Loeschen moeglicher ausgeloester Interrupts
//    IEC0bits.CNIE = 1; // Aktivieren des Input Change Interrupts
    // msg_tx = "";
}

// Senden der Hall-Sensoren sobald Statusaenderung
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt (void)
{
    IFS0bits.CNIF = 0; // loeschen des Interrupt-Flags
    
    msg_tx[0] = read_HallSensors();     // Speichern des Hallstatus als 1-Byte grosse Nachricht im Char-Array-Format
    msg_tx[1] = '\0';                   // Abschließen des Char-Arrays
    send_msg(msg_tx);                   // Uebergabe der Nachricht an UART-Kommunikationsmodul
}

/* Funktion zur Auswertung der über UART Empfangenen Nachricht */
void handle_msg_rx(char *msg)
{
    msg_rx = msg;
    // Falls 'msg_rx[0]' den Buchstaben 'v' (vorwÃ¤rts) oder 'r' (rÃ¼ckwÃ¤rts) enthÃ¤lt, dann...
    if (msg_rx[0] == 118 || msg_rx[0] == 114) {
        // Ã?berprÃ¼fung einer gewÃ¼nschten RichtungsÃ¤nderung durch Vergleich zwischen Kommandobefehl und aktueller Drehrichtung
        set_des_dir(msg_rx[0]);
    } 
        
    // Falls 'msg_rx[0]' keine der beiden Buchstaben 'v' oder 'r' enthÃ¤lt, kÃ¶nnte Befehl eine GeschwindigkeitsÃ¤nderung sein... 
    else {
        // Umwandlung von char 'msg_rx[0]' in eine float-Variable 'msg_rx[0]'
        des_speed = msg_rx[0] - 48;

        // Falls Befehl eine Zahl im Bereich von 0 und 9 beschreibt, dann hat der Benutzer eine gewÃ¼nschte GeschwindigkeitsÃ¤nderung gesendet
        if (des_speed >= 0 && des_speed <= 9) {
            set_des_speed(des_speed);   // Aktualisierung der Sollgeschwindigkeit
        }
    }
}

// Funktion zum Senden des Stromes
void send_current()
{
    int I;
    int string_ende;
    
    I = get_I_motor_ADval() - 506;  // Abfragen des ADC-Stromwertes und Bilden von negativen und positiven Stromwerten mit dem 0-Punkt 503
    
    string_ende = itoa(I, msg_tx);  // Wandeln des Integerwertes in einen String und Speichern im msg_tx-Char-Array
    msg_tx[string_ende++] = '\n';   // Erzeugen eines Zeilenumbruchs nach dem Wert
    msg_tx[string_ende] = '\0';     // Setzen des Ende des Strings
    
    send_msg(msg_tx);               // Uebergabe der Nachricht an UART-Kommmunikationsmodul
}

/* Funktion zum Umwandeln eines signed Integer in einen String
 * 
 * Übergabeparameter:
 *  - value: der zu wandelnde Integerwert
 *  - str: pointer auf die Speicheradresse, die erstes Byte des Strings enthalten soll
 * 
 * Rückgabeparameter:
 *  - Integerwert, der auf die letzte Speicheradresse des Strings zeigt
 */
int itoa(int value, char *str)
{
    int i = 0;              // Laufvariable
    int divisor = 10000;    // Durch diesen Wert wird der Int-Wert geteilt
    int printed = 0;        // Erkennung, ob bereits eine Ziffer gespeichert wurde
    
    // Pruefen, ob die Zahl negativ ist
    if(value < 0)
    {
        str[i] = '-';       // Ist die Zahl negativ wird ein Minus gespeichert
        value = -value;     // und der Betrag der Zahl fuer die weitere Verarbeitung gebildet
        i++;
    }
    
    // Solange Divisor mindestens 1 ist, sind noch Ziffern zu speichern
    while(divisor > 0)
    {
        // Gespeichert wird eine Ziffer nur, wenn die an der Stelle mit der Wertigkeit des Divisors eine Ziffer > 0 ist,
        // bereits eine Ziffer gespeichert wurde oder der Divisor 1 ist (Dann ist die Zahl 0)
        if(value/divisor || printed || divisor == 1)
        {
            str[i] = value/divisor + '0';   // Extrahieren der Stelle mit der Wertigkeit des Divisors und umwandeln in ASCII-Code
            value = value % divisor;        // Bilden des noch zu wandelnden Wertes durch Modulo-Operation 
            printed = 1;                    // Erkennung, dass bereits eine Ziffer gespeichert wurde
            i++;                            // Erhoehen der Laufvariable, so dass beim naechsten Schreibvorgang die naechste Speicherstelle beschrieben wird
        }
        divisor = divisor/10;               // Verringern des Divisors um eine Stellenwertigkeit
    }
    
    str[i] = '\0';      // Setzen des Ende des Strings
    
    return i;           // Rueckgabe der Variable, die auf das Ende des Strings zeigt
}