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

// Eigene Header
#include "uart_com.h"
#include "motor_stat.h"
#include "commutation.h"
#include "des_speed.h"
#include "com_interface.h"
#include "global_const.h"
// #include "p30F4011.h"


/*
 * 
 */
int main() {
    PWM_Init();         // Initialisierung der PWM
    HallSensor_Init();  // Initialisierung der Hall-Sensoren
    UART2_Init();       // Initialisierung des UART2-Moduls
    com_interface_init(); // Initialisierung der Kommunikationsschnittstelle
    
    while (1)
    {
        handle_msg_rx();
        motor_commutation(getDesRichtung(), getDesSpeed(), read_HallSensors());
    }
    
    return (EXIT_SUCCESS);
}

