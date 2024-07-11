/*
 * File:   UART.c
 * Author: Jan Ritter (105773) & Fabian Patzwall (106081)
 *
 * Created on 18. April 2024, 09:57
 */

#include <avr/io.h>
#include <util/delay.h>


#define FOSC 1000000;
#define BAUD 9600;

/*
 * Initialisiert alle Hardwarekomponenten, die fuer die UART-Kommunikation 
 * notwednig sind
 */
void UART_init(void) {
    /*Setzen der Baudrate*/
    UBRR0 = 12;
    /*TRX & RTX Pins aktivieren*/
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    /*Frame Format: 8Data, 2Stop Bits*/
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
    /*Setzen U2X0*/
    UCSR0A |= 1<<U2X0;
}

/*
 * Schreibt einen Char nach UART
 * @param data: der Char
 */
void UART_putC( unsigned char data) {
    while ( ! (UCSR0A & (1<<UDRE0)));
        UDR0 = data;
}

/*
 * schreibt einen C-String nach UART
 * @param str: der String
 */
void putS(char* str) {
    while (*str) {
        UART_putC(*str);
        str++;
    }
}


/*
 * Empfaengt Daten ueber UART
 * @ret: die Daten
 */
unsigned char UART_ReceiveC() {
    //Warte das Daten empfangen werden (RXC0 Flag)
    while (! (UCSR0A & (1<<RXC0)));
        //Hole die Daten aus dem Bufferregister
        return UDR0;
}
 