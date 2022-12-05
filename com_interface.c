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
char msg_tx[100]; // Pointer f�r char-Arrays, zum Speichern der zusendenden Nachrichten
char *msg_rx; // Pointer f�r char-Arrays, zum Speichern einer empfangenen Nachricht
float des_speed;    // Variable zum Speichern der gesendeten Sollgeschwindigkeit
int string_ende;        // Variable zum Speichern der Laufvariable, die auf das Ende des msg-Strings zeigt

// Funktion zur Initialisierung der Kommunikationssteuerung
void com_interface_init()
{
    // msg_tx = "";
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
            set_des_speed(des_speed);   // Aktualisierung der Sollgeschwindigkeit
        }
    }
}

// Funktion zum Senden des Stromes
void send_current()
{
    int I;
    int string_ende;
    
    I = get_I_motor_ADval() - 510;  // Abfragen des ADC-Stromwertes und Bilden von negativen und positiven Stromwerten mit dem 0-Punkt 503
    
    string_ende = itoa(I, msg_tx, 1);  // Wandeln des Integerwertes in einen String und Speichern im msg_tx-Char-Array
    msg_tx[string_ende++] = '\n';   // Erzeugen eines Zeilenumbruchs nach dem Wert
    msg_tx[string_ende] = '\0';     // Setzen des Ende des Strings
    
    send_msg(msg_tx);               // Uebergabe der Nachricht an UART-Kommmunikationsmodul
}

/* Funktion zum Senden der Winkelgeschwindigkeit und des Drehwinkels */
void send_motor_stat()
{
    int tmp_ende;           // Variable zum Speichern der Laufvariable, die auf das Ende des tmp-Strings zeigt
    char tmp[50];           // tmp-String --> Zusaetzlicher String zum Speichern der Drehzahl
    int i;                  // Laufvariable for-Schleife

    string_ende = itoa(get_drehzahl(), msg_tx, 1); // Speichere Drehzahl in msg_tx
    msg_tx[string_ende++] = '\n';                   // Zeilenumbruch
    tmp_ende = itoa(get_I_motor_ADval() - 510, tmp, 1);  // Speichere Motorstrom in einem zusaetzlichem String
    
    // Fuege zusaetzlichen String zu msg_tx hinzu
    for(i = 0; i < tmp_ende; i++)   
    {
        msg_tx[string_ende++] = tmp[i];
    }
    
    msg_tx[string_ende++] = '\n';   // Zeilenumbruch
    msg_tx[string_ende++] = '-';    // Markierung, visuelles Nachrichtende
    msg_tx[string_ende++] = '\n';   // Zeilenumbruch
    msg_tx[string_ende] = '\0';     // Technisches Nachrichtende
    
    send_msg(msg_tx);
}

// Funktion zum Senden des Stromes
void send_drehwinkel()
{
    string_ende = itoa(get_drehwinkel(), msg_tx, 0);  // Wandeln des Integerwertes in einen String und Speichern im msg_tx-Char-Array
    msg_tx[string_ende++] = '\n';   // Erzeugen eines Zeilenumbruchs nach dem Wert
    msg_tx[string_ende] = '\0';     // Setzen des Ende des Strings
    
    send_msg(msg_tx);               // Uebergabe der Nachricht an UART-Kommmunikationsmodul
}

/* Funktion zum Umwandeln eines signed oder unsigned Integer in einen String
 * 
 * �bergabeparameter:
 *  - value: der zu wandelnde Integerwert
 *  - str: pointer auf die Speicheradresse, die erstes Byte des Strings enthalten soll
 *  - is_signed: 1, wenn der Integerwert signed ist, sonst 0
 * 
 * R�ckgabeparameter:
 *  - Integerwert, der auf die letzte Speicheradresse des Strings zeigt
 */
int itoa(int value, char *str, int is_signed)
{
    int i = 0;              // Laufvariable
    int divisor = 10000;    // Durch diesen Wert wird der Int-Wert geteilt
    int printed = 0;        // Erkennung, ob bereits eine Ziffer gespeichert wurde
    unsigned int u_value;
    
    // Pruefen, ob Zahl signed ist
    if(is_signed)
    {
        // Pruefen, ob die Zahl negativ ist
        if(value < 0)
        {
            str[i] = '-';       // Ist die Zahl negativ wird ein Minus gespeichert
            value = -value;     // und der Betrag der Zahl fuer die weitere Verarbeitung gebildet
            i++;
        }
    }
    
    u_value = value;
    
    // Solange Divisor mindestens 1 ist, sind noch Ziffern zu speichern
    while(divisor > 0)
    {
        // Gespeichert wird eine Ziffer nur, wenn an der Stelle mit der Wertigkeit des Divisors eine Ziffer > 0 ist,
        // bereits eine Ziffer gespeichert wurde oder der Divisor 1 ist (Dann ist die Zahl 0)
        if(u_value/divisor || printed || divisor == 1)
        {
            str[i] = u_value/divisor + '0';   // Extrahieren der Stelle mit der Wertigkeit des Divisors und umwandeln in ASCII-Code
            u_value = u_value % divisor;        // Bilden des noch zu wandelnden Wertes durch Modulo-Operation 
            printed = 1;                    // Erkennung, dass bereits eine Ziffer gespeichert wurde
            i++;                            // Erhoehen der Laufvariable, so dass beim naechsten Schreibvorgang die naechste Speicherstelle beschrieben wird
        }
        divisor = divisor/10;               // Verringern des Divisors um eine Stellenwertigkeit
    }
    
    str[i] = '\0';      // Setzen des Ende des Strings
    
    return i;           // Rueckgabe der Variable, die auf das Ende des Strings zeigt
}