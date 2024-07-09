/*
 * File:   main.c
 * Author: Jan Ritter (105773) & Fabian Patzwall (106081)
 *
 * Created on 18. April 2024, 09:50
 */


#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "UART.h"
#include "Game.h"
#include "apa102.h"
#include <util/delay.h>

#define MAX_MESSAGE_LENGTH 16

board myBoard;
board theirBoard;

volatile char receivedMessageBuffer[MAX_MESSAGE_LENGTH];
volatile char receivedMessage[MAX_MESSAGE_LENGTH];
volatile char currentMessage[MAX_MESSAGE_LENGTH];
volatile uint8_t messageIndex = 0;
volatile bool messageComplete = false;

enum State {
    STATE_PLACE_SHIPS,
    STATE_INIT,
    STATE_MYTURN,
    STATE_WAITFORANSWER,
    STATE_THEIRTURN,
    STATE_GAMEOVER
};

//Variable um den Zustand zu halten
enum State currentState = STATE_PLACE_SHIPS;

//Interrupt um bei vollem Register die empfangenen Datenauszulesen
ISR(USART_RX_vect) {
    
    char receivedChar = UDR0;
    
    if (messageIndex < MAX_MESSAGE_LENGTH - 1) {
        receivedMessageBuffer[messageIndex] = receivedChar;
        messageIndex++;
    }
    
    //if (receivedChar == '\n' || messageIndex >= MAX_MESSAGE_LENGTH - 1)messageIndex > 4
    if (receivedChar == '\n' || messageIndex >= MAX_MESSAGE_LENGTH - 1){//TODO Zwischen Nachricht und Endzeichen muss bei uns immer ein leerzeichen bestehen
        receivedMessageBuffer[messageIndex] = '\n'; // Null-Zeichen hinzufügen, um die Nachricht zu beenden
        messageIndex = 0; // Index zurücksetzen
        strcpy(receivedMessage, receivedMessageBuffer);
        //for (int i = 0 ; i < MAX_MESSAGE_LENGTH ; i++){
        //    receivedMessageBuffer[i] = ' ';
        //}
        messageComplete = true; // Nachricht empfangen markieren
    }
}

int main(void) {
    
    init_hardware();
    sei(); //Globale Interrupts ein
    UART_init(); //Initialieseren UART Parameter
    init_field(&myBoard);
    init_field(&theirBoard);
    
    char command[4] = "";
    int x = 0;
    int y = 0;
    int AliveShips = 0;
    int Gegnernummer = 0; 
    uint8_t opponentReady = 0;
    bool won = false;
    
    
   apa_Init();
   _delay_ms(50);
   // Setzt das Display auf gewünschte Farbe
   apa_setUnicolor('g');
    while (1) {
    
        //Zustandsmaschine
        switch (currentState) {
            case STATE_PLACE_SHIPS:
                updatePosition(4, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                srand((TCNT1)); //Zufallsgenerator Initialisieren
                uint8_t myNumber = rand() % 100; //Zufallszahl zwischen 0-99;
                
                updatePosition(3, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                updatePosition(3, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                updatePosition(2, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                updatePosition(2, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                updatePosition(1, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                updatePosition(1, &myBoard);
                AliveShips++;
                apa_showBoard(&myBoard);
                
                sprintf((char *)currentMessage, "RDY%d", myNumber);
                currentState = STATE_INIT;
                break;
            
            case STATE_INIT:
                if (messageComplete && checkMessage(receivedMessage)) {
                    parseMessage(&receivedMessage, &command, &x, &y, opponentReady);
                    messageComplete = false; //Zurücksetzen für neue Nachricht
                    if (strcmp(command, "RDY") == 0) {                      
                        Gegnernummer = (x*10)+y;
                        opponentReady = 1;
                    }
                }     
                if (opponentReady) {                               
                    if (myNumber > Gegnernummer) {
                        currentState = STATE_THEIRTURN;
                    } else if (myNumber < Gegnernummer) {
                        currentState = STATE_MYTURN;
                    } else if (myNumber = Gegnernummer) {
                        srand((TCNT1)); //Zufallsgenerator erneut initialisieren
                        myNumber = rand() % 100;
                        sprintf((char *)currentMessage, "RDY%d", myNumber);
                        opponentReady = false;
                    }
                }
                _delay_ms(50); // Entspannungszeit für Hterm
                putS(&currentMessage);
                break;
                
            case STATE_THEIRTURN:     
                apa_showBoard(&myBoard);
                
                sprintf((char *)currentMessage, "RPT%d%d", 0, 0);
                putS(&currentMessage);
                
                if (messageComplete) {
                    if (checkMessage(receivedMessage)) {
                        parseMessage(receivedMessage, command, &x, &y, opponentReady);
                        messageComplete = false; //Zurücksetzen für neue Nachricht     
                        if (strcmp(command, "SHT") == 0) {                      
                            if (myBoard[x][y] == SHIP) {
                                if(isSunk(&myBoard,x,y)){
                                    sprintf((char *)currentMessage, "SNK%c%d", x + 'A', y);
                                    AliveShips--;
                                    currentState = STATE_MYTURN;
                                    if (AliveShips == 0){
                                        sprintf((char *)currentMessage, "OVR%c%d", x + 'A', y);
                                        currentState = STATE_GAMEOVER;
                                    }
                                }else{
                                sprintf((char *)currentMessage, "HIT%c%d", x + 'A', y);
                                myBoard[x][y] = HIT;
                                currentState = STATE_MYTURN;
                                }                         
                            } else {
                                sprintf((char *)currentMessage, "MIS%c%d", x + 'A', y);
                                putS(&currentMessage);
                                myBoard[x][y] = MISS;
                                currentState = STATE_MYTURN; 
                            }
                        putS(&currentMessage);                            
                        apa_showBoard(&myBoard);
                        _delay_ms(300); // Um Trefferfeedback zu sehen    
                        }
                    }
                }        
                break;
                
            case STATE_MYTURN:
                apa_showBoard(&theirBoard);
                updatePositionShoot(&x,&y,&theirBoard);
                sprintf((char *)currentMessage, "SHT%c%d", x + 'A', y);
                putS(&currentMessage); 
                currentState = STATE_WAITFORANSWER;             
                break;
                
            case STATE_WAITFORANSWER:                            
                if (messageComplete && checkMessage(receivedMessage)) {
                    parseMessage(&receivedMessage, &command, &x, &y, opponentReady);
                    messageComplete = false; //Zurücksetzen für neue Nachricht
                    if (strcmp(command, "RPT") == 0) {
                        putS(&currentMessage);
                    }
                    else if (strcmp(command, "HIT") == 0) {                      
                        theirBoard[x][y] = HIT;
                        currentState = STATE_THEIRTURN; 
                        }
                    else if (strcmp(command, "MIS") == 0){
                        theirBoard[x][y] = MISS;
                        currentState = STATE_THEIRTURN; 
                        }
                    else if (strcmp(command, "SNK") == 0){
                        isSunk(&theirBoard,x,y);
                        currentState = STATE_THEIRTURN; 
                        }
                    else if (strcmp(command, "OVR") == 0){
                        isSunk(&theirBoard,x,y);
                        won = true;
                        currentState = STATE_GAMEOVER; 
                        } 
                
                apa_showBoard(&theirBoard);
                _delay_ms(500); // Um Trefferfeedback zu sehen
                }
                break;
                                 
            case STATE_GAMEOVER:
                if (won) {
                    init_field_random(&myBoard, TCNT1);
                    apa_showBoard(&myBoard);
                    _delay_ms(100); //PARTYLICHT   
                }else{
                    apa_setUnicolor('r');
                }
                break;
        }
         
    }
}
