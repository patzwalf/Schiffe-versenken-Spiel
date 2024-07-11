/* 
 * File:   UART.h
 * Author: Jan Ritter (105773) & Fabian Patzwall (106081)
 *
 * Created on 18. April 2024, 09:46
 * 
 * Die UART.c ermöglicht den Empfand und das Senden von Nachrichten via UART. 
 * Die Implementation blockiert nicht signifikant die Laufzeit.
 */

#ifndef UART_H
#define	UART_H



#ifdef	__cplusplus
extern "C" {
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

void UART_init(void);

void UART_putC(unsigned char data);

unsigned char UART_ReceiveC();

void putS(char* str);
