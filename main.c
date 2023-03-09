/* 
 * File:   main.c
 * Author: erikw
 *
 * Created on 17. Oktober 2022, 12:40
 */


/* Konfiguration des Mikrocontrollers ueber Device Configuration Register mit entsprechenden Configuration Bits */

// FOSC
#pragma config FPR = XT_PLL8            // Primary Oscillator Mode (XT w/PLL 8x)
#pragma config FOS = PRI                // Oscillator Source (Primary Oscillator)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_1         // WDT Prescaler B (1:1)
#pragma config FWPSA = WDTPSA_1         // WDT Prescaler A (1:1)
#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_OFF         // POR Timer Value (Timer Disabled)
#pragma config BODENV = BORV20          // Brown Out Voltage (Reserved)
#pragma config BOREN = PBOR_OFF         // PBOR Enable (Disabled)
#pragma config LPOL = PWMxL_ACT_HI      // Low-side PWM Output Polarity (Active High)
#pragma config HPOL = PWMxH_ACT_HI      // High-side PWM Output Polarity (Active High)
#pragma config PWMPIN = RST_IOPIN       // PWM Output Pin Reset (Control with PORT/TRIS regs)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = CODE_PROT_OFF      // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)


/* Einbinden benoetigter Header-Dateien */

// Library-Header
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "p30F4011.h"

// Eigene Header
#include "uart_com.h"
#include "motor_stat.h"
#include "commutation.h"
#include "des_speed.h"
#include "com_interface.h"
#include "global_const.h"
#include "scheduler.h"
#include "adc_module.h"

// Variablen fuer Tests und Zeitbestimmung
    //int temp;
    //int count = 0;
    //int value = 0;

/*
 * 
 */
int main() {
    PWM_Init();         // Initialisierung der PWM
    UART2_Init();       // Initialisierung des UART2-Moduls
    com_interface_init(); // Initialisierung der Kommunikationsschnittstelle
    adc_init();             // Initialisierung des AD-Wandler-Moduls
    motor_stat_init();  // Initialisierung der Hall-Sensoren und der Strommessung
    scheduler_init();       // Initialisierung des Timers fuer zeitgesteuerte Funktionsaufrufe
//    temp = ADCBUF0;

    /** Folgender Code ist fuer die Bestimmung des Oszillator-Taktes
//    TRISDbits.TRISD2 = 0;   // Pin D2, gleichbedeutend mit OSC3 ist Output
//    T2CONbits.TCS = 0;      // Interne Zeitbasis: TCY
//    T2CONbits.T32 = 0;      // 16 Bit Timer
//    T2CONbits.TCKPS = 0;    // Prescale 1:1
//    PR2 = 1599;              // Zuruecksetzen des Timers nach 1599+1 * 62,5 ns = 100 µs
//    
//    OC3CONbits.OCTSEL = 0;  // Timer 2 ist Zeitbasis
//    OC3R = 1599;             // Toggle des Ausgangs nach 1599+1 * 62,5 ns = 100 µs
//    OC3CONbits.OCM = 0xB;   // Compare-Event Toggled Ausgang OC3
//    T2CONbits.TON = 1;      // Timer 2 starten
     **/
    
    while (1)
    {
//        // Folgender Code war zur Zeitbestimmung der AD-Wandlung gedacht --> nicht mehr benoetigt
//        if(temp != ADCBUF0)
//        {
//            if(count == 0){
//                T1CON = 0x8000;
//                count++;
//                temp = ADCBUF0;
//            }else if(count == 1)
//            {
//                value = TMR1;
//                T1CON = 0x0000;
//                TMR1 = 0;
//            }
//        }
    }
    
    return (EXIT_SUCCESS);
}

