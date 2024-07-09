/* 
 * File:   apa102.h
 * Author: 24SMC02
 *
 * Created on 16. Mai 2024, 08:54
 */

#ifndef APA102_H
#define	APA102_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* APA102_H */


void apa_Init(void);

void apa_setLeds(uint8_t *rawarray, uint16_t numLeds);

void apa_setLed();

void apa_setUnicolor(char color);

void SPI_write(uint8_t c);

void apa_showBoard(board *field);

//void displayArray(Fieldstate board[][COLS])