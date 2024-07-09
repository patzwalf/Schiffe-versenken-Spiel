/* 
 * File:   Game.h
 * Author: Jan Ritter (105773) & Fabian Patzwall (106081)
 *
 * Created on 18. April 2024, 09:48
 */

#ifndef GAME_H
#define	GAME_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* GAME_H */

#include <stdbool.h>
#define ROWS 8
#define COLS 8
/** Definiere die Groesse des Spielfelds */
#define FIELD_SIZE (COLS * ROWS)

enum Fieldstate{
            EMPTY = 0,
            HIT,
            MISS,
            SHIP,
            SUNK,
            PREVIEW_SHIP           
};

typedef enum Fieldstate Fieldstate;

/** Typendefinition fuer ein Spielfeld. */
typedef Fieldstate board[ROWS][COLS];

void init_hardware();

void updatePosition(int shipLength, board* field);

void updatePositionShoot(int *xPointer, int *yPointer, board* field);

void init_field(board* field);

void init_field_random(board* field, uint16_t seed);

void parseMessage(char* message, char* command, int *x, int *y, bool opponentRdy);

//Ueberprueft den Syntax einer eingegangenen Nachricht
int checkMessage(char* message);

bool isSunk(board *field, int x, int y);

uint16_t waitForButtonPress();
