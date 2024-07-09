#include <avr/io.h>
#include "Game.h"
#include "apa102.h"

#define NUM_LEDS ROWS*COLS

/*
 * Initialisiert alles, was Harwareseitig fuer das Ansprechen eines apa102
 * Displays nötig ist.
 */
void apa_Init(){
  //MOSI als Ausgang  
  PORTB |= _BV(PB3);    //Pegel High
  DDRB |= _BV(PB3);
  //SCK als Ausgang
  PORTB &= ~_BV(PB5);   //Pegel Low
  DDRB |= _BV(PB5);     //Ausgang
  //MISO als Eingang
  PORTB |= _BV(PB4);    //Pull-Up an
  DDRB &= ~_BV(PB4);
  //SlaveSelect als Ausgang
  PORTB |= _BV(PB2);    //PullUp
  DDRB |= _BV(PB2);     //Ausgang

    //SPI Control Register Seite 184
  SPCR |= _BV(MSTR);    //Master mode (hier villeicht ohne |, damit alles save 0?)
  SPCR |= _BV(SPE);     //SPI an  
  SPCR &= ~_BV(DORD);   //MSB first
  SPCR &= ~_BV(CPOL);   //SCK low bei idle
  SPCR |= _BV(SPR0);    //speed = fosc/8 // 1mHz / 8 = 125kHz ?!?
  SPSR |= _BV(SPI2X);
}

/*
 * Schreibt einen einzelnen char nach spi
 * @param c: der char
 */
void SPI_write(uint8_t c){
    SPDR = c;
    //wait for transmission to complete
    while(!(SPSR & (1<<SPIF)));
}

/*
 * Schreibt eine ganze LED-Matrix nach Protokollvorgabe nach spi
 * @param rawarray: eindimensionales Array mit Farbvorgaben
 * @param numLeds: zu beschreibene LEDs
 */
void apa_setLeds(uint8_t *rawarray, uint16_t numLeds){
    
    //uint8_t *rawarray = (uint8_t*)ledarray;

//start Frame 32 Nullen
  for(int i = 0 ; i < 4 ; i++){
    SPI_write(0x00);
  }
  
  for(int i = 0 ; i < (numLeds*4) ; i+=4){
    SPI_write(0xe0 + rawarray[i]); //da erste 3 bits immer 1, rest für brigthness
    SPI_write(rawarray[i+1]);
    SPI_write(rawarray[i+2]);
    SPI_write(rawarray[i+3]);
  }
  
//end Frame
  for(int i = 0 ; i < 4 ; i++){
    SPI_write(0x00);
  }
}

/*
 * erstellt ein Eindimensinales Array mit einheitlicher Farbbelegeung mit der
 * Helligkeitsstufe 1
 * @param: color: b = blau, r = rot, g = gruen
 */
void apa_setUnicolor(char color){
    uint8_t rawarray[FIELD_SIZE*4] = {};
    int offset = (color == 'r') ? 3 :
        (color == 'g') ? 2 : (color == 'b') ? 1 : 0;
    for (int i = 0 ; i < 256 ; i+=4){
        //  Brigthness
        //  Blue
        //  Green
        //  Red
        rawarray[i] = 1;
        rawarray[i+offset] = 255;
        apa_setLeds(&rawarray, FIELD_SIZE);
    }
    
}


/*
 * wandelt den Typ board in ein zweidimensionales Array um, dass von apa_setLEDs()
 * eingelesen werden kann und ruft diese direkt auf
 * @param field: Das Feld
 */
void apa_showBoard(board *field){
    //struct apa_RGB ledMyBoard[FIELD_SIZE];
    uint8_t rawarray[FIELD_SIZE*4] = {};
    
    for (int x = 0 ; x < ROWS ; x++){
        for (int y = 0 ; y < COLS ; y++){
            switch((*field)[x][y]){
            case EMPTY:
                //Formel Color: BRG
                //aktuelleSpalte * REIHE * 4 + aktuelleReihe*4 + offset für Colors
                //blue
                rawarray[(x*ROWS*4)+y*4] = 1;
                rawarray[(x*ROWS*4)+y*4 + 1] = 100;
                rawarray[(x*ROWS*4)+y*4 + 2] = 25;
                rawarray[(x*ROWS*4)+y*4 + 3] = 0;
                break;   
            case HIT://orange
                rawarray[(x*ROWS*4)+y*4] = 1;
                rawarray[(x*ROWS*4)+y*4 + 1] = 0;
                rawarray[(x*ROWS*4)+y*4 + 2] = 100;
                rawarray[(x*ROWS*4)+y*4 + 3] = 255;
                 break;   
            case MISS://aus
                rawarray[(x*ROWS*4)+y*4] = 1;
                rawarray[(x*ROWS*4)+y*4 + 1] = 0;
                rawarray[(x*ROWS*4)+y*4 + 2] = 0;
                rawarray[(x*ROWS*4)+y*4 + 3] = 0;
                break;
            case SHIP:
                //white
                rawarray[(x*ROWS*4)+y*4] = 1;
                rawarray[(x*ROWS*4)+y*4 + 1] = 255;
                rawarray[(x*ROWS*4)+y*4 + 2] = 255;
                rawarray[(x*ROWS*4)+y*4 + 3] = 255;
                break;
            case SUNK:
                //RED
                rawarray[(x*ROWS*4)+y*4] = 1;
                rawarray[(x*ROWS*4)+y*4 + 1] = 0;
                rawarray[(x*ROWS*4)+y*4 + 2] = 0;
                rawarray[(x*ROWS*4)+y*4 + 3] = 255;
                break;    
            case PREVIEW_SHIP:
                //türkis
                rawarray[(x*ROWS*4)+y*4] = 1;
                rawarray[(x*ROWS*4)+y*4 + 1] = 20;
                rawarray[(x*ROWS*4)+y*4 + 2] = 255;
                rawarray[(x*ROWS*4)+y*4 + 3] = 0;
                break;
            }}
    }
    apa_setLeds(&rawarray, FIELD_SIZE);
}
