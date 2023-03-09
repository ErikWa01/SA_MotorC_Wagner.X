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
char msg_tx[100]; // Pointer für char-Arrays, zum Speichern der zusendenden Nachrichten
char *msg_rx; // Pointer für char-Arrays, zum Speichern einer empfangenen Nachricht
int string_ende;        // Variable zum Speichern der Laufvariable, die auf das Ende des msg-Strings zeigt

// Funktion zur Initialisierung der Kommunikationssteuerung
void com_interface_init()
{
    // Keine Initialisierung notwendig
}

/* Funktion zur Auswertung der über UART Empfangenen Nachricht */
void handle_msg_rx(char *msg)
{
    msg_rx = msg;
    
    // Enthaelt msg_rx[0] das Zeichen '-' ist Sollspannung negativ anzusehen, bei '+' positiv
    if(msg_rx[0] == '-' || msg_rx[0] == '+')
    {
        set_volt_pos_d(msg_rx[0]);
        
    // Enthaelt msg_rx[0] das Zeichen 'a' ist gewuenschter Controlmode analog, bei 'd' digital
    }else if(msg_rx[0] == 'a' || msg_rx[0] == 'd')
    {
        set_control_mode(msg_rx[0]);
    }
    
    // Falls 'msg_rx[0]' keine der Zeichen '-', '+', 'a' oder 'd' enthaelt, koennte Befehl eine Tastgradaenderung sein
    else
    {
        // Umwandlung von char 'msg_rx[0]' in eine float-Variable 'msg_speed'
        float msg_duty_val = (float)(msg_rx[0] - 48);

        // Falls Befehl eine Zahl im Bereich von 0 und 9 beschreibt, dann hat der Benutzer eine gewuenschte Tastgradaenderung gesendet
        if (msg_duty_val >= 0 && msg_duty_val <= 9) {
            set_des_duty_val_d(msg_duty_val);   // Aktualisierung des Solltastgrades
        }
    }
}

// Funktion zum Senden des Stromes
void send_current()
{
    int I;
    int string_ende;
    
    I = get_I_motor_ADval();  // Abfragen des ADC-Stromwertes und Bilden von negativen und positiven Stromwerten mit dem 0-Punkt 503
    
    string_ende = itoa(I, msg_tx, 1);  // Wandeln des Integerwertes in einen String und Speichern im msg_tx-Char-Array
    msg_tx[string_ende++] = '\n';   // Erzeugen eines Zeilenumbruchs nach dem Wert
    msg_tx[string_ende] = '\0';     // Setzen des Ende des Strings
    
    send_msg(msg_tx);               // Uebergabe der Nachricht an UART-Kommmunikationsmodul
}

/* Funktion zum Senden der Drehzahl und des Motorstroms - Format MATLAB
 * Wurde genutzt, um Daten für die Graphenerstellung in MATLAB zu formatieren
 */
//void send_motor_stat()
//{
//    int tmp_ende;           // Variable zum Speichern der Laufvariable, die auf das Ende des tmp-Strings zeigt
//    char tmp[50];           // tmp-String --> Zusaetzlicher String zum Speichern der Drehzahl
// 
//
//    tmp_ende = itoa(get_n_LL_soll(), tmp, 1);
//    string_ende = add_to_string(&msg_tx[0], tmp, tmp_ende);
//    string_ende += add_to_string(&msg_tx[string_ende], ",", 1);
//            
//    tmp_ende = itoa(get_drehzahl(), tmp, 1);                             // Speichere Drehzahl in tmp
//    string_ende += add_to_string(&msg_tx[string_ende], tmp, tmp_ende);    // Fuege tmp an msg_tx an
//    string_ende += add_to_string(&msg_tx[string_ende], ",", 1);
//    
//    tmp_ende = itoa(get_I_motor_ADval(), tmp, 1);  // Speichere Motorstrom in einem zusaetzlichem String
//    string_ende += add_to_string(&msg_tx[string_ende], tmp, tmp_ende);
//    
//    msg_tx[string_ende++] = '\n';   // Zeilenumbruch
//    msg_tx[string_ende] = '\0';     // Technisches Nachrichtende
//    
//    send_msg(msg_tx);
//}

/* Funktion zum Senden der Drehzahl und des Motorstroms */
void send_motor_stat()
{
    int tmp_ende;           // Variable zum Speichern der Laufvariable, die auf das Ende des tmp-Strings zeigt
    char tmp[50];           // tmp-String --> Zusaetzlicher String zum Speichern der Drehzahl
 

    /* Folgende Zeilen erstellen einen String im Format:
     * 
     * LL n soll: x
     * n ist: y
     * I ist: z
     * 
     * Es wird somit die Soll-Leerlaufdrehzahl, die Ist-Drehzahl und der Strom ausgegeben
     */
    string_ende = add_to_string(msg_tx, "LL n soll: ", 11);             // Fuege in Nachricht: "LL n soll: " ein
    tmp_ende = itoa(get_n_LL_soll(), tmp, 1);                           // Wandle Soll-Leerlaufdrehzahl in String tmp um
    string_ende += add_to_string(&msg_tx[string_ende], tmp, tmp_ende);  // Fuege tmp an Nachrichtende hinzu
    
    string_ende += add_to_string(&msg_tx[string_ende], "\nn ist: ", 8); // Fuege "\nn ist: " an Nachrichtende hinzu
    tmp_ende = itoa(get_drehzahl(), tmp, 1);                            // Wandle Drehzahl in String tmp
    string_ende += add_to_string(&msg_tx[string_ende], tmp, tmp_ende);  // Fuege tmp an msg_tx hinzu
    
    string_ende += add_to_string(&msg_tx[string_ende], "\nI ist: ", 8); // Fuege "\nI ist: " an Nachrichtende hinzu
    tmp_ende = itoa(get_I_motor_ADval(), tmp, 1);                       // Wandle Strom in String tmp
    string_ende += add_to_string(&msg_tx[string_ende], tmp, tmp_ende);  // Fuege tmp an msg_tx hinzu
    
    msg_tx[string_ende++] = '\n';   // Zeilenumbruch
    msg_tx[string_ende++] = '\n';   // Zeilenumbruch
    msg_tx[string_ende] = '\0';     // Technisches Nachrichtende
    
    send_msg(msg_tx);   // Sende Nachricht
}

// Funktion zum Senden des Stromes
void send_drehwinkel()
{
    string_ende = itoa(get_drehwinkel(), msg_tx, 0);  // Wandeln des Integerwertes in einen String und Speichern im msg_tx-Char-Array
    msg_tx[string_ende++] = '\n';   // Erzeugen eines Zeilenumbruchs nach dem Wert
    msg_tx[string_ende] = '\0';     // Setzen des Ende des Strings
    
    send_msg(msg_tx);               // Uebergabe der Nachricht an UART-Kommmunikationsmodul
}

//Funktion zum Senden des Eingangswertes an Control-Anschluss Pin 2
void send_control_2_ADval()
{
    string_ende = itoa(get_control_2_ADval(), msg_tx, 0);   // Wandeln des Integerwertes in einen String und Speichern im msg_tx-Char-Array
    msg_tx[string_ende++] = '\n';   // Erzeugen eines Zeilenumbruchs nach dem Wert
    msg_tx[string_ende] = '\0';     // Setzen des Ende des Strings
    
    send_msg(msg_tx);               // Uebergabe der Nachricht an UART-Kommmunikationsmodul
}

/* Funktion zum Umwandeln eines signed oder unsigned Integer in einen String
 * 
 * Übergabeparameter:
 *  - value: der zu wandelnde Integerwert
 *  - str: pointer auf die Speicheradresse, die erstes Byte des Strings enthalten soll
 *  - is_signed: 1, wenn der Integerwert signed ist, sonst 0
 * 
 * Rückgabeparameter:
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

/* Funktion zum anhaengen eines Strings an einen anderen String
 *
 * Übergabeparameter:
 *  - dest: pointer auf die letzte beschriebene Stelle des Zielstrings
 *  - source: pointer auf die Anfangsadresse des einzufuegenden Strings
 *  - size: Laenge des einzufuegenden Strings
 * 
 * Rückgabeparameter:
 *  - Anzahl hinzugefuegter Zeichen (int)
 */
int add_to_string(char *dest, char *source, int size)
{
    int i;
    
    for(i = 0; i < size; i++)   
    {
        dest[i] = source[i];
    }
    
    dest[i] = '\0';
    
    return i;
}