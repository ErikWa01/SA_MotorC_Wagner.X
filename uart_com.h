/* 
 * File:   uart_com.h
 * Author: erikw
 *
 * Created on 17. Oktober 2022, 13:28
 */

#ifndef UART_COM_H
#define	UART_COM_H

#ifdef	__cplusplus
extern "C" {
#endif

void UART2_Init();
void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void);
void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void);
char get_command();


#ifdef	__cplusplus
}
#endif

#endif	/* UART_COM_H */

